#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkSMPImplementation.h"
#include "vtkObject.h"
#include "vtkMutexLock.h"

#include <map>

class vtkPoints;
class vtkMutexLock;

class VTK_SMP_EXPORT vtkFunctor
{
public:
  virtual void operator () ( vtkIdType, vtkSMPThreadID ) const = 0;

protected:
  vtkFunctor();
  ~vtkFunctor();
};

class VTK_SMP_EXPORT vtkFunctorInitialisable : public vtkFunctor
{
public:
  virtual void init ( vtkSMPThreadID ) const = 0;
  bool CheckAndSetInitialized() const;

protected:
  vtkFunctorInitialisable();
  ~vtkFunctorInitialisable();

private:
  mutable bool IsInitialized;
};

namespace vtkSMP
{

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

  void VTK_SMP_EXPORT ForEach(vtkIdType first, vtkIdType last, const vtkFunctorInitialisable& f );

  void VTK_SMP_EXPORT FillThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result );
}

#endif //__vtkSMP_h__
