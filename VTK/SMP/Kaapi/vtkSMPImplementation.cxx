#include "vtkSMP.h"
#include "vtkCommand.h"
#include <kaapic.h>
#include <kaapi_atomic.h>
#include <cmath>

void smpInit(void)
  {
  kaapic_init( KAAPIC_START_ONLY_MAIN );
  }
void smpFini(void)
  {
  kaapic_finalize();
  }

void func_call ( int32_t b, int32_t e, int32_t tid, const vtkFunctor* o )
  {
  for ( int32_t k = b; k < e; ++k )
    {
    (*o)( k, tid );
    }
  }

void func_call_init ( int32_t b, int32_t e, int32_t tid, const vtkFunctorInitialisable* o )
  {
  if ( o->ShouldInitialize(tid) )
    {
    o->Init( tid );
    }
  for ( int32_t k = b; k < e; ++k )
    {
    (*o)( k, tid );
    }
  }

void my_parallel ( const vtkTask* function, const vtkObject* data, vtkSMPThreadID tid )
  {
  function->Execute( tid, data );
  }

void my_spawn ( const vtkTask* function )
  {
  function->Execute( kaapic_get_thread_num(), 0 );
  }

typedef struct tree_work
{
  vtkTreeIndex current;
  vtkTreeIndex steal;
  const vtkParallelTree* Tree;
  vtkFunctor* op;
  kaapi_lock_t lock;
} work_t;

static int check_remaining_work( work_t* work )
  {
  if ( work->current.index == -1 ) return 1;

  if ( work->steal.level == work->current.level && work->current.index > work->steal.index ) return 1;

  return 0; /* success */
  }

static void thief_entrypoint( void* args, kaapi_thread_t* thread )
  {
  work_t* const work = (work_t*)(args);

  vtkSMPThreadID kid = (vtkSMPThreadID) kaapi_get_self_kid();

  vtkFunctorInitialisable* iop = vtkFunctorInitialisable::SafeDownCast( work->op );
  if ( iop && iop->ShouldInitialize( kid ) )
    iop->Init( kid );

  while ( !check_remaining_work( work ) )
    {
    vtkTreeIndex next_node = work->Tree->TraverseNode( work->current, work->op, kid );
    kaapi_atomic_lock( &(work->lock) );
    work->current = next_node;
    kaapi_atomic_unlock( &(work->lock) );
    }

  kaapi_atomic_destroylock( &(work->lock) );
  }

static int kaapi_tree_steal( work_t* work, int w_req, vtkIdType* out_id, int* out_lvl )
  {
  /* Disable steals on branches beeing develloped by master thread */
  if ( work->steal.index == work->current.index || work->current.index == -1 ) return 0;

  int result = w_req;

  kaapi_atomic_lock( &(work->lock) );
  int sl = work->steal.level;
  vtkTreeIndex index_next = work->Tree->GetAncestor( work->current, sl + 1 );
  vtkTreeIndex index_steal = work->Tree->GetAncestor( index_next, sl );
  kaapi_atomic_unlock( &(work->lock) );

  printf("steal: %d %d\n", index_next.index, index_steal.index);

  int n_req = work->steal.index - index_steal.index;

  if ( n_req >= w_req )
    {
    work->steal.index -= w_req;
    *out_id = work->steal.index;
    *out_lvl = work->steal.level;
    }
  else
    {
    vtkTreeIndex si = work->Tree->GetLastDescendant( index_steal );

    int nn_req = si.index - index_steal.index;
    work->steal.level = *out_lvl = sl + 1;
    if ( nn_req + n_req < w_req ) // not enought
      {
      work->steal.index = *out_id = index_next.index;
      result = nn_req + n_req;
      }
    else
      {
      work->steal.index = *out_id = si.index - ( w_req - n_req );
      }
    }

  ++(*out_id);
  return result;
  }

