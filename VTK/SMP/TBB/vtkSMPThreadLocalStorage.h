template<class T>
vtkThreadLocal<T>::vtkThreadLocal() :
    vtkObject(), ThreadLocalStorage()
  {
  }

template<class T>
vtkThreadLocal<T>::~vtkThreadLocal()
  {
  for ( typename vtkThreadLocalStorageContainer<T*>::iterator it = ThreadLocalStorage.begin();
        it != ThreadLocalStorage.end(); ++it )
    {
    if ( *it )
      (*it)->UnRegister( this );
    *it = 0;
    }
  ThreadLocalStorage.clear();
  }

template<class T>
void vtkThreadLocal<T>::PrintSelf( ostream &os, vtkIndent indent )
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Class stored: " << typeid(T).name() << endl;
  os << indent << "Local storage: " << endl;
  size_t i = 0;
  for ( typename vtkThreadLocalStorageContainer<T*>::iterator it = ThreadLocalStorage.begin();
        it != ThreadLocalStorage.end(); ++it, ++i )
    {
    os << indent.GetNextIndent() << "id " << i << ": (" << *it << ")" << endl;
    if ( *it ) (*it)->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
    }
  }

template<class T>
T* vtkThreadLocal<T>::NewLocal( T *specificImpl )
  {
  T*& local = ThreadLocalStorage.local();
  if ( local )
    {
    local->UnRegister(this);
    }

  local = specificImpl->NewInstance();
  if (local)
    {
    local->Register(this);
    local->Delete();
    }

  return local;
  }

template<class T>
T* vtkThreadLocal<T>::NewLocal( )
  {
  T*& local = ThreadLocalStorage.local();
  if ( local )
    {
    local->UnRegister(this);
    }

  local = T::New();
  if (local)
    {
    local->Register(this);
    local->Delete();
    }

  return local;
  }

template<class T>
void vtkThreadLocal<T>::SetLocal( T* item )
  {
  T*& local = ThreadLocalStorage.local();
  if ( local )
    {
    local->UnRegister(this);
    }

  local = item;
  if (local)
    {
    local->Register(this);
    }
  }

template<class T>
T* vtkThreadLocal<T>::GetLocal( )
  {
  T*& local = ThreadLocalStorage.local();
  return local;
  }

template<class T> template<class Derived>
Derived* vtkThreadLocal<T>::GetLocal()
  {
  T*& local = ThreadLocalStorage.local();
  Derived* derived_local = Derived::SafeDownCast(local);
  if ( !derived_local )
    {

    }
  return derived_local;
  }

template<class T>
vtkThreadLocal<T>* vtkThreadLocal<T>::New()
  {
  return new vtkThreadLocal<T>();
  }

template<class T>
typename vtkThreadLocalStorageContainer<T*>::iterator vtkThreadLocal<T>::GetOrCreateAll( T* specificImpl )
  {
  for ( typename vtkThreadLocalStorageContainer<T*>::iterator it = ThreadLocalStorage.begin();
        it != ThreadLocalStorage.end(); ++it )
    {
    if ( *it == NULL )
      {
      *it = specificImpl->NewInstance();

      if ( (*it) )
        {
        (*it)->Register( this );
        (*it)->Delete();
        }
      }
    }
  return ThreadLocalStorage.begin();
  }

template<class T>
typename vtkThreadLocalStorageContainer<T*>::iterator vtkThreadLocal<T>::GetOrCreateAll( )
  {
  for ( typename vtkThreadLocalStorageContainer<T*>::iterator it = ThreadLocalStorage.begin();
        it != ThreadLocalStorage.end(); ++it )
    {
    if ( *it == NULL )
      {
      *it = T::New();

      if ( (*it) )
        {
        (*it)->Register( this );
        (*it)->Delete();
        }
      }
    }
  return ThreadLocalStorage.begin();
  }

template<class T>
typename vtkThreadLocalStorageContainer<T*>::iterator vtkThreadLocal<T>::GetAll( vtkIdType skipThreads )
  {
  typename vtkThreadLocalStorageContainer<T*>::iterator value = ThreadLocalStorage.begin();
  while ( skipThreads )
    {
    ++value;
    --skipThreads;
    }
  return value;
  }

template<class T>
typename vtkThreadLocalStorageContainer<T*>::iterator vtkThreadLocal<T>::EndOfAll( )
  {
  return ThreadLocalStorage.end();
  }
