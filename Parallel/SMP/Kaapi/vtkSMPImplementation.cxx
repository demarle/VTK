#include "vtkSMP.h"
#include "vtkCommand.h"
#include "vtkFunctor.h"
#include <kaapic.h>
#include <cmath>

KaapiInit::KaapiInit() // : com(ka::System::join_community()) { }
  {
  kaapic_init(KAAPIC_START_ONLY_MAIN);
  }

KaapiInit::~KaapiInit()
  {
//  com.leave();
//  ka::System::terminate();
  kaapic_finalize();
  }

const KaapiInit performInit;

template<class Functor>
class Work {
  public:
    /* Dummy constructor for type checking */
    Work() : _op(NULL), _array(0), _grain(0) {}

    Work(vtkIdType beg, size_t size, const Functor* op)
      {
      _op    = op;
      _array = beg;
      _grain = sqrt(size);
      kaapi_atomic_initlock( &_lock );
      kaapi_workqueue_init_with_lock(&_wq, 0, size, &_lock);
      }

    Work(vtkIdType beg, size_t size, const Functor* op, kaapi_workqueue_index_t grain)
      {
      _op    = op;
      _array = beg;
      _grain = grain;
      kaapi_atomic_initlock( &_lock );
      kaapi_workqueue_init_with_lock(&_wq, 0, size, &_lock);
      }

    ~Work() {kaapi_atomic_destroylock( &_lock );}

    /* execute sequential work */
    bool doWork()
      {
      kaapi_workqueue_index_t b, e;
      vtkIdType beg, end;
      if ( kaapi_workqueue_pop(&_wq, &b, &e, _grain) ) return false;
      end = _array+e;
      for ( beg = _array+b; beg < end; ++beg )
        {
        (*_op)( beg );
        }
      return true;
      }

    /* split work and reply to requests */
    int split (ka::StealContext* sc, int nreq, ka::ListRequest::iterator beg, ka::ListRequest::iterator end);

    const Functor*& getFunctor() {return _op;}

  protected:
    /* extract parallel work for nreq. Return the unit size */
    bool helper_split( int& nreq, vtkIdType& beg, vtkIdType& end)
      {
      kaapi_atomic_lock( &_lock );
      kaapi_workqueue_index_t steal_size, i,j;
      kaapi_workqueue_index_t range_size = kaapi_workqueue_size(&_wq);
      if (range_size <= _grain)
        {
        kaapi_atomic_unlock( &_lock );
        return false;
        }

      steal_size = range_size * nreq / (nreq + 1);
      if (steal_size == 0)
        {
        nreq = (range_size / _grain) - 1;
        steal_size = nreq*_grain;
        }

      /* perform the actual steal. */
      if (kaapi_workqueue_steal(&_wq, &i, &j, steal_size))
        {
        kaapi_atomic_unlock( &_lock );
        return false;
        }
      kaapi_atomic_unlock( &_lock );
      beg = _array + i;
      end = _array + j;
      return true;
      }

  protected:
    const Functor*          _op;
    vtkIdType               _array;
    kaapi_workqueue_t       _wq;
    kaapi_lock_t            _lock;
    kaapi_workqueue_index_t _grain;
};

template<class Functor>
struct TaskWork : public ka::Task<1>::Signature<ka::RW<Work<Functor> > > {};

template<class Functor>
struct TaskBodyCPU<TaskWork<Functor> > {
  void operator() ( ka::pointer_rw<Work<Functor> > work )
    {
    while( work->doWork() );
    }
};

template<>
struct TaskBodyCPU<TaskWork<vtkFunctorInitializable> > {
  void operator() ( ka::pointer_rw<Work<vtkFunctorInitializable> > work )
    {
    const vtkFunctorInitializable* functor = work->getFunctor();
    if ( functor->ShouldInitialize() )
      functor->Init();
    while( work->doWork() );
    }
};

template<class Functor>
struct TaskSplitter<TaskWork<Functor> > {
  int operator() ( ka::StealContext* sc,
                   int nreq,
                   ka::ListRequest::iterator begin,
                   ka::ListRequest::iterator end,
                   ka::pointer_rw<Work<Functor> > work )
    {
    return work->split(sc, nreq, begin, end);
    }
};

