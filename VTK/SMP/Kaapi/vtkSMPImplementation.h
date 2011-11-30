#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapic.h>

struct vtkFunctor {
  virtual void operator () (vtkIdType) const = 0;
};

void __attribute__ ((constructor)) smpInit(void);
void __attribute__ ((destructor)) smpFini(void);

void functor( int32_t* b, int32_t* e, int32_t* tid, const vtkFunctor* o, vtkIdType f );

void InternalForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op );

#define ENABLE_XKAAPI

#endif //__vtkSMPImplementation_h_
