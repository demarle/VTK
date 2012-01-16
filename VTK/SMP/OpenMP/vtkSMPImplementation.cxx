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

void InternalMerge( const vtkMergeable* f )
{
  #pragma omp parallel
  {
    f->merge( omp_get_thread_num() );
  }
}

void InternalPreMerge( const vtkMergeableInitialisable* f )
{
  #pragma omp parallel
  {
    f->pre_merge( omp_get_thread_num() );
  }
}