template<class Functor>
int Work<Functor>::split (ka::StealContext* sc, int nreq, ka::ListRequest::iterator beg, ka::ListRequest::iterator end)
  {
  /* stolen range */
  vtkIdType beg_theft;
  vtkIdType end_theft;
  size_t size_theft;

  if (!helper_split( nreq, beg_theft, end_theft ))
    return 0;
  size_theft = (end_theft-beg_theft)/nreq;

  /* thief work: create a task */
  for (; nreq>1; --nreq, ++beg, beg_theft+=size_theft)
    {
    beg->Spawn<TaskWork<Functor> >(sc)(new (*beg) Work<Functor>( beg_theft, size_theft, _op, _grain));
    beg->commit();
    }
  beg->Spawn<TaskWork<Functor> >(sc)(new (*beg) Work<Functor>( beg_theft, end_theft-beg_theft, _op, _grain));
  beg->commit();
  ++beg;

  return 0;
}

template<class T1, class T2, class T3, class T4, class T5, class T6>
struct TaskParallel_ : public ka::Task<15>::Signature<const vtkTask*, T1*, T2*, T3*, T4*, T5*, T6*, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType> {};

template<> template<class T1, class T2, class T3, class T4, class T5, class T6>
struct TaskBodyCPU<TaskParallel_<T1, T2, T3, T4, T5, T6> > {
  void operator() ( const vtkTask* function,
                    T1* data1, T2* data2, T3* data3, T4* data4, T5* data5, T6* data6,
                    vtkIdType offset1, vtkIdType offset2,
                    vtkIdType offset3, vtkIdType offset4,
                    vtkIdType offset5, vtkIdType offset6,
                    vtkIdType offset7, vtkIdType offset8 )
    {
    function->Execute( data1, data2, data3, data4, data5, data6, offset1, offset2, offset3, offset4, offset5, offset6, offset7, offset8 );
    }
};

template<class T>
struct TaskParallel : public ka::Task<2>::Signature<const vtkTask*, T*> {};

template<> template<class T>
struct TaskBodyCPU<TaskParallel<T> > {
  void operator() ( const vtkTask* function, T* data )
    {
    function->Execute(data);
    }
};

