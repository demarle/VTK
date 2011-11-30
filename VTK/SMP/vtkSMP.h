#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkSMPImplementation.h"
#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"

#include <map>

class vtkPoints;

namespace vtkSMP
{

  // vtkMutexLocker
  class VTK_SMP_EXPORT vtkMutexLocker
    {
    public :
      vtkMutexLocker(vtkMutexLock* lock);
      ~vtkMutexLocker();

    protected :
      vtkMutexLock* Lock;
    };

  // vtkThreadLocal
  template <class T>
  class VTK_SMP_EXPORT vtkThreadLocal
    {
    public :
      vtkThreadLocal() { this->Lock = vtkMutexLock::New(); }

      ~vtkThreadLocal() { this->ThreadLocalStorage.clear(); this->Lock->Delete(); }

      // Description:
      // Store a thread-local item.
      vtkThreadLocal<T>& operator = (T item)
        {
        vtkMutexLocker(this->Lock);
        this->ThreadLocalStorage[vtkMultiThreader::GetCurrentThreadID()] = item;
        return *this;
        }

      // Description :
      // convert this vtkThreadLocal to the stored item
      operator T&()
        {
        vtkMutexLocker(this->Lock);
        return this->ThreadLocalStorage[vtkMultiThreader::GetCurrentThreadID()];
        }

    protected :
      // the __thread c++Ox modifier cannot be used because this is not static
      // create an explicit map and use the thread id key instead.
      vtkstd::map<vtkMultiThreaderIDType, T> ThreadLocalStorage;

      //TODO : using a mutexlock is safe but inefficent,
      // replace this by a read/write lock
      vtkMutexLock* Lock;
    };

  // ForEach template : parallel loop over an iterator
  void VTK_SMP_EXPORT ForEach(vtkIdType first, vtkIdType last, const vtkFunctor& op);

//  void VTK_SMP_EXPORT ForEachCoordinates(vtkPoints* data, void(*op)(float&));

}

#endif //__vtkSMP_h__
