#include "vtkSMP.h"

void omp_traversal( vtkIdType index, int lvl, vtkIdType BranchingFactor, const vtkParallelTree* Tree, vtkFunctor* func )
  {
  vtkFunctorInitialisable* f = vtkFunctorInitialisable::SafeDownCast(func);
  if ( f && f->ShouldInitialize() ) f->Init();
  if ( Tree->TraverseNode(index, lvl, func) )
    {
    for ( vtkIdType i = index * BranchingFactor + 1, j = 0; j < BranchingFactor; ++i, ++j )
      {
      #pragma omp task
        omp_traversal( i, lvl + 1, BranchingFactor, Tree, func );
      }
    #pragma omp taskwait
    }
  }

namespace vtkSMP
{
  int InternalGetTid()
    {
    return omp_get_thread_num();
    }

  int InternalGetNumberOfThreads()
    {
    int numThreads = 1;
    #pragma omp parallel shared(numThreads)
    #pragma omp master
      {
      numThreads = omp_get_num_threads();
      }
    return numThreads;
    }

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain)
    {
    #pragma omp default(none)
    #pragma omp parallel for
    for ( vtkIdType i = first ; i < last ; ++i )
      (*op)( i );
    }

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op, int grain)
    {
    #pragma omp parallel
    {
    if ( op->ShouldInitialize( ) )
      op->Init( );
    }
    #pragma omp default(none)
    #pragma omp parallel for
    for ( vtkIdType i = first ; i < last ; ++i )
      (*op)( i );
    }

  template<>
  void Parallel<vtkSMPMergePoints> ( const vtkTask* function,
                                     vtkSMP::vtkThreadLocal<vtkSMPMergePoints>::iterator data,
                                     vtkIdType skipThreads )
    {
    #pragma omp parallel shared(skipThreads, data)
      {
      int num_thread = omp_get_thread_num();
      if ( skipThreads <= num_thread )
        function->Execute( *(data+num_thread) );
      }
    }

  template<>
  void Parallel<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray> (
      const vtkTask* function,
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
    #pragma omp parallel shared(skipThreads, data1, data2, data3, data4, data5, data6, offset1, offset2, offset3, offset4, offset5, offset6, offset7, offset8 )
      {
      int num_thread = omp_get_thread_num();
      if ( skipThreads <= num_thread )
        function->Execute( *(data1+num_thread),
                           *(data2+num_thread),
                           *(data3+num_thread),
                           *(data4+num_thread),
                           *(data5+num_thread),
                           *(data6+num_thread),
                           *(offset1+num_thread),
                           *(offset2+num_thread),
                           *(offset3+num_thread),
                           *(offset4+num_thread),
                           *(offset5+num_thread),
                           *(offset6+num_thread),
                           *(offset7+num_thread),
                           *(offset8+num_thread) );
      }
    }

  void Traverse( const vtkParallelTree* Tree, vtkFunctor* func )
    {
    int lvl;
    vtkIdType bf;
    Tree->GetTreeSize(lvl,bf);
    #pragma omp parallel
    #pragma omp master
      {
      omp_traversal( 0, 0, bf, Tree, func);
      }
    }
}
