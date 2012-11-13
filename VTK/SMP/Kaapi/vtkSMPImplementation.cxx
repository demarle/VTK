#include "vtkSMP.h"
#include "vtkCommand.h"
#include <kaapic.h>
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
  kaapi_workqueue_t wq;
  const vtkParallelTree* Tree;
  vtkFunctor* op;
  int max_level;
  vtkIdType branching_factor;
  vtkIdType* nodes_per_subtrees;
  int root_lvl;
} work_t;

vtkIdType convert_to_row_first( kaapi_workqueue_index_t i, int level, work_t* work )
  {
  if ( !level ) return 0;
  int lvl = 1;
  vtkIdType index = 1;
  while ( lvl <= level )
    {
    vtkIdType size = work->nodes_per_subtrees[work->max_level - lvl];
    while ( i > size )
      {
      i -= size;
      ++index;
      }
    if ( level != lvl )
      index = index * work->branching_factor + 1;
    ++lvl;
    --i;
    }
  return index;
  }

static void thief_entrypoint( void* args, kaapi_thread_t* thread )
  {
  work_t* const work = (work_t*)(args);

  vtkSMPThreadID kid = (vtkSMPThreadID) kaapi_get_self_kid();

  vtkFunctorInitialisable* iop = vtkFunctorInitialisable::SafeDownCast( work->op );
  if ( iop && iop->ShouldInitialize( kid ) )
    iop->Init( kid );

  kaapi_workqueue_index_t i, nil;

  int level = work->root_lvl;
  vtkIdType id = convert_to_row_first(kaapi_workqueue_range_begin(&work->wq), level, work);
  while ( !kaapi_workqueue_pop(&work->wq, &i, &nil, 1) )
    {
    if ( work->Tree->TraverseNode( id, level, work->op, kaapic_get_thread_num() ) )
      {
      ++level;
      id *= work->branching_factor;
      }
    else
      {
      if ( work->max_level != level )
        kaapi_workqueue_pop(&work->wq, &i, &nil, work->nodes_per_subtrees[work->max_level - level] - 1);
      while ( !(id % work->branching_factor) && level > work->root_lvl )
        {
        --level;
        id = (id - 1) / work->branching_factor;
        }
      }
    ++id;
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
  kaapi_workqueue_index_t i, j;
  int steal_lvl = work->root_lvl + 1;

  kaapi_request_t* req = kaapi_api_listrequest_iterator_get(lr, lri);
  while ( req && steal_lvl < work->max_level )
    {
    if ( !(kaapi_workqueue_steal(&work->wq, &i, &j, work->nodes_per_subtrees[work->max_level - steal_lvl]) ) )
      {
      work_t* const tw = (work_t*)kaapi_request_pushdata(req, sizeof(work_t));
      tw->Tree = work->Tree;
      tw->op = work->op;
      tw->branching_factor = work->branching_factor;
      tw->max_level = work->max_level;
      tw->nodes_per_subtrees = work->nodes_per_subtrees;
      tw->root_lvl = steal_lvl;
      kaapi_workqueue_init_with_kproc( &tw->wq, i, j, req->ident );
      kaapi_task_init( kaapi_request_toptask(req), thief_entrypoint, tw);
      kaapi_request_pushtask_adaptive( req, victim_task, splitter, 0 );
      kaapi_request_committask(req);

      req = kaapi_api_listrequest_iterator_next(lr, lri);
      }
    else
      ++steal_lvl;
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

  void Traverse( const vtkParallelTree *Tree, vtkFunctor* func )
    {
    work_t work;
    kaapi_workqueue_index_t i, nil;

    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );

    /* get the self thread */
    kaapi_thread_t* thread = kaapi_self_thread();

    /* initialize work */
    work.Tree = Tree;
    work.op = func;
    Tree->GetTreeSize( work.max_level, work.branching_factor );
    work.nodes_per_subtrees = new vtkIdType[work.max_level + 1];
    vtkIdType value = work.nodes_per_subtrees[work.root_lvl = 0] = 1;
    for ( vtkIdType i = 1; i <= work.max_level; ++i )
      {
      value *= work.branching_factor;
      work.nodes_per_subtrees[i] = ++value;
      }
    kaapi_workqueue_init(&work.wq, 0, (kaapi_workqueue_index_t)value);

    /* push an adaptive task */
    void* sc = kaapi_task_begin_adaptive(
          thread,
          KAAPI_SC_CONCURRENT | KAAPI_SC_NOPREEMPTION,
          splitter,
          &work     /* arg for splitter = work to split */
          );

    int level = work.root_lvl;
    vtkIdType id = 0;
    while ( !kaapi_workqueue_pop(&work.wq, &i, &nil, 1) )
      {
      if ( Tree->TraverseNode( id, level, func, kaapic_get_thread_num() ) )
        {
        ++level;
        id *= work.branching_factor;
        }
      else
        {
        if ( work.max_level != level )
          kaapi_workqueue_pop(&work.wq, &i, &nil, work.nodes_per_subtrees[work.max_level - level] - 1);
        while ( !(id % work.branching_factor) && level > work.root_lvl )
          {
          --level;
          id = (id - 1) / work.branching_factor;
          }
        }
      ++id;
      }

    kaapi_task_end_adaptive(thread, sc);

    /* wait for thieves */
    kaapi_sched_sync( );

    delete [] work.nodes_per_subtrees;

    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );

    }
}
