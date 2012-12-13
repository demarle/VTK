#include "vtkSMP.h"

int fillTid() {return -1;}

TBBInit::TBBInit() : init(), tids(fillTid) { atomique = -1; }
TBBInit::~TBBInit() {}
int TBBInit::getTID() const
  {
  int& tid = tids.local();
  if (tid == -1)
    {
    tid = ++atomique;
    }
  return tid;
  }

const TBBInit performInit;

class FuncCall
{
  const vtkFunctor* o;

public:
  void operator() ( const tbb::blocked_range<vtkIdType>& r ) const
    {
    for ( vtkIdType k = r.begin(); k < r.end(); ++k )
      {
      (*o)( k );
      }
    }

  FuncCall ( const vtkFunctor* _o ) : o(_o) { }
  ~FuncCall () { }
};

class FuncCallInit
{
  const vtkFunctorInitialisable* o;

public:
  void operator() ( const tbb::blocked_range<vtkIdType>& r ) const
    {
    if ( o->ShouldInitialize() )
      {
      o->Init( );
      }
    for ( vtkIdType k = r.begin(); k < r.end(); ++k )
      {
      (*o)( k );
      }
    }

  FuncCallInit ( const vtkFunctorInitialisable* _o ) : o(_o) { }
  ~FuncCallInit () { }
};

template<class T>
class TaskParallel : public tbb::task {
  public:
    const vtkTask* _task;
    T* _data;
    TaskParallel ( const vtkTask* t, T* d ) : _task(t), _data(d) {}
    tbb::task* execute()
      {
      _task->Execute(_data);
      return NULL;
      }
};

template<class T1, class T2, class T3, class T4, class T5, class T6>
class TaskParallel_ : public tbb::task {
  public:
    const vtkTask* _task;
    T1* _data1;
    T2* _data2;
    T3* _data3;
    T4* _data4;
    T5* _data5;
    T6* _data6;
    vtkIdType _offset1, _offset2, _offset3, _offset4, _offset5, _offset6, _offset7, _offset8;
    TaskParallel_ ( const vtkTask* t, T1* d1, T2* d2, T3* d3, T4* d4, T5* d5, T6* d6,
                    vtkIdType o1, vtkIdType o2, vtkIdType o3, vtkIdType o4,
                    vtkIdType o5, vtkIdType o6, vtkIdType o7, vtkIdType o8 ) :
      _task(t), _data1(d1), _data2(d2), _data3(d3), _data4(d4), _data5(d5), _data6(d6),
      _offset1(o1), _offset2(o2), _offset3(o3), _offset4(o4),
      _offset5(o5), _offset6(o6), _offset7(o7), _offset8(o8) {}
    tbb::task* execute()
      {
      _task->Execute(_data1, _data2, _data3, _data4, _data5, _data6, _offset1, _offset2, _offset3, _offset4, _offset5, _offset6, _offset7, _offset8 );
      return NULL;
      }
};

class TaskTraverse {
    const vtkParallelTree* Tree;
    vtkFunctor* Functor;
    const int level;
    const vtkIdType index, BranchingFactor;
  public:
    TaskTraverse( const vtkParallelTree* t, vtkFunctor* f, int l, vtkIdType i, vtkIdType b )
      : Tree(t), Functor(f), level(l), index(i), BranchingFactor(b) { }
    void operator() () const
      {
      vtkFunctorInitialisable* f = vtkFunctorInitialisable::SafeDownCast(Functor);
      if ( f && f->ShouldInitialize() ) f->Init();
      if ( Tree->TraverseNode(index, level, Functor) )
        {
        int l = level + 1;
        tbb::task_group g;
        for ( vtkIdType i = index * BranchingFactor + 1, j = 0; j < BranchingFactor; ++i, ++j )
          {
          g.run(TaskTraverse(Tree, Functor, l, i, BranchingFactor));
          }
        g.wait();
        }
      }
};

vtkIdType fillTLS() {return 0;}

//--------------------------------------------------------------------------------
namespace vtkSMP
{
  int InternalGetNumberOfThreads()
    {
    return tbb::task_scheduler_init::default_num_threads();
    }

  int InternalGetTid()
    {
    return performInit.getTID();
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
    {
    vtkIdType n = last - first;
    if (!n) return;
    vtkIdType g = grain ? grain : sqrt(n);
    tbb::parallel_for( tbb::blocked_range<vtkIdType>( first, last, g ), FuncCall( op ) );
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op, int grain )
    {
    vtkIdType n = last - first;
    if (!n) return;
    vtkIdType g = grain ? grain : sqrt(n);
    tbb::parallel_for( tbb::blocked_range<vtkIdType>( first, last, g ), FuncCallInit( op ) );
    }

  template<>
  void Parallel<vtkSMPMergePoints>( const vtkTask* function, vtkSMP::vtkThreadLocal<vtkSMPMergePoints>::iterator data1, vtkIdType skipThreads )
    {
    for (vtkIdType tid = 0; tid < skipThreads; ++tid )
      {
      ++data1;
      }
    tbb::task_list list;
    for (vtkIdType tid = skipThreads; tid < tbb::task_scheduler_init::default_num_threads(); ++tid )
      {
      list.push_back(*new(tbb::task::allocate_root()) TaskParallel<vtkSMPMergePoints>(function, *data1++));
      }
    tbb::task::spawn_root_and_wait(list);
    }

  template<>
  void Parallel<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>( const vtkTask* function,
                 vtkSMP::vtkThreadLocal<vtkIdList>::iterator data1,
                 vtkSMP::vtkThreadLocal<vtkCellData>::iterator data2,
                 vtkSMP::vtkThreadLocal<vtkCellArray>::iterator data3,
                 vtkSMP::vtkThreadLocal<vtkCellArray>::iterator data4,
                 vtkSMP::vtkThreadLocal<vtkCellArray>::iterator data5,
                 vtkSMP::vtkThreadLocal<vtkCellArray>::iterator data6,
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
    tbb::task_list list;
    for ( vtkIdType tid = skipThreads; tid < tbb::task_scheduler_init::default_num_threads(); ++tid )
      {
      list.push_back(*new(tbb::task::allocate_root()) TaskParallel_<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>(
                    function, *data1++, *data2++, *data3++, *data4++, *data5++, *data6++,
                    *offset1++, *offset2++, *offset3++, *offset4++, *offset5++, *offset6++, *offset7++, *offset8++ ));
      }
    tbb::task::spawn_root_and_wait(list);
    }

  void Traverse(const vtkParallelTree *Tree, vtkFunctor *func)
    {
    int level;
    vtkIdType bf;
    Tree->GetTreeSize(level, bf);
    tbb::task_group g;
    g.run(TaskTraverse(Tree, func, 0, 0, bf));
    g.wait();
    }
}
