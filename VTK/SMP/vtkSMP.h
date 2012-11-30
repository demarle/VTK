#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkSMPImplementation.h"
#include "vtkObject.h"

class vtkPoints;
class vtkPointData;
class vtkCellArray;
class vtkCellData;
class vtkSMPMergePoints;
class vtkIdList;

class VTK_SMP_EXPORT vtkFunctor : public vtkObject
{
  vtkFunctor ( const vtkFunctor& );
  void operator =( const vtkFunctor& );

public:
  vtkTypeMacro(vtkFunctor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void operator () ( vtkIdType, vtkSMPThreadID ) const = 0;

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

  virtual void Init ( vtkSMPThreadID ) const = 0;
  bool ShouldInitialize( vtkSMPThreadID tid ) const;

protected:
  mutable vtkIdType* IsInitialized;

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

  void Execute( ... ) const {}

protected:
  vtkTask();
  ~vtkTask();
};

class VTK_SMP_EXPORT vtkParallelTree
{
public:
  virtual int TraverseNode( vtkIdType id, int lvl, vtkFunctor* function, vtkSMPThreadID tid ) const = 0;
  virtual void GetTreeSize ( int& max_level, vtkIdType& branching_factor ) const = 0;
};

namespace vtkSMP
{
  vtkSMPThreadID VTK_SMP_EXPORT GetNumberOfThreads( );

  class VTK_SMP_EXPORT vtkIdTypeThreadLocal : public vtkObject
    {
    protected:
      vtkIdTypeThreadLocal();
      ~vtkIdTypeThreadLocal();

    public:
      vtkTypeMacro( vtkIdTypeThreadLocal, vtkObject );
      static vtkIdTypeThreadLocal* New();

      void PrintSelf( ostream &os, vtkIndent indent );
      vtkThreadLocalStorageContainer<vtkIdType>::iterator GetAll();
      vtkThreadLocalStorageContainer<vtkIdType>::iterator EndOfAll();
      void SetLocal ( vtkSMPThreadID tid, vtkIdType value );
      vtkIdType GetLocal ( vtkSMPThreadID tid );
      vtkIdType GetLocal ();

    protected:
      vtkThreadLocalStorageContainer<vtkIdType>::type ThreadLocalStorage;
    };

  template<class T>
  class VTK_SMP_EXPORT vtkThreadLocal : public vtkObject
    {
    protected :
      vtkThreadLocal();
      ~vtkThreadLocal();

    public:
      vtkTypeMacro(vtkThreadLocal, vtkObject)
      static vtkThreadLocal<T>* New();

      void PrintSelf( ostream &os, vtkIndent indent );

      T* NewLocal ( vtkSMPThreadID tid, T* specificImpl );
      T* NewLocal ( vtkSMPThreadID tid );

      typename vtkThreadLocalStorageContainer<T*>::iterator GetOrCreateAll( T* specificImpl );
      typename vtkThreadLocalStorageContainer<T*>::iterator GetOrCreateAll( );

      typename vtkThreadLocalStorageContainer<T*>::iterator GetAll( vtkIdType skipThreads = 0 ) ;

      typename vtkThreadLocalStorageContainer<T*>::iterator EndOfAll( );

      void SetLocal ( vtkSMPThreadID tid, T* item );

      T* GetLocal( vtkSMPThreadID tid );

      T* GetLocal();

      template<class Derived>
      void FillDerivedThreadLocal ( vtkThreadLocal<Derived>* other );

    protected:
      typename vtkThreadLocalStorageContainer<T*>::type ThreadLocalStorage;
    };
  #include "vtkSMPThreadLocalStorage.h" // load runtime-specific definition of TLS

  // ForEach template : parallel loop over an iterator
  void VTK_SMP_EXPORT ForEach( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain = 0 );

  void VTK_SMP_EXPORT ForEach( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* f, int grain = 0 );

  template<class T>
  void VTK_SMP_EXPORT Parallel( const vtkTask* function,
                                typename vtkThreadLocalStorageContainer<T*>::iterator data1,
                                vtkSMPThreadID skipThreads = 1 );

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  void VTK_SMP_EXPORT Parallel( const vtkTask* function,
                                typename vtkThreadLocalStorageContainer<T1*>::iterator data1,
                                typename vtkThreadLocalStorageContainer<T2*>::iterator data2,
                                typename vtkThreadLocalStorageContainer<T3*>::iterator data3,
                                typename vtkThreadLocalStorageContainer<T4*>::iterator data4,
                                typename vtkThreadLocalStorageContainer<T5*>::iterator data5,
                                typename vtkThreadLocalStorageContainer<T6*>::iterator data6,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset1,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset2,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset3,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset4,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset5,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset6,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset7,
                                vtkThreadLocalStorageContainer<vtkIdType>::iterator offset8,
                                vtkSMPThreadID skipThreads = 1 );

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

}

#endif //__vtkSMP_h__
