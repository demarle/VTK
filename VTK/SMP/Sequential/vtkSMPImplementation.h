#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <algorithm>
#include <vector>

typedef int vtkSMPThreadID;

struct vtkFunctor {
  virtual void operator () ( vtkIdType, vtkSMPThreadID ) const = 0;
};

struct vtkFunctorInitialisable : public vtkFunctor
{
  virtual void init ( vtkSMPThreadID ) const = 0;
};

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);
void InternalInit( const vtkFunctorInitialisable* f );
void InternalGetThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result );

#endif //__vtkSMPImplementation_h_
