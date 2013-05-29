#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include <tbb/tbb.h>

class TBBInit {
    tbb::task_scheduler_init init;
    mutable tbb::enumerable_thread_specific<int> tids;
    mutable tbb::atomic<int> atomique;
  public:
    TBBInit();
    ~TBBInit();

    int getTID() const;
};

#endif //__vtkSMPImplementation_h_
