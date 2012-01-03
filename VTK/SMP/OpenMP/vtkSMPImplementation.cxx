#include "vtkSMP.h"
#include <omp.h>

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  #pragma omp default(none)
  #pragma omp parallel for
  for ( vtkIdType i = first ; i < last ; ++i )
    (*op)(i, omp_get_thread_num());
}

void InternalInit( const vtkFunctorInitialisable* f )
{
  #pragma omp parallel
  {
    f->init( omp_get_thread_num() );
  }
}

void InternalGetThreadsIDs(vtkstd::vector<vtkSMPThreadID>& result)
{
  int numThreads = 1;
  #pragma omp parallel shared(numThreads)
  {
  #pragma omp master
    {
      numThreads = omp_get_num_threads();
    }
  }

  for ( vtkSMPThreadID i = 0; i < numThreads; ++i )
    result.push_back(i);
}
