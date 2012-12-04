#include "vtkSMP.h"
#include "vtkCommand.h"
#include <kaapic.h>
#include <kaapi++>
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
  if ( o->ShouldInitialize() )
    {
    o->Init();
    }
  for ( int32_t k = b; k < e; ++k )
    {
    (*o)( k, tid );
    }
  }

template<class T1, class T2, class T3, class T4, class T5, class T6>
void my_parallel ( const vtkTask* function,
                   T1* data1, T2* data2, T3* data3, T4* data4, T5* data5, T6* data6,
                   vtkIdType offset1, vtkIdType offset2,
                   vtkIdType offset3, vtkIdType offset4,
                   vtkIdType offset5, vtkIdType offset6,
                   vtkIdType offset7, vtkIdType offset8 )
  {
  function->Execute( data1, data2, data3, data4, data5, data6, offset1, offset2, offset3, offset4, offset5, offset6, offset7, offset8 );
  }

template<class T>
void my_parallel_1 ( const vtkTask* function,
                     T* data )
  {
  function->Execute( data );
  }

void my_parallel_2 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2 )
  {
  function->Execute( data1, data2 );
  }

void my_parallel_3 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3 )
  {
  function->Execute( data1, data2, data3 );
  }

void my_parallel_4 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4 )
  {
  function->Execute( data1, data2, data3, data4 );
  }

void my_parallel_5 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4,
                     vtkObject* data5 )
  {
  function->Execute( data1, data2, data3, data4, data5 );
  }

void my_parallel_6 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4,
                     vtkObject* data5,
                     vtkObject* data6 )
  {
  function->Execute( data1, data2, data3, data4, data5, data6 );
  }

void my_parallel_7 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4,
                     vtkObject* data5,
                     vtkObject* data6,
                     vtkObject* data7 )
  {
  function->Execute( data1, data2, data3, data4, data5, data6, data7 );
  }

void my_parallel_8 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4,
                     vtkObject* data5,
                     vtkObject* data6,
                     vtkObject* data7,
                     vtkObject* data8 )
  {
  function->Execute( data1, data2, data3, data4, data5, data6, data7, data8 );
  }

void my_parallel_9 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4,
                     vtkObject* data5,
                     vtkObject* data6,
                     vtkObject* data7,
                     vtkObject* data8,
                     vtkObject* data9 )
  {
  function->Execute( data1, data2, data3, data4, data5, data6, data7, data8, data9 );
  }

