#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkSMPImplementation.h"
#include "vtkInstantiator.h"
#include "vtkObject.h"
#include <vector>
#include <typeinfo>

class vtkPoints;
class vtkPointData;
class vtkCellArray;
class vtkCellData;
class vtkSMPMergePoints;
class vtkIdList;
namespace vtkSMP { class vtkIdTypeThreadLocal; }

class VTK_SMP_EXPORT vtkFunctor : public vtkObject
{
  vtkFunctor ( const vtkFunctor& );
  void operator =( const vtkFunctor& );

public:
  vtkTypeMacro(vtkFunctor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void operator () ( vtkIdType ) const = 0;

protected:
  vtkFunctor();
  ~vtkFunctor();
};

class VTK_SMP_EXPORT vtkFunctorInitialisable : public vtkFunctor
{
  vtkFunctorInitialisable ( const vtkFunctorInitialisable& );
  void operator =( const vtkFunctorInitialisable& );

public:
  vtkTypeMacro(vtkFunctorInitialisable,vtkFunctor);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Init ( ) const = 0;
  bool ShouldInitialize( ) const;

protected:
  void Initialized( ) const;
  mutable vtkstd::vector<vtkIdType> IsInitialized;

  vtkFunctorInitialisable();
  ~vtkFunctorInitialisable();
};

class VTK_SMP_EXPORT vtkTask : public vtkObjectBase
{
  vtkTask(const vtkTask&);
  void operator =(const vtkTask&);

public:
  vtkTypeMacro(vtkTask, vtkObjectBase);
  void PrintSelf(ostream &os, vtkIndent indent);

  void Execute( ... ) const
    {
    cout << "Shouldn't be invocked." << endl;
    }
  virtual void Execute( vtkSMPMergePoints* ) const {}
  virtual void Execute( vtkIdList* map,
                        vtkCellData* clData,
                        vtkCellArray* verts,
                        vtkCellArray* lines,
                        vtkCellArray* polys,
                        vtkCellArray* strips,
                        vtkIdType vertCellOffset,
                        vtkIdType vertTupleOffset,
                        vtkIdType lineCellOffset,
                        vtkIdType lineTupleOffset,
                        vtkIdType polyCellOffset,
                        vtkIdType polyTupleOffset,
                        vtkIdType stripCellOffset,
                        vtkIdType stripTupleOffset ) const {}
  virtual void Execute( ) {}

protected:
  vtkTask();
  ~vtkTask();
};

class VTK_SMP_EXPORT vtkParallelTree
{
public:
  virtual int TraverseNode( vtkIdType id, int lvl, vtkFunctor* function ) const = 0;
  virtual void GetTreeSize ( int& max_level, vtkIdType& branching_factor ) const = 0;
};

namespace vtkSMP
{
  int InternalGetNumberOfThreads();
  int InternalGetTid();

  template<class T>
  class VTK_SMP_EXPORT vtkThreadLocal : public vtkObject
    {
    protected :
      vtkThreadLocal() : vtkObject(), ThreadLocalStorage(InternalGetNumberOfThreads(), NULL), Specific("") {}
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
        T* item = specificImpl->NewInstance();
        this->SetLocal( item );
        if(item) item->Delete();

        return item;
        }

      T* NewLocal ( )
        {
        if ( this->Specific != "" )
          {
          return this->NewLocal(this->Specific.c_str());
          }
        T* item = static_cast<T*>(T::New());
        this->SetLocal( item );
        if(item) item->Delete();

        return item;
        }

