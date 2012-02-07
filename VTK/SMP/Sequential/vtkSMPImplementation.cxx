#include "vtkSMP.h"

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  for ( ; first < last; ++first )
    (*op)( first, 0 );
}

vtkSMPThreadID InternalGetNumberOfThreads( )
{
  return 1;
}

void InternalParallel( const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) , vtkSMPThreadID skipThreads )
{
  if (!skipThreads)
    m( f, 0 );
}
