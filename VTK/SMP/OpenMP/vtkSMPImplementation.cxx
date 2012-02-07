#include "vtkSMP.h"
#include <omp.h>

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  #pragma omp default(none)
  #pragma omp parallel for
  for ( vtkIdType i = first ; i < last ; ++i )
    (*op)(i, omp_get_thread_num());
}

vtkSMPThreadID InternalGetNumberOfThreads( )
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

void InternalParallel( const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) , vtkSMPThreadID skipThreads )
{
  #pragma omp parallel shared(skipThreads)
  {
    int num = omp_get_thread_num();
    if ( num >= skipThreads )
      m( f, num );
  }
}
