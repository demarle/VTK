#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <algorithm>
#include <vector>

typedef int vtkSMPThreadID;

class vtkFunctor;
class vtkFunctorInitialisable;
class vtkMergeable;
class vtkMergeableInitialisable;

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);
void InternalInit( const vtkFunctorInitialisable* f );
vtkSMPThreadID InternalGetNumberOfThreads( );
void InternalParallel( const vtkFunctor* f, int whichOne, vtkSMPThreadID skipThreads );

#endif //__vtkSMPImplementation_h_
