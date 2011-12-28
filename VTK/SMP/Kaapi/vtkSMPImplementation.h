#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapic.h>
#include <vector>

void __attribute__ ((constructor)) smpInit(void);
void __attribute__ ((destructor)) smpFini(void);

typedef int32_t vtkSMPThreadID;

struct vtkFunctor {
  virtual void operator () ( vtkIdType, vtkSMPThreadID ) const = 0;
};

struct vtkFunctorInitialisable : public vtkFunctor
{
  virtual void init ( vtkSMPThreadID ) const = 0;
};

void functor( int32_t b, int32_t e, int32_t tid, const vtkFunctor* o, vtkIdType f );

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op);
void InternalInit( const vtkFunctorInitialisable* f );
void InternalGetThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result );

#endif //__vtkSMPImplementation_h_
