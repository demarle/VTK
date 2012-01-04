#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapic.h>
#include <vector>

void __attribute__ ((constructor)) smpInit(void);
void __attribute__ ((destructor)) smpFini(void);

typedef int32_t vtkSMPThreadID;

class vtkFunctor;
class vtkFunctorInitialisable;
class vtkMergeable;
class vtkMergeableInitialisable;

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);
void InternalInit( const vtkFunctorInitialisable* f );
void InternalGetThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result );
void InternalMerge( const vtkMergeable* f );
void InternalPreMerge( const vtkMergeableInitialisable* f );

#endif //__vtkSMPImplementation_h_
