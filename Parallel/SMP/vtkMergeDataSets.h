#ifndef __vtkMergeDataSets_h__
#define __vtkMergeDataSets_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"

class vtkSMPMergePoints;
class vtkPointData;
class vtkPoints;
class vtkCellArray;
template<class T> class vtkThreadLocal<T>;

class VTKPARALLELSMP_EXPORT vtkMergeDataSets : public vtkObject
{
    vtkMergeDataSets(const vtkMergeDataSets&);
    void operator=(const vtkFunctor&);

  public:
    vtkTypeMacro(vtkMergeDataSets,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);
    static vtkMergeDataSets* New;

    vtkSetMacro(MasterThreadPopulatedOutput,int);
    vtkBooleanMacro(MasterThreadPopulatedOutput,int);

    void MergePolyData(
        vtkPoints* outPoints,
        vtkThreadLocal<vtkPoints>* inPoints,
        const double bounds[6],
        vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
        vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
        vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
        vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
        vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
        vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData);
    void MergePolyData(
        vtkSMPMergePoints* outPoints,
        vtkThreadLocal<vtkSMPMergePoints>* inPoints,
        vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
        vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
        vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
        vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
        vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
        vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData);

  protected:
    vtkMergeDataSets();
    ~vtkMergeDataSets();

    int MasterThreadPopulatedOutput;
    vtkIdType** TreatedTable;
};

#endif
