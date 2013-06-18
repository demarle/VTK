#ifndef _vtkDummyMergeFunctor_h_
#define _vtkDummyMergeFunctor_h_

//used to initialize locator especially in the case of skip_threads

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkFunctor.h"
#include "vtkThreadLocal.h"

class vtkCellData;
class vtkCellArray;
class vtkIdList;
class vtkOffsetManager;
class vtkPointData;
class vtkPoints;
class vtkSMPMergePoints;

class VTKPARALLELSMP_EXPORT vtkDummyMergeFunctor : public vtkFunctor
{
  vtkDummyMergeFunctor ( const vtkDummyMergeFunctor& );
  void operator =( const vtkDummyMergeFunctor& );

protected:
  vtkIdType NumberOfCells;
  vtkIdType NumberOfPoints;

  vtkDummyMergeFunctor ();
  ~vtkDummyMergeFunctor ();

public:
  vtkTypeMacro(vtkDummyMergeFunctor,vtkFunctor);
  static vtkDummyMergeFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  vtkThreadLocal<vtkSMPMergePoints>* Locators;
  vtkThreadLocal<vtkPoints>* InPoints;

  vtkThreadLocal<vtkPointData>* InPd;
  vtkThreadLocal<vtkCellData>* InCd;

  vtkThreadLocal<vtkIdList>* Maps;

  vtkThreadLocal<vtkCellArray>* InVerts;
  vtkThreadLocal<vtkCellArray>* InLines;
  vtkThreadLocal<vtkCellArray>* InPolys;
  vtkThreadLocal<vtkCellArray>* InStrips;

  vtkSMPMergePoints* outputLocator;
  vtkCellArray* outputVerts;
  vtkCellArray* outputLines;
  vtkCellArray* outputPolys;
  vtkCellArray* outputStrips;
  vtkCellData* outputCd;
  vtkPointData* outputPd;

  vtkOffsetManager* vertOffset;
  vtkOffsetManager* lineOffset;
  vtkOffsetManager* polyOffset;
  vtkOffsetManager* stripOffset;

  void operator ()( vtkIdType pointId ) const;

  vtkIdType GetNumberOfCells() const { return NumberOfCells; }
  vtkIdType GetNumberOfPoints() const { return NumberOfPoints; }

  void InitializeNeeds( vtkThreadLocal<vtkSMPMergePoints>* _locator,
                        vtkThreadLocal<vtkPoints>* _points,
                        vtkSMPMergePoints* _outlocator,
                        vtkThreadLocal<vtkCellArray>* _inverts,
                        vtkCellArray* _outverts,
                        vtkThreadLocal<vtkCellArray>* _inlines,
                        vtkCellArray* _outlines,
                        vtkThreadLocal<vtkCellArray>* _inpolys,
                        vtkCellArray* _outpolys,
                        vtkThreadLocal<vtkCellArray>* _instrips,
                        vtkCellArray* _outstrips,
                        vtkThreadLocal<vtkPointData>* _inpd,
                        vtkPointData* _outpd,
                        vtkThreadLocal<vtkCellData>* _incd,
                        vtkCellData* _outcd );
};

#endif //_vtkDummyMergeFunctor_h_
