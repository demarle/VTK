#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapic.h>

void __attribute__ ((constructor)) smpInit(void);
void __attribute__ ((destructor)) smpFini(void);

typedef int32_t vtkSMPThreadID;

#endif //__vtkSMPImplementation_h_
