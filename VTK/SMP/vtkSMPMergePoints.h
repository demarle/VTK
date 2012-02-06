#ifndef VTKSMPMERGEPOINTS_H
#define VTKSMPMERGEPOINTS_H

#include "vtkMergePoints.h"

class vtkMutexLock;

class vtkSMPMergePoints : public vtkMergePoints
{
  vtkMutexLock** LockTable;
  vtkMutexLock* CreatorLock;

protected:
  vtkSMPMergePoints();
  ~vtkSMPMergePoints();

public:
  vtkTypeMacro(vtkSMPMergePoints, vtkMergePoints);
  static vtkSMPMergePoints* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  int InitPointInsertion(vtkPoints *newPts, const double bounds[], vtkIdType estSize);
  int SetUniquePoint(const double x[], vtkIdType &ptId);

  void FreeSearchStructure();
};

#endif // VTKSMPMERGEPOINTS_H
