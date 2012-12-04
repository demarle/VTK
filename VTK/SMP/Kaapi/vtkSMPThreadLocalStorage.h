template<class T>
vtkThreadLocal<T>::vtkThreadLocal() :
    vtkObject(), ThreadLocalStorage(kaapic_get_concurrency())
  {
  memset(&ThreadLocalStorage[0], 0, sizeof(T*) * kaapic_get_concurrency());
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
  for ( size_t i = 0; i < this->ThreadLocalStorage.size(); ++i )
    {
    os << indent.GetNextIndent() << "id " << i << ": (" << ThreadLocalStorage[i] << ")" << endl;
    if ( !ThreadLocalStorage[i] ) continue;
    ThreadLocalStorage[i]->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
    }
  }

template<class T>
T* vtkThreadLocal<T>::NewLocal( T *specificImpl )
  {
  vtkSMPThreadID tid = kaapic_get_thread_num();
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

template<class T>
T* vtkThreadLocal<T>::NewLocal( )
  {
  vtkSMPThreadID tid = kaapic_get_thread_num();
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

template<class T>
void vtkThreadLocal<T>::SetLocal( T* item )
  {
  vtkSMPThreadID tid = kaapic_get_thread_num();
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

template<class T>
T* vtkThreadLocal<T>::GetLocal( )
  {
  return this->ThreadLocalStorage[kaapic_get_thread_num()];
  }

template<class T> template<class Derived>
void vtkThreadLocal<T>::FillDerivedThreadLocal( vtkThreadLocal<Derived>* other )
  {
  typename vtkThreadLocalStorageContainer<T*>::iterator src = this->GetAll();
  for ( typename vtkThreadLocalStorageContainer<Derived*>::iterator it = other->GetAll();
        it != other->EndOfAll(); ++it, ++src )
    {
    if ( (*it) )
      (*it)->UnRegister(other);
    (*it) = Derived::SafeDownCast(*src);
    if ( (*it) )
      (*it)->Register(other);
    }
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
typename vtkThreadLocalStorageContainer<T*>::iterator vtkThreadLocal<T>::GetAll( vtkSMPThreadID skipThreads )
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
