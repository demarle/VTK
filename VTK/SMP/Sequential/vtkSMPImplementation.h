#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <algorithm>
#include <vector>

typedef int vtkSMPThreadID;

class vtkFunctor;

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);
vtkSMPThreadID InternalGetNumberOfThreads( );
void InternalParallel( const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) , vtkSMPThreadID skipThreads );

#endif //__vtkSMPImplementation_h_
