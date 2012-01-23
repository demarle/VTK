#include "vtkSMP.h"

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  vtkIdType i = first;
  while ( i < last )
    {
    (*op)( i, 0 );
    ++i;
    }
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
