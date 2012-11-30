#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <omp.h>
#include <vector>
#include <typeinfo>

typedef int vtkSMPThreadID;
template<class T>
struct vtkThreadLocalStorageContainer {
  typedef vtkstd::vector<T*> type;
};

#endif //__vtkSMPImplementation_h_
