#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"
#include <vector>

#include "vtkFunctorInitializable.h"

class vtkCellArray;
class vtkCellData;
class vtkFunctor;
class vtkFunctorIntializable;
class vtkParallelTree;
class vtkPointData;
class vtkPoints;
class vtkSMPMergePoints;
class vtkTask;

//======================================================================================
int vtkSMPInternalGetNumberOfThreads();
int vtkSMPInternalGetTid();

template<class T>
class VTKPARALLELSMP_EXPORT vtkThreadLocal : public vtkObject
{
 protected :
 vtkThreadLocal() : vtkObject(), ThreadLocalStorage(vtkSMPInternalGetNumberOfThreads(), NULL) {}
  ~vtkThreadLocal()
    {
    for ( iterator it = ThreadLocalStorage.begin();
          it != ThreadLocalStorage.end(); ++it )
      {
      if ( *it )
        (*it)->UnRegister( this );
      *it = 0;
      }
    ThreadLocalStorage.clear();
    }

 public:
  typedef typename vtkstd::vector<T*>::iterator iterator;

  vtkTypeMacro(vtkThreadLocal, vtkObject)
    static vtkThreadLocal<T>* New() { return new vtkThreadLocal<T>(); }

  void PrintSelf( ostream &os, vtkIndent indent )
  {
    this->Superclass::PrintSelf( os, indent );
    os << indent << "Class stored: " << typeid(T).name() << endl;
    os << indent << "Local storage: " << endl;
    size_t i = 0;
    for ( iterator it = ThreadLocalStorage.begin(); it != ThreadLocalStorage.end(); ++it, ++i )
      {
      os << indent.GetNextIndent() << "id " << i << ": (" << *it << ")" << endl;
      if ( *it ) (*it)->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
      }
  }

  T* NewLocal ( T* specificImpl )
  {
    int tid = vtkSMPInternalGetTid();
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

  T* NewLocal ( )
  {
    int tid = vtkSMPInternalGetTid();
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

  iterator Begin( vtkIdType startItem = 0 )
  {
    iterator value = ThreadLocalStorage.begin();
    while ( startItem )
      {
      ++value;
      --startItem;
      }
    return value;
  }

  iterator End( )
  {
    return ThreadLocalStorage.end();
  }

  void SetLocal ( T* item )
  {
    int tid = vtkSMPInternalGetTid();
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

  T* GetLocal()
  {
    return this->ThreadLocalStorage[vtkSMPInternalGetTid()];
  }

  template<class Derived>
    Derived* GetLocal()
    {
    return Derived::SafeDownCast(this->ThreadLocalStorage[vtkSMPInternalGetTid()]);
    }

  template<class Derived>
    void FillDerivedThreadLocal( vtkThreadLocal<Derived>* other )
    {
    T* elem;
    iterator src = ThreadLocalStorage.begin();
    for ( typename vtkThreadLocal<Derived>::iterator it = other->Begin();
          it != other->End(); ++it, ++src )
      {
      if ( (elem = *it) ) elem->UnRegister(other);
      Derived* d = (*it) = Derived::SafeDownCast(*src);
      if ( d ) d->Register(other);
      }
    }

 protected:
  vtkstd::vector<T*> ThreadLocalStorage;
};

//======================================================================================

// ForEach template : parallel loop over an iterator
void VTKPARALLELSMP_EXPORT vtkSMPForEachOp( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain = 0 );

void VTKPARALLELSMP_EXPORT vtkSMPForEachOp( vtkIdType first, vtkIdType last, const vtkFunctorInitializable* f, int grain = 0 );

template<class T>
void VTKPARALLELSMP_EXPORT vtkSMPParallelOp( const vtkTask* function,
                              typename vtkThreadLocal<T>::iterator data1,
                              vtkIdType skipThreads = 1 );

template<class T1, class T2, class T3, class T4, class T5, class T6>
void VTKPARALLELSMP_EXPORT vtkSMPParallelOp( const vtkTask* function,
                              typename vtkThreadLocal<T1>::iterator data1,
                              typename vtkThreadLocal<T2>::iterator data2,
                              typename vtkThreadLocal<T3>::iterator data3,
                              typename vtkThreadLocal<T4>::iterator data4,
                              typename vtkThreadLocal<T5>::iterator data5,
                              typename vtkThreadLocal<T6>::iterator data6,
                              vtkstd::vector<vtkIdType>::iterator offset1,
                              vtkstd::vector<vtkIdType>::iterator offset2,
                              vtkstd::vector<vtkIdType>::iterator offset3,
                              vtkstd::vector<vtkIdType>::iterator offset4,
                              vtkstd::vector<vtkIdType>::iterator offset5,
                              vtkstd::vector<vtkIdType>::iterator offset6,
                              vtkstd::vector<vtkIdType>::iterator offset7,
                              vtkstd::vector<vtkIdType>::iterator offset8,
                              vtkIdType skipThreads = 1 );

void VTKPARALLELSMP_EXPORT vtkSMPTraverseOp( const vtkParallelTree* Tree, vtkFunctor* func );

void VTKPARALLELSMP_EXPORT vtkSMPMergePointsOp( vtkPoints* outPoints, vtkThreadLocal<vtkPoints>* inPoints, const double bounds[6],
                                 vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
                                 vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
                                 vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
                                 vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
                                 vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
                                 vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData, int SkipThreads );

void VTKPARALLELSMP_EXPORT vtkSMPMergePointsOp( vtkSMPMergePoints* outPoints, vtkThreadLocal<vtkSMPMergePoints>* inPoints,
                                 vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
                                 vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
                                 vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
                                 vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
                                 vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
                                 vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData, int SkipThreads );

#endif //__vtkSMP_h__
