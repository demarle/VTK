template<class T>
vtkThreadLocal<T>::vtkThreadLocal() :
    vtkObject(), ThreadLocalStorage(GetNumberOfThreads())
  {
  }

template<class T>
vtkThreadLocal<T>::~vtkThreadLocal()
  {
  for ( typename vtkThreadLocalStorageContainer<T>::type::iterator it = ThreadLocalStorage.begin();
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
T* vtkThreadLocal<T>::NewLocal( vtkSMPThreadID tid, T *specificImpl )
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

template<class T>
T* vtkThreadLocal<T>::NewLocal( vtkSMPThreadID tid )
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

template<class T>
void vtkThreadLocal<T>::SetLocal( vtkSMPThreadID tid, T* item )
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

template<class T>
T* vtkThreadLocal<T>::GetLocal( vtkSMPThreadID tid )
  {
  return this->ThreadLocalStorage[tid];
  }

template<class T>
T* vtkThreadLocal<T>::GetLocal( )
  {
  return this->ThreadLocalStorage[omp_get_thread_num()];
  }

template<class T> template<class Derived>
void vtkThreadLocal<T>::FillDerivedThreadLocal( vtkThreadLocal<Derived>* other )
  {
  for ( typename vtkThreadLocalStorageContainer<T>::type::size_type i = 0; i < ThreadLocalStorage.size(); ++i )
    {
    other->SetLocal( i, Derived::SafeDownCast(ThreadLocalStorage[i]) );
    }
  }

template<class T>
vtkThreadLocal<T>* vtkThreadLocal<T>::New()
  {
  return new vtkThreadLocal<T>();
  }
