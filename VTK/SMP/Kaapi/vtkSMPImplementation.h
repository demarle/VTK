#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapic.h>
#include <vector>
#include <typeinfo>

void __attribute__ ((constructor)) smpInit(void);
void __attribute__ ((destructor)) smpFini(void);

typedef int32_t vtkSMPThreadID;
template<class T>
struct vtkThreadLocalStorageContainer {
  typedef vtkstd::vector<T> type;
  typedef typename vtkstd::vector<T>::iterator iterator;
};

#endif //__vtkSMPImplementation_h_