static int splitter(
    struct kaapi_task_t* victim_task,
    void*  args,
    struct kaapi_listrequest_t* lr,
    struct kaapi_listrequest_iterator_t* lri
    )
  {
  work_t* const work = (work_t*)(args);

  /* nrequests count */
  int nreq = kaapi_api_listrequest_iterator_count( lri );

  vtkIdType stolen_id = -1;
  int stolen_lvl = -1;
  nreq = kaapi_tree_steal( work, nreq, &stolen_id, &stolen_lvl );
  printf("splitter called: %d, %d, %d, %d -> %d\n",
         work->current.index, work->current.level,
         work->steal.index, work->steal.level, nreq );
  if ( nreq == 0 ) return 0; /* Nothing to steal */

  kaapi_request_t* req = kaapi_api_listrequest_iterator_get(lr, lri);
  vtkTreeIndex root = vtkTreeIndex( stolen_id, stolen_lvl );
  while ( req != 0 )
    {
    /* thief work: not adaptive result because no preemption is used here  */
    work_t* const tw = (work_t*)kaapi_request_pushdata(req, sizeof(work_t));
    tw->Tree = work->Tree;
    tw->current = root;
    tw->steal = root;
    tw->op = work->op;
    kaapi_atomic_initlock( &(tw->lock) );

    kaapi_task_init( kaapi_request_toptask(req), thief_entrypoint, tw);
    kaapi_request_pushtask_adaptive( req, victim_task, splitter, 0 );

    if ( --nreq == 0 ) break; /* Not enought branches to steal */
    req = kaapi_api_listrequest_iterator_next(lr, lri);
    root = work->Tree->GetNextStealableNode( root );
    }

  return 0;
  }

//--------------------------------------------------------------------------------
namespace vtkSMP
{
  vtkSMPThreadID InternalGetTID()
    {
    return kaapic_get_thread_num();
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
    {
    vtkIdType n = last - first;
    if (!n) return;
    uint32_t granularity = grain ? grain : sqrt(n);
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, granularity, granularity);
    kaapic_foreach( first, last, &attr, 1, func_call, op );
    kaapic_foreach_attr_destroy(&attr);
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op, int grain )
    {
    vtkIdType n = last - first;
    if (!n) return;
    uint32_t granularity = grain ? grain : sqrt(n);
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, granularity, granularity);
    kaapic_foreach( first, last, &attr, 1, func_call_init, op );
    kaapic_foreach_attr_destroy(&attr);
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return kaapic_get_concurrency();
    }

  void Parallel( const vtkTask* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = 1; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 3, my_parallel,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, data, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_V, tid, 1, KAAPIC_TYPE_INT );
      }
    if ( !skipThreads )
      {
      kaapic_spawn_attr_set_kproc(&attr, 0);
      kaapic_spawn( &attr, 3, my_parallel,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, data, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_V, 0, 1, KAAPIC_TYPE_INT );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }

  void BeginSpawnRegion()
    {
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    }

  void Spawn( const vtkTask *function )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_spawn( &attr, 1, my_spawn, KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR );
    kaapic_spawn_attr_destroy(&attr);
    }

  void Sync( )
    {
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    }

  void Traverse( const vtkParallelTree *Tree, vtkFunctor* func )
    {
    work_t work;

    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );

    /* get the self thread */
    kaapi_thread_t* thread = kaapi_self_thread();

    /* initialize work */
    work.current.index = 0;
    work.current.level = 0;
    work.steal.index = 0;
    work.steal.level = 0;
    work.Tree = Tree;
    work.op = func;
    kaapi_atomic_initlock( &(work.lock) );


    /* push an adaptive task */
    void* sc = kaapi_task_begin_adaptive(
          thread,
          KAAPI_SC_CONCURRENT | KAAPI_SC_NOPREEMPTION,
          splitter,
          &work     /* arg for splitter = work to split */
          );

    while ( !check_remaining_work( &work ) )
      {
      vtkTreeIndex next_node = Tree->TraverseNode( work.current, func, kaapic_get_thread_num() );
      kaapi_atomic_lock( &(work.lock) );
      work.current = next_node;
      kaapi_atomic_unlock( &(work.lock) );
      }

    kaapi_task_end_adaptive(thread, sc);

    kaapi_atomic_destroylock( &(work.lock) );

    /* wait for thieves */
    kaapi_sched_sync( );

    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );

    }
}