      T* NewLocal ( const char* type )
        {
        vtkObject* obj = vtkInstantiator::CreateInstance(type);
        T* item = T::SafeDownCast(obj);
        if (item)
          {
          this->SetLocal(item);
          item->Delete();
          return item;
          }
        this->SetLocal(0);
        if (obj) obj->Delete();
        return 0;
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

      void SetSpecificClassName( const char* type )
        {
        this->Specific = vtkstd::string(type);
        }

      void SetLocal ( T* item )
        {
        int tid = InternalGetTid();
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
        T* item = this->ThreadLocalStorage[InternalGetTid()];
        if (item) return item;
        return this->NewLocal();
        }

      template<class Derived>
      Derived* GetLocal()
        {
        return Derived::SafeDownCast(this->ThreadLocalStorage[InternalGetTid()]);
        }

      template<class Derived>
      void FillDerivedThreadLocal( vtkThreadLocal<Derived>* other )
        {
        T* elem;
        iterator src = ThreadLocalStorage.begin();
        for ( typename vtkSMP::vtkThreadLocal<Derived>::iterator it = other->Begin();
              it != other->End(); ++it, ++src )
          {
          if ( (elem = *it) ) elem->UnRegister(other);
          Derived* d = (*it) = Derived::SafeDownCast(*src);
          if ( d ) d->Register(other);
          }
        }

    protected:
      vtkstd::vector<T*> ThreadLocalStorage;
      vtkstd::string Specific;
    };

  // ForEach template : parallel loop over an iterator
  void VTK_SMP_EXPORT ForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain = 0 );

  void VTK_SMP_EXPORT ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* f, int grain = 0 );

  void VTK_SMP_EXPORT StaticForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op );

  void VTK_SMP_EXPORT StaticForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op );

  template<class T>
  void VTK_SMP_EXPORT Parallel( const vtkTask* function,
                                typename vtkSMP::vtkThreadLocal<T>::iterator data1,
                                vtkIdType skipThreads = 1 );

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  void VTK_SMP_EXPORT Parallel( const vtkTask* function,
                                typename vtkSMP::vtkThreadLocal<T1>::iterator data1,
                                typename vtkSMP::vtkThreadLocal<T2>::iterator data2,
                                typename vtkSMP::vtkThreadLocal<T3>::iterator data3,
                                typename vtkSMP::vtkThreadLocal<T4>::iterator data4,
                                typename vtkSMP::vtkThreadLocal<T5>::iterator data5,
                                typename vtkSMP::vtkThreadLocal<T6>::iterator data6,
                                vtkstd::vector<vtkIdType>::iterator offset1,
                                vtkstd::vector<vtkIdType>::iterator offset2,
                                vtkstd::vector<vtkIdType>::iterator offset3,
                                vtkstd::vector<vtkIdType>::iterator offset4,
                                vtkstd::vector<vtkIdType>::iterator offset5,
                                vtkstd::vector<vtkIdType>::iterator offset6,
                                vtkstd::vector<vtkIdType>::iterator offset7,
                                vtkstd::vector<vtkIdType>::iterator offset8,
                                vtkIdType skipThreads = 1 );

  void VTK_SMP_EXPORT Traverse( const vtkParallelTree* Tree, vtkFunctor* func );

  void VTK_SMP_EXPORT MergePoints( vtkPoints* outPoints, vtkThreadLocal<vtkPoints>* inPoints, const double bounds[6],
                                   vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
                                   vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
                                   vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
                                   vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
                                   vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
                                   vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData, int SkipThreads );

  void VTK_SMP_EXPORT MergePoints( vtkSMPMergePoints* outPoints, vtkThreadLocal<vtkSMPMergePoints>* inPoints,
                                   vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
                                   vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
                                   vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
                                   vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
                                   vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
                                   vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData, int SkipThreads );

  class VTK_SMP_EXPORT vtkSpawnTasks : public vtkObject {
    protected:
      vtkSpawnTasks();
      ~vtkSpawnTasks();

      void InternalSpawn( vtkTask* );

    public:
      vtkTypeMacro(vtkSpawnTasks,vtkObject);
      static vtkSpawnTasks* New();
      void PrintSelf(ostream &os, vtkIndent indent);

    private:
      vtkSpawnTasks( const vtkSpawnTasks& );
      void operator =( const vtkSpawnTasks& );
  };
}

#endif //__vtkSMP_h__
