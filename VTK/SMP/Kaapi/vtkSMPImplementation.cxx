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
  vtkTreeTraversalHelper tree_helper;
  const vtkParallelTree* Tree;
  vtkFunctor* op;
} work_t;

static void thief_entrypoint( void* args, kaapi_thread_t* thread )
  {
  work_t* const work = (work_t*)(args);

  vtkSMPThreadID kid = (vtkSMPThreadID) kaapi_get_self_kid();

  vtkFunctorInitialisable* iop = vtkFunctorInitialisable::SafeDownCast( work->op );
  if ( iop && iop->ShouldInitialize( kid ) )
    iop->Init( kid );

  vtkIdType i;
  int l;
  while ( i != -1 )
    {
    work->Tree->TraverseNode( i, l, &(work->tree_helper), work->op, kid );
    work->tree_helper.execute( &i, &l );
    }

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

  vtkTreeIndex* result_iterator;
  nreq = work->tree_helper.steal( nreq, result_iterator );
  if ( nreq++ == 0 ) return 0; /* Nothing to steal + hack for --nreq later */

  kaapi_request_t* req = kaapi_api_listrequest_iterator_get(lr, lri);
  vtkTreeIndex* next = result_iterator->next;
  while ( --nreq )
    {
    work_t* const tw = (work_t*)kaapi_request_pushdata(req, sizeof(work_t));
    tw->Tree = work->Tree;
    tw->op = work->op;
    tw->tree_helper.push_head( result_iterator->index, result_iterator->level );

    kaapi_task_init( kaapi_request_toptask(req), thief_entrypoint, tw);
    kaapi_request_pushtask_adaptive( req, victim_task, splitter, 0 );
    kaapi_request_committask(req);

    req = kaapi_api_listrequest_iterator_next(lr, lri);
    result_iterator = next;
    next = result_iterator->next;
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
    work.Tree = Tree;
    work.op = func;


    /* push an adaptive task */
    void* sc = kaapi_task_begin_adaptive(
          thread,
          KAAPI_SC_CONCURRENT | KAAPI_SC_NOPREEMPTION,
          splitter,
          &work     /* arg for splitter = work to split */
          );

    vtkIdType index = 0;
    int level = 0;
    while ( index != -1 )
      {
      Tree->TraverseNode( index, level, &(work.tree_helper), func, kaapic_get_thread_num() );
      work.tree_helper.execute( &index, &level );
      }

    kaapi_task_end_adaptive(thread, sc);

    /* wait for thieves */
    kaapi_sched_sync( );

    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );

    }
}
