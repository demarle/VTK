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
    if (op->CheckAndSetInitialized())
      {
      #pragma omp parallel
        op->Init( omp_get_thread_num() );
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

  void Parallel( const vtkFunctor* f, const vtkSMPCommand* callback, vtkSMPThreadID skipThreads )
    {
    #pragma omp parallel shared(skipThreads)
      {
      int num = omp_get_thread_num();
      if ( num >= skipThreads )
        callback->Execute( f, vtkCommand::UserEvent + 42, &num );
      }
    }

}
