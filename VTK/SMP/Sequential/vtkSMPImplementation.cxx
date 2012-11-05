#include "vtkSMP.h"

void sequential_traverse( vtkIdType index, int lvl, vtkIdType BranchingFactor, const vtkParallelTree* Tree, vtkFunctor* func )
  {
  if ( Tree->TraverseNode( index, lvl, func, 0 ) )
    {
    for ( vtkIdType i = index * BranchingFactor + 1, j = 0; j < BranchingFactor; ++i, ++j )
      {
      sequential_traverse( i, lvl + 1, BranchingFactor, Tree, func );
      }
    }
  }

namespace vtkSMP
{

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
    {
    for ( ; first < last; ++first )
      (*op)( first, 0 );
    }

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* f, int grain )
    {
    if ( f->ShouldInitialize(0) )
      f->Init( 0 );
    for ( ; first < last; ++first )
      (*f)( first, 0 );
    }

  vtkSMPThreadID GetNumberOfThreads( )
    {
    return 1;
    }

  vtkSMPThreadID InternalGetTID()
    {
    return 0;
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
}
