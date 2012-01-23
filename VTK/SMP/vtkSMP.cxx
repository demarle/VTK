#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"

//--------------------------------------------------------------------------------
vtkFunctor::vtkFunctor() { }

vtkFunctor::~vtkFunctor() { }

void vtkFunctor::Parallel( vtkSMPThreadID tid, int whichOne ) const
{
  switch(whichOne)
  {
  case 1:
    Parallel1(tid);
    return;
  case 2:
    Parallel2(tid);
    return;
  case 3:
    Parallel3(tid);
    return;
  case 4:
    Parallel4(tid);
    return;
  case 5:
    Parallel5(tid);
    return;
  case 6:
    Parallel6(tid);
    return;
  case 7:
    Parallel7(tid);
    return;
  case 8:
    Parallel8(tid);
    return;
  case 9:
    Parallel9(tid);
    return;
  default:
    Parallel0(tid);
    return;
  }
}

//--------------------------------------------------------------------------------
vtkFunctorInitialisable::vtkFunctorInitialisable() : vtkFunctor()
  {
  IsInitialized = false;
  }

vtkFunctorInitialisable::~vtkFunctorInitialisable() { }

bool vtkFunctorInitialisable::CheckAndSetInitialized() const
  {
  bool ret = IsInitialized;
  IsInitialized = true;
  return ret;
  }


namespace vtkSMP
{
  //--------------------------------------------------------------------------------
  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor& op)
    {
    InternalForEach( first, last, &op );
    }

  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctorInitialisable& f )
    {
    if (!f.CheckAndSetInitialized())
      {
      InternalInit( &f );
      }
    InternalForEach( first, last, &f );
    }

  void Parallel(const vtkFunctor &op, int whichMethod, vtkSMPThreadID skipThreads )
  {
    InternalParallel( &op, whichMethod, skipThreads );
  }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return InternalGetNumberOfThreads( );
    }

  //--------------------------------------------------------------------------------
//  vtkStandardNewMacro(vtkThreadLocal);

//  vtkThreadLocal::vtkThreadLocal() : vtkObject() { }

//  vtkThreadLocal::~vtkThreadLocal()
//    {
//    for ( typename vtkstd::map<vtkSMPThreadID, vtkObject*>::iterator it = this->ThreadLocalStorage.begin();
//          it != this->ThreadLocalStorage.end(); ++it )
//      {
//      it->second->UnRegister(this);
//      it->second = 0;
//      }
//    this->ThreadLocalStorage.clear();
//    }

//  void vtkThreadLocal::PrintSelf(ostream &os, vtkIndent indent)
//    {
//    this->Superclass::PrintSelf( os, indent );
//    os << indent << "Class stored: ";
//    if (this->ThreadLocalStorage.size())
//      cout << this->ThreadLocalStorage[0]->GetClassName() << endl;
//    else
//      cout << "unknown" << endl;
//    os << indent << "Local storage: " << endl;
//    for ( vtkstd::map<vtkSMPThreadID, vtkObject*>::iterator it = this->ThreadLocalStorage.begin();
//          it != this->ThreadLocalStorage.end(); ++it )
//      {
//      os << indent.GetNextIndent() << "id " << it->first << ": (" << it->second << ")" << endl;
//      it->second->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
//      }
//    }

//  void vtkThreadLocal::SetLocal ( vtkSMPThreadID tid, vtkObject* item )
//    {
//    if (this->ThreadLocalStorage[tid])
//      {
//      this->ThreadLocalStorage[tid]->UnRegister(this);
//      }

//    if (item)
//      {
//      item->Register(this);
//      }
//    this->ThreadLocalStorage[tid] = item;
//    }

//  vtkObject* vtkThreadLocal::GetLocal(vtkSMPThreadID tid)
//    {
//    return this->ThreadLocalStorage[tid];
//    }

}
