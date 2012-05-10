#include "vtkSMP.h"
#include <omp.h>

namespace vtkSMP
{

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

  void Spawn ( const vtkTask* function, const vtkObject* data )
    {
    #pragma omp parallel
    #pragma omp single
      {
      #pragma omp task
      function->Execute( omp_get_thread_num(), data );
      #pragma omp taskwait
      }
    }
}
