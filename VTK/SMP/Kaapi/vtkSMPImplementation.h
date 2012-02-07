#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapic.h>

void __attribute__ ((constructor)) smpInit(void);
void __attribute__ ((destructor)) smpFini(void);

typedef int32_t vtkSMPThreadID;

class vtkFunctor;
class vtkFunctorInitialisable;

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);
void InternalParallel( const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) , vtkSMPThreadID skipThreads );
vtkSMPThreadID InternalGetNumberOfThreads( );

#endif //__vtkSMPImplementation_h_
