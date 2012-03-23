#include "vtkSMP.h"

namespace vtkSMP
{

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
    {
    for ( ; first < last; ++first )
      (*op)( first, 0 );
    }

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable *f )
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

  void Parallel( const vtkSMPCommand* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    if (!skipThreads)
      function->Execute( 0, data );
    }

}
