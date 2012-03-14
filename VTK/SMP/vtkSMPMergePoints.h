#ifndef VTKSMPMERGEPOINTS_H
#define VTKSMPMERGEPOINTS_H

#include "vtkMergePoints.h"

class vtkPointData;
class vtkMutexLock;

class VTK_SMP_EXPORT vtkSMPMergePoints : public vtkMergePoints
{
  typedef vtkMutexLock *vtkMutexLockPtr;
  vtkMutexLock** LockTable;
  vtkMutexLock* CreatorLock;

protected:
  vtkSMPMergePoints();
  ~vtkSMPMergePoints();

public:
  vtkTypeMacro(vtkSMPMergePoints, vtkMergePoints);
  static vtkSMPMergePoints* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  int InitLockInsertion(vtkPoints *newPts, const double bounds[], vtkIdType estSize);
  void AddPointIdInBucket( vtkIdType ptId ) {}
  int SetUniquePoint( double x[3], vtkIdType& id ) {}
  void FixSizeOfPointArray();

  void Merge ( vtkSMPMergePoints* locator, vtkIdType idx, vtkPointData *outPd, vtkPointData *ptData, vtkIdList* idList );
  vtkIdType GetNumberOfIdInBucket ( vtkIdType idx );
  vtkIdType GetNumberOfBuckets();

  void FreeSearchStructure();
};

#endif // VTKSMPMERGEPOINTS_H
