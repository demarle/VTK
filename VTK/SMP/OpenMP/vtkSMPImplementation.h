#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <vector>

typedef int vtkSMPThreadID;

class vtkFunctor;
class vtkFunctorInitialisable;

void InternalForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op );
void InternalParallel( const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) , vtkSMPThreadID skipThreads );
vtkSMPThreadID InternalGetNumberOfThreads( );

#endif //__vtkSMPImplementation_h_
