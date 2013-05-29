#include "vtkSMP.h"

void sequential_traverse( vtkIdType index, int lvl, vtkIdType BranchingFactor, const vtkParallelTree* Tree, vtkFunctor* func )
  {
  if ( Tree->TraverseNode( index, lvl, func ) )
    {
    for ( vtkIdType i = index * BranchingFactor + 1, j = 0; j < BranchingFactor; ++i, ++j )
      {
      sequential_traverse( i, lvl + 1, BranchingFactor, Tree, func );
      }
    }
  }

namespace vtkSMP
{
  int InternalGetTid()
    {
    return 0;
    }

  int InternalGetNumberOfThreads()
    {
    return 1;
    }

  vtkSMPThreadID GetNumberOfThreads( )
    {
    return 1;
    }

  vtkSMPThreadID InternalGetTID()
    {
    return 0;
    }

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
    {
    for ( ; first < last; ++first )
      (*op)( first );
    }

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* f, int grain )
    {
    if ( f->ShouldInitialize() )
      f->Init( );
    for ( ; first < last; ++first )
      (*f)( first );
    }

  void Parallel( const vtkTask* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    if (!skipThreads)
      function->Execute( 0, data );
    }

  void Traverse( const vtkParallelTree* Tree, vtkFunctor* func )
    {
    int lvl;
    vtkIdType bf;
    Tree->GetTreeSize(lvl,bf);
    sequential_traverse( 0, 0, bf, Tree, func );
    }

  template<>
  void Parallel<vtkSMPMergePoints> ( const vtkTask* function,
                                     vtkSMP::vtkThreadLocal<vtkSMPMergePoints>::iterator data,
                                     vtkIdType skipThreads )
    {
    int num_thread = 0;
    if ( skipThreads <= num_thread )
      function->Execute( *(data+num_thread) );
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
    int num_thread = 0;
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
