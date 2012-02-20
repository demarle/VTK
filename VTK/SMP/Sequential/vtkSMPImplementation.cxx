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
    if (f->CheckAndSetInitialized())
      f->Init( 0 );
    for ( ; first < last; ++first )
      (*f)( first, 0 );
    }

  vtkSMPThreadID GetNumberOfThreads( )
    {
    return 1;
    }

  void Parallel( const vtkFunctor* f, const vtkSMPCommand* callback , vtkSMPThreadID skipThreads )
    {
    if (!skipThreads)
      callback->Execute( f, vtkCommand::UserEvent + 42, &skipThreads );
    }

}
