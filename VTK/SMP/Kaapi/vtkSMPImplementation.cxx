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

void tth_do_init ( vtkTreeTraversalHelper* tth, vtkIdType size )
  {
  tth->Init( size );
  }

vtkIdType tth_get_current ( vtkTreeTraversalHelper* tth )
  {
  return tth->current_head;
  }

vtkTreeIndex tth_get ( vtkTreeTraversalHelper* tth )
  {
  return tth->Get();
  }

vtkTreeIndex tth_steal ( vtkTreeTraversalHelper* tth, vtkIdType i )
  {
  return tth->Steal( i );
  }

typedef struct tree_work
{
  kaapi_workqueue_t wq;
  vtkTreeTraversalHelper nodes;
  const vtkParallelTree* Tree;
  vtkFunctor* op;
  vtkIdType size;
} work_t;

static void thief_entrypoint( void* args, kaapi_thread_t* thread )
  {
  work_t* const work = (work_t*)(args);

  vtkSMPThreadID kid = (vtkSMPThreadID) kaapi_get_self_kid();

  vtkFunctorInitialisable* iop = vtkFunctorInitialisable::SafeDownCast( work->op );
  if ( iop && iop->ShouldInitialize( kid ) )
    iop->Init( kid );

  kaapi_workqueue_index_t i, nil;

  while ( !kaapi_workqueue_pop(&(work->wq), &i, &nil, 1) )
    {
    vtkTreeIndex id = tth_get( &(work->nodes) );
    work->Tree->TraverseNode( id.index, id.level, &(work->nodes), work->op, kid );
    kaapi_workqueue_push( &(work->wq), (kaapi_workqueue_index_t)tth_get_current( &(work->nodes) ) );
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

  /* nrequests count */
  int nreq = kaapi_api_listrequest_iterator_count( lri );

redo_steal:
  kaapi_workqueue_index_t range_size = kaapi_workqueue_size(&work->wq);
  if ( --range_size < 1 )
    return 0;

  if ( range_size < nreq )
    nreq = range_size;

  if (kaapi_workqueue_steal(&work->wq, &i, &j, nreq))
    goto redo_steal;

  kaapi_request_t* req = kaapi_api_listrequest_iterator_get(lr, lri);
  for ( ; i < j; ++i )
    {
    work_t* const tw = (work_t*)kaapi_request_pushdata(req, sizeof(work_t));
    tw->Tree = work->Tree;
    tw->op = work->op;
    tw->size = work->size;
    tth_do_init( &(tw->nodes), tw->size );
    kaapi_workqueue_init_with_kproc( &tw->wq,
                                     (kaapi_workqueue_index_t)tw->size,
                                     (kaapi_workqueue_index_t)tw->size,
                                     req->ident );
    vtkTreeIndex id = tth_steal( &(work->nodes), i );
    tw->nodes.push_tail( id.index, id.level );
    kaapi_workqueue_push( &tw->wq, (kaapi_workqueue_index_t)tth_get_current( &(tw->nodes) ) );


    kaapi_task_init( kaapi_request_toptask(req), thief_entrypoint, tw);
    kaapi_request_pushtask_adaptive( req, victim_task, splitter, 0 );
    kaapi_request_committask(req);

    req = kaapi_api_listrequest_iterator_next(lr, lri);
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
    kaapi_workqueue_index_t i, nil;

    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );

    /* get the self thread */
    kaapi_thread_t* thread = kaapi_self_thread();

    /* initialize work */
    work.Tree = Tree;
    work.op = func;
    work.size = Tree->GetTreeSize();
    tth_do_init( &(work.nodes), work.size );
    kaapi_workqueue_init(&work.wq, (kaapi_workqueue_index_t)work.size, (kaapi_workqueue_index_t)work.size);
    work.nodes.push_tail( 0, 0 );
    kaapi_workqueue_push( &work.wq, (kaapi_workqueue_index_t)tth_get_current( &(work.nodes) ) );

    /* push an adaptive task */
    void* sc = kaapi_task_begin_adaptive(
          thread,
          KAAPI_SC_CONCURRENT | KAAPI_SC_NOPREEMPTION,
          splitter,
          &work     /* arg for splitter = work to split */
          );

    while ( !kaapi_workqueue_pop(&work.wq, &i, &nil, 1) )
      {
      vtkTreeIndex id = tth_get( &(work.nodes) );
      Tree->TraverseNode( id.index, id.level, &work.nodes, func, kaapic_get_thread_num() );
      kaapi_workqueue_push( &work.wq, (kaapi_workqueue_index_t)tth_get_current( &(work.nodes) ) );
      }

    kaapi_task_end_adaptive(thread, sc);

    /* wait for thieves */
    kaapi_sched_sync( );

    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );

    }
}