typedef struct tree_work
{
  kaapi_workqueue_t wq;
  const vtkParallelTree* Tree;
  vtkFunctor* op;
  int max_level;
  int cur_level;
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

  vtkFunctorInitializable* iop = vtkFunctorInitializable::SafeDownCast( work->op );
  if ( iop && iop->ShouldInitialize( ) )
    iop->Init( );

  kaapi_workqueue_index_t i, nil;

  vtkIdType id = convert_to_row_first(kaapi_workqueue_range_begin(&work->wq), work->cur_level, work);
  while ( !kaapi_workqueue_pop(&work->wq, &i, &nil, 1) )
    {
    if ( work->Tree->TraverseNode( id, work->cur_level, work->op ) )
      {
      ++work->cur_level;
      id *= work->branching_factor;
      }
    else
      {
      if ( work->max_level != work->cur_level )
        {
        kaapi_workqueue_pop(&work->wq, &i, &nil, work->nodes_per_subtrees[work->max_level - work->cur_level] - 1);
        }
      while ( !(id % work->branching_factor) && work->cur_level > work->root_lvl )
        {
        --work->cur_level;
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
  while ( req && work->cur_level >= steal_lvl && steal_lvl < work->max_level )
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
      tw->cur_level = steal_lvl;
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

inline void doFor( int32_t b, int32_t e, int32_t tid, const vtkFunctor* o )
  {
  for (int32_t k = b; k < e; ++k)
    {
    (*o)( k );
    }
  }
inline void doForInit( int32_t b, int32_t e, int32_t tid, const vtkFunctorInitializable* o )
  {
  if (o->ShouldInitialize())
    o->Init();
  for (int32_t k = b; k < e; ++k)
    {
    (*o)( k );
    }
  }

//--------------------------------------------------------------------------------
int vtkSMPInternalGetNumberOfThreads()
{
  return kaapi_getconcurrency();
}

int vtkSMPInternalGetTid()
{
  return kaapi_get_self_kid();
}

void vtkSMPForEachOp ( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
{
  vtkIdType n = last - first;
  int g = grain ? grain : sqrt(n);
  kaapic_begin_parallel(KAAPIC_FLAG_DEFAULT);
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, g, g);
  kaapic_foreach( first, last, &attr, 1, doFor, op );
  kaapic_end_parallel(KAAPIC_FLAG_DEFAULT);
  kaapic_foreach_attr_destroy(&attr);
}

void vtkSMPForEachOp ( vtkIdType first, vtkIdType last, const vtkFunctorInitializable* op, int grain )
{
  vtkIdType n = last - first;
  int g = grain ? grain : sqrt(n);
  kaapic_begin_parallel(KAAPIC_FLAG_DEFAULT);
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, g, g);
  kaapic_foreach( first, last, &attr, 1, doForInit, op );
  kaapic_end_parallel(KAAPIC_FLAG_DEFAULT);
  kaapic_foreach_attr_destroy(&attr);
}

template<>
void vtkSMPParallelOp<vtkSMPMergePoints> ( const vtkTask* function,
                                           vtkThreadLocal<vtkSMPMergePoints>::iterator data1,
                                           vtkIdType skipThreads )
{
  for ( vtkIdType tid = 0; tid < skipThreads; ++tid )
    {
    ++data1;
    }
//    kaapi_begin_parallel( KAAPI_SCHEDFLAG_DEFAULT );
  kaapic_begin_parallel(KAAPIC_FLAG_DEFAULT);
  for ( vtkIdType tid = skipThreads; tid < kaapi_getconcurrency(); ++tid )
    {
    ka::Spawn<TaskParallel<vtkSMPMergePoints> >()( function, *data1++ );
    }
  kaapic_end_parallel(KAAPIC_FLAG_DEFAULT);
//    kaapi_end_parallel( KAAPI_SCHEDFLAG_DEFAULT );
}

template<>
void vtkSMPParallelOp<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>
  (
   const vtkTask* function,
   vtkThreadLocal<vtkIdList>::iterator data1,
   vtkThreadLocal<vtkCellData>::iterator data2,
   vtkThreadLocal<vtkCellArray>::iterator data3,
   vtkThreadLocal<vtkCellArray>::iterator data4,
   vtkThreadLocal<vtkCellArray>::iterator data5,
   vtkThreadLocal<vtkCellArray>::iterator data6,
   vtkstd::vector<vtkIdType>::iterator offset1,
   vtkstd::vector<vtkIdType>::iterator offset2,
   vtkstd::vector<vtkIdType>::iterator offset3,
   vtkstd::vector<vtkIdType>::iterator offset4,
   vtkstd::vector<vtkIdType>::iterator offset5,
   vtkstd::vector<vtkIdType>::iterator offset6,
   vtkstd::vector<vtkIdType>::iterator offset7,
   vtkstd::vector<vtkIdType>::iterator offset8,
   vtkIdType skipThreads )
{
  for ( vtkIdType tid = 0; tid < skipThreads; ++tid )
    {
    ++data1; ++data2; ++data3; ++data4; ++data5; ++data6;
    ++offset1; ++offset2; ++offset3; ++offset4; ++offset5; ++offset6; ++offset7; ++offset8;
    }
//    kaapi_begin_parallel( KAAPI_SCHEDFLAG_DEFAULT );
  kaapic_begin_parallel(KAAPIC_FLAG_DEFAULT);
  for ( vtkIdType tid = skipThreads; tid < kaapi_getconcurrency(); ++tid )
    {
    ka::Spawn<TaskParallel_<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray> >()(
                                                                                                                function, *data1++, *data2++, *data3++, *data4++, *data5++, *data6++,
                                                                                                                *offset1++, *offset2++, *offset3++, *offset4++, *offset5++, *offset6++, *offset7++, *offset8++ );
    }
  kaapic_end_parallel(KAAPIC_FLAG_DEFAULT);
//    kaapi_end_parallel( KAAPI_SCHEDFLAG_DEFAULT );
}

void vtkSMPTraverseOp( const vtkParallelTree *Tree, vtkFunctor* func )
{
  work_t work;
  kaapi_workqueue_index_t i, nil;

  kaapic_begin_parallel(KAAPIC_FLAG_DEFAULT);

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

  work.cur_level = work.root_lvl;
  vtkIdType id = 0;
  while ( !kaapi_workqueue_pop(&work.wq, &i, &nil, 1) )
    {
    if ( Tree->TraverseNode( id, work.cur_level, func ) )
      {
      ++work.cur_level;
      id *= work.branching_factor;
      }
    else
      {
      if ( work.max_level != work.cur_level )
        {
        kaapi_workqueue_pop(&work.wq, &i, &nil, work.nodes_per_subtrees[work.max_level - work.cur_level] - 1);
        }
      while ( !(id % work.branching_factor) && work.cur_level > work.root_lvl )
        {
        --work.cur_level;
        id = (id - 1) / work.branching_factor;
        }
      }
    ++id;
    }

  kaapi_task_end_adaptive(thread, sc);

  /* wait for thieves */
  kaapi_sched_sync( );

  delete [] work.nodes_per_subtrees;

  kaapic_end_parallel(KAAPIC_FLAG_DEFAULT);

}
