#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkSMPImplementation.h"
#include "vtkObject.h"
#include "vtkMutexLock.h"

#include <map>
#include <typeinfo>  //for 'typeid'

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
  virtual void print() const = 0;

protected:
  vtkFunctorInitialisable();
  ~vtkFunctorInitialisable();

private:
  mutable bool IsInitialized;
};

class VTK_SMP_EXPORT vtkMergeable
{
public:
  virtual void merge ( vtkSMPThreadID ) const = 0;

protected:
  vtkMergeable();
  ~vtkMergeable();
};

class VTK_SMP_EXPORT vtkMergeableInitialisable : public vtkMergeable
{
public:
  virtual void pre_merge ( vtkSMPThreadID ) const = 0;

protected:
  vtkMergeableInitialisable();
  ~vtkMergeableInitialisable();
};

namespace vtkSMP
{

  template<class T>
  class VTK_SMP_EXPORT vtkThreadLocal : public vtkObject
    {
    protected :
      vtkThreadLocal() : vtkObject() { }
      ~vtkThreadLocal()
        {
        for ( typename vtkstd::map<vtkSMPThreadID, T*>::iterator it = ThreadLocalStorage.begin();
              it != ThreadLocalStorage.end(); ++it )
          {
          it->second->UnRegister( this );
          it->second = 0;
          }
        ThreadLocalStorage.clear();
        }

    public:
      vtkTypeMacro(vtkThreadLocal, vtkObject)
      static vtkThreadLocal<T>* New()
        {
        return new vtkThreadLocal<T>();
        }

      void PrintSelf(ostream &os, vtkIndent indent)
        {
        this->Superclass::PrintSelf( os, indent );
        os << indent << "Class stored: " << typeid(T).name() << endl;
        os << indent << "Local storage: " << endl;
        for ( typename vtkstd::map<vtkSMPThreadID, T*>::iterator it = this->ThreadLocalStorage.begin();
              it != this->ThreadLocalStorage.end(); ++it )
          {
          os << indent.GetNextIndent() << "id " << it->first << ": (" << it->second << ")" << endl;
          it->second->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
          }
        }

      T* NewLocal ( vtkSMPThreadID tid, T* specificImpl )
        {
        if (this->ThreadLocalStorage[tid])
          {
          this->ThreadLocalStorage[tid]->UnRegister(this);
          }

        T* item = specificImpl->NewInstance();
        if (item)
          {
          item->Register(this);
          item->Delete();
          }
        this->ThreadLocalStorage[tid] = item;

        return item;
        }

      T* NewLocal ( vtkSMPThreadID tid )
        {
        if (this->ThreadLocalStorage[tid])
          {
          this->ThreadLocalStorage[tid]->UnRegister(this);
          }

        T* item = T::New();
        if (item)
          {
          item->Register(this);
          item->Delete();
          }
        this->ThreadLocalStorage[tid] = item;

        return item;
        }

      T* GetLocal( vtkSMPThreadID tid )
        {
        return this->ThreadLocalStorage[tid];
        }

    protected:
      // the __thread c++Ox modifier cannot be used because this is not static
      // create an explicit map and use the thread id key instead.
      vtkstd::map<vtkSMPThreadID, T*> ThreadLocalStorage;
    };

  // ForEach template : parallel loop over an iterator
  void VTK_SMP_EXPORT ForEach(vtkIdType first, vtkIdType last, const vtkFunctor& op );

  void VTK_SMP_EXPORT ForEach(vtkIdType first, vtkIdType last, const vtkFunctorInitialisable& f );

  void VTK_SMP_EXPORT FillThreadsIDs( vtkstd::vector<vtkSMPThreadID>& result );

  void VTK_SMP_EXPORT Merge( const vtkMergeable& f );

  void VTK_SMP_EXPORT PreMerge( const vtkMergeableInitialisable& f );
}

#endif //__vtkSMP_h__
