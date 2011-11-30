#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <algorithm>


struct vtkFunctor {
  virtual void operator () (vtkIdType) const = 0;
};

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);

#endif //__vtkSMPImplementation_h_
