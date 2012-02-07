#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"

//--------------------------------------------------------------------------------
vtkFunctor::vtkFunctor() { }

vtkFunctor::~vtkFunctor() { }


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


void vtkFunctorInitialization( const vtkFunctor* o, vtkSMPThreadID tid )
{
  static_cast<const vtkFunctorInitialisable*>(o)->Init( tid );
}

namespace vtkSMP
{
  //--------------------------------------------------------------------------------
  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctor& op )
    {
    InternalForEach( first, last, &op );
    }

  void ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable& f )
    {
    if (!f.CheckAndSetInitialized())
      {
      InternalParallel( &f, vtkFunctorInitialization, 0 );
      }
    InternalForEach( first, last, &f );
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return InternalGetNumberOfThreads( );
    }

  void Parallel( const vtkFunctor &f, void (*exe)(const vtkFunctor*, vtkSMPThreadID), vtkSMPThreadID skipThreads)
    {
      InternalParallel( &f, exe, skipThreads );
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