void my_parallel_0 ( const vtkTask* function,
                     vtkObject* data1,
                     vtkObject* data2,
                     vtkObject* data3,
                     vtkObject* data4,
                     vtkObject* data5,
                     vtkObject* data6,
                     vtkObject* data7,
                     vtkObject* data8,
                     vtkObject* data9,
                     vtkObject* data0 )
  {
  function->Execute( data1, data2, data3, data4, data5, data6, data7, data8, data9, data0 );
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

  vtkFunctorInitialisable* iop = vtkFunctorInitialisable::SafeDownCast( work->op );
  if ( iop && iop->ShouldInitialize( ) )
    iop->Init( );

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
  vtkIdTypeThreadLocal::vtkIdTypeThreadLocal()
      : vtkObject(), ThreadLocalStorage(kaapic_get_concurrency())
    {
    memset(&ThreadLocalStorage[0], 0, sizeof(vtkIdType) * kaapic_get_concurrency());
    }

  vtkIdTypeThreadLocal::~vtkIdTypeThreadLocal()
    {
    ThreadLocalStorage.clear();
    }

  vtkThreadLocalStorageContainer<vtkIdType>::iterator vtkIdTypeThreadLocal::GetAll()
    {
    return ThreadLocalStorage.begin();
    }

  vtkThreadLocalStorageContainer<vtkIdType>::iterator vtkIdTypeThreadLocal::EndOfAll()
    {
    return ThreadLocalStorage.end();
    }

  void vtkIdTypeThreadLocal::SetLocal( vtkIdType value )
    {
    ThreadLocalStorage[kaapic_get_thread_num()] = value;
    }

  vtkIdType vtkIdTypeThreadLocal::GetLocal()
    {
    return ThreadLocalStorage[kaapic_get_thread_num()];
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


  template<class T>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T*>::iterator data1,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 2, my_parallel_1,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }
  template<>
  void Parallel<vtkSMPMergePoints> ( const vtkTask* function,
                                     vtkThreadLocalStorageContainer<vtkSMPMergePoints*>::iterator data1,
                                     vtkSMPThreadID skipThreads )
    {
    for ( vtkSMPThreadID tid = 0; tid < skipThreads; ++tid )
      {
      ++data1;
      }
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 2, my_parallel_1<vtkSMPMergePoints>,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1++), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 3, my_parallel_2,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2, class T3>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 4, my_parallel_3,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2, class T3, class T4>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 5, my_parallel_4,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2, class T3, class T4, class T5>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 6, my_parallel_5,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data5+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 7, my_parallel_6,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data5+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data6+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset1,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset2,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset3,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset4,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset5,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset6,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset7,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset8,
                 vtkSMPThreadID skipThreads = 1 )
    {
    for ( vtkSMPThreadID tid = 0; tid < skipThreads; ++tid )
      {
      ++data1; ++data2; ++data3; ++data4; ++data5; ++data6;
      ++offset1; ++offset2; ++offset3; ++offset4; ++offset5; ++offset6; ++offset7; ++offset8;
      }
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 15, my_parallel,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data1++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data2++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data3++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data4++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data5++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data6++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_V, *offset1++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset2++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset3++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset4++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset5++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset6++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset7++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset8++, 1, KAAPIC_TYPE_INT );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }
  template<>
  void Parallel<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>( const vtkTask* function,
                 vtkThreadLocalStorageContainer<vtkIdList*>::iterator data1,
                 vtkThreadLocalStorageContainer<vtkCellData*>::iterator data2,
                 vtkThreadLocalStorageContainer<vtkCellArray*>::iterator data3,
                 vtkThreadLocalStorageContainer<vtkCellArray*>::iterator data4,
                 vtkThreadLocalStorageContainer<vtkCellArray*>::iterator data5,
                 vtkThreadLocalStorageContainer<vtkCellArray*>::iterator data6,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset1,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset2,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset3,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset4,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset5,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset6,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset7,
                 vtkThreadLocalStorageContainer<vtkIdType>::iterator offset8,
                 vtkSMPThreadID skipThreads )
    {
    for ( vtkSMPThreadID tid = 0; tid < skipThreads; ++tid )
      {
      ++data1; ++data2; ++data3; ++data4; ++data5; ++data6;
      ++offset1; ++offset2; ++offset3; ++offset4; ++offset5; ++offset6; ++offset7; ++offset8;
      }
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 15, my_parallel<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data1++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data2++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data3++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data4++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data5++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *data6++, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_V, *offset1++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset2++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset3++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset4++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset5++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset6++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset7++, 1, KAAPIC_TYPE_INT,
                    KAAPIC_MODE_V, *offset8++, 1, KAAPIC_TYPE_INT );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }

  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                 typename vtkThreadLocalStorageContainer<T7*>::iterator data7,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 8, my_parallel_7,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data5+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data6+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data7+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                 typename vtkThreadLocalStorageContainer<T7*>::iterator data7,
                 typename vtkThreadLocalStorageContainer<T8*>::iterator data8,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 9, my_parallel_8,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data5+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data6+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data7+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data8+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                 typename vtkThreadLocalStorageContainer<T7*>::iterator data7,
                 typename vtkThreadLocalStorageContainer<T8*>::iterator data8,
                 typename vtkThreadLocalStorageContainer<T9*>::iterator data9,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 10, my_parallel_9,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data5+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data6+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data7+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data8+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data9+tid), 1, KAAPIC_TYPE_PTR );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }


  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T0>
  void Parallel( const vtkTask* function,
                 typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                 typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                 typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                 typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                 typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                 typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                 typename vtkThreadLocalStorageContainer<T7*>::iterator data7,
                 typename vtkThreadLocalStorageContainer<T8*>::iterator data8,
                 typename vtkThreadLocalStorageContainer<T9*>::iterator data9,
                 typename vtkThreadLocalStorageContainer<T0*>::iterator data0,
                 vtkSMPThreadID skipThreads = 1 )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = skipThreads; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 11, my_parallel_0,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data1+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data2+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data3+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data4+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data5+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data6+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data7+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data8+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data9+tid), 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, *(data0+tid), 1, KAAPIC_TYPE_PTR );
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
