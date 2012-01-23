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
  void Parallel( vtkSMPThreadID, int) const;

protected:
  vtkFunctor();
  ~vtkFunctor();

  virtual void Parallel0( vtkSMPThreadID ) const { };
  virtual void Parallel1( vtkSMPThreadID ) const { };
  virtual void Parallel2( vtkSMPThreadID ) const { };
  virtual void Parallel3( vtkSMPThreadID ) const { };
  virtual void Parallel4( vtkSMPThreadID ) const { };
  virtual void Parallel5( vtkSMPThreadID ) const { };
  virtual void Parallel6( vtkSMPThreadID ) const { };
  virtual void Parallel7( vtkSMPThreadID ) const { };
  virtual void Parallel8( vtkSMPThreadID ) const { };
  virtual void Parallel9( vtkSMPThreadID ) const { };
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

      void PrintSelf( ostream &os, vtkIndent indent )
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

      void SetLocal ( vtkSMPThreadID tid, T* item )
        {
        if ( this->ThreadLocalStorage[tid] )
          {
          this->ThreadLocalStorage[tid]->UnRegister(this);
          }

        if ( item )
          {
          item->Register( this );
          }

        this->ThreadLocalStorage[tid] = item;
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

  void VTK_SMP_EXPORT Parallel( const vtkFunctor& op, int whichMethod, vtkSMPThreadID skipThreads = 1 );

  vtkSMPThreadID VTK_SMP_EXPORT GetNumberOfThreads( );

}

#endif //__vtkSMP_h__
