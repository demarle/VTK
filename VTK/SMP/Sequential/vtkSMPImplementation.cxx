#include "vtkSMP.h"

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  for ( ; first < last; ++first )
    (*op)( first, 0 );
}

void InternalInit( const vtkFunctorInitialisable* f )
{
  f->init( 0 );
}

vtkSMPThreadID InternalGetNumberOfThreads( )
{
  return 1;
}

void InternalParallel( const vtkFunctor *f, int whichOne, vtkSMPThreadID skipThreads )
{
  if (!skipThreads)
    f->Parallel( 0, whichOne );
}
