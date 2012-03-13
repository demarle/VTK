#ifndef VTKSMPMERGEPOINTS_H
#define VTKSMPMERGEPOINTS_H

#include "vtkMergePoints.h"

class vtkPointData;

class VTK_SMP_EXPORT vtkSMPMergePoints : public vtkMergePoints
{
  vtkIdType* TreatedTable;

protected:
  vtkSMPMergePoints();
  ~vtkSMPMergePoints();

public:
  vtkTypeMacro(vtkSMPMergePoints, vtkMergePoints);
  static vtkSMPMergePoints* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  int InitPointInsertion(vtkPoints *newPts, const double bounds[], vtkIdType estSize);
  void FixSizeOfPointArray();

  void Merge ( vtkSMPMergePoints* locator, vtkIdType idx, vtkPointData *outPd, vtkPointData *ptData, vtkIdList* idList );
  vtkIdType GetNumberOfIdInBucket ( vtkIdType idx );
  vtkIdType GetNumberOfBuckets();
  int MustTreatBucket ( vtkIdType idx );

  void FreeSearchStructure();
};

#endif // VTKSMPMERGEPOINTS_H
