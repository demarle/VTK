#include "vtkSMP.h"
#include <omp.h>

namespace vtkSMP
{

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
    {
    #pragma omp default(none)
    #pragma omp parallel for
    for ( vtkIdType i = first ; i < last ; ++i )
      (*op)(i, omp_get_thread_num());
    }

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op)
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

  void Parallel( const vtkSMPCommand* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    #pragma omp parallel shared(skipThreads)
      {
      int num = omp_get_thread_num();
      if ( num >= skipThreads )
        function->Execute( num, data );
      }
    }

}
