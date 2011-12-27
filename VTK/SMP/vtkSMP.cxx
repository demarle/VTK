#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"

namespace vtkSMP
{
  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor& op)
    {
    InternalForEach( first, last, &op );
    }

  void InitialiseThreadLocal( const vtkFunctorInitialisable& f )
    {
    InternalInit( &f );
    }

  void VTK_SMP_EXPORT FillThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result )
    {
    result.clear();
    InternalGetThreadsIDs( result );
    }

  //--------------------------------------------------------------------------------
//  vtkMutexLocker::vtkMutexLocker(vtkMutexLock* lock)
//    {
//    this->Lock = lock;
//    this->Lock->Lock();
//    }
//  vtkMutexLocker::~vtkMutexLocker()
//    {
//    this->Lock->Unlock();
//    }

  //--------------------------------------------------------------------------------
  vtkStandardNewMacro(vtkThreadLocal);

  vtkThreadLocal::vtkThreadLocal() : vtkObject() { }

  vtkThreadLocal::~vtkThreadLocal()
    {
    for ( typename vtkstd::map<vtkSMPThreadID, vtkObject*>::iterator it = this->ThreadLocalStorage.begin();
          it != this->ThreadLocalStorage.end(); ++it )
      {
      it->second->UnRegister(this);
      it->second = 0;
      }
    this->ThreadLocalStorage.clear();
    }

  void vtkThreadLocal::PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf( os, indent );
    os << indent << "Class stored: ";
    if (this->ThreadLocalStorage.size())
      cout << this->ThreadLocalStorage[0]->GetClassName() << endl;
    else
      cout << "unknown" << endl;
    os << indent << "Local storage: " << endl;
    for ( vtkstd::map<vtkSMPThreadID, vtkObject*>::iterator it = this->ThreadLocalStorage.begin();
          it != this->ThreadLocalStorage.end(); ++it )
      {
      os << indent.GetNextIndent() << "id " << it->first << ": (" << it->second << ")" << endl;
      it->second->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
      }
    }

  void vtkThreadLocal::SetLocal ( vtkSMPThreadID tid, vtkObject* item )
    {
    if (this->ThreadLocalStorage[tid])
      this->ThreadLocalStorage[tid]->UnRegister(this);
    item->Register(this);
    this->ThreadLocalStorage[tid] = item;
    }

  vtkObject* vtkThreadLocal::GetLocal(vtkSMPThreadID tid)
    {
    return this->ThreadLocalStorage[tid];
    }

}
