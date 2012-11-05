#include "vtkSMP.h"
#include <omp.h>

void omp_traversal( vtkIdType index, int lvl, vtkIdType BranchingFactor, const vtkParallelTree* Tree, vtkFunctor* func )
  {
  if ( Tree->TraverseNode(index, lvl, func, omp_get_thread_num()) )
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

  vtkSMPThreadID InternalGetTID()
    {
    return omp_get_thread_num();
    }

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain)
    {
    #pragma omp default(none)
    #pragma omp parallel for
    for ( vtkIdType i = first ; i < last ; ++i )
      (*op)(i, omp_get_thread_num());
    }

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op, int grain)
    {
    #pragma omp parallel
    {
    vtkSMPThreadID ThreadNum = omp_get_thread_num();
    if ( op->ShouldInitialize(ThreadNum) )
      op->Init( ThreadNum );
    }
    #pragma omp default(none)
    #pragma omp parallel for
    for ( vtkIdType i = first ; i < last ; ++i )
      (*op)(i, omp_get_thread_num());
    }

  vtkSMPThreadID GetNumberOfThreads( )
    {
    int numThreads = 1;
    #pragma omp parallel shared(numThreads)
      {
      #pragma omp master
        {
        numThreads = omp_get_num_threads();
        }
    }

    return numThreads;
    }

  void Parallel ( const vtkTask* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    #pragma omp parallel shared(skipThreads)
      {
      int num = omp_get_thread_num();
      if ( num >= skipThreads )
        function->Execute( num, data );
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
