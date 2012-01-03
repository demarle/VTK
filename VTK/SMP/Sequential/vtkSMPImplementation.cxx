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

void InternalGetThreadsIDs(vtkstd::vector<vtkSMPThreadID>& result)
{
  result.push_back( 0 );
}
