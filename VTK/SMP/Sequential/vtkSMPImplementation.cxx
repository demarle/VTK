#include "vtkSMP.h"

namespace vtkSMP
{

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
    {
    for ( ; first < last; ++first )
      (*op)( first, 0 );
    }

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* f, int grain )
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

  vtkSMPThreadID InternalGetTID()
    {
    return 0;
    }

  void Parallel( const vtkTask* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    if (!skipThreads)
      function->Execute( 0, data );
    }

  void Traverse( const vtkParallelTree* Tree, vtkFunctor* func )
    {
    vtkTreeIndex current( 0, 0 );
    while ( current.index != -1 )
      {
      current = Tree->TraverseNode( current, func, 0 );
      }
    }
}
