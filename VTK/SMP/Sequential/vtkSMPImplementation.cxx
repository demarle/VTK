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

void InternalMerge( const vtkMergeable *f )
{
  f->merge( 0 );
}

void InternalPreMerge( const vtkMergeableInitialisable *f )
{
  f->pre_merge( 0 );
}
