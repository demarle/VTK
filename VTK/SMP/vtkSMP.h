#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkSMPImplementation.h"
#include "vtkObject.h"
#include "vtkMutexLock.h"

#include <map>

class vtkPoints;
class vtkMutexLock;

namespace vtkSMP
{

  // vtkMutexLocker
//  class VTK_SMP_EXPORT vtkMutexLocker : public vtkObjectBase
//    {
//    public :
//      vtkMutexLocker(vtkMutexLock* lock);
//      ~vtkMutexLocker();

//    protected :
//      vtkMutexLock* Lock;
//    };

  class VTK_SMP_EXPORT vtkThreadLocal : public vtkObject
    {
    protected :
      vtkThreadLocal();
      ~vtkThreadLocal();

    public:
      vtkTypeMacro(vtkThreadLocal, vtkObject)
      static vtkThreadLocal* New();
      void PrintSelf(ostream &os, vtkIndent indent);

      void SetLocal ( vtkSMPThreadID tid, vtkObject* item );

      vtkObject* GetLocal(vtkSMPThreadID tid);

    protected:
      // the __thread c++Ox modifier cannot be used because this is not static
      // create an explicit map and use the thread id key instead.
      vtkstd::map<vtkSMPThreadID, vtkObject*> ThreadLocalStorage;
    };

  // ForEach template : parallel loop over an iterator
  void VTK_SMP_EXPORT ForEach(vtkIdType first, vtkIdType last, const vtkFunctor& op );

  void VTK_SMP_EXPORT InitialiseThreadLocal( const vtkFunctorInitialisable& f );

  void VTK_SMP_EXPORT FillThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result );
}

#endif //__vtkSMP_h__
