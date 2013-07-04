/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPMergePoints.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPMergePoints - !!!!
// .SECTION Description
// !!!!

#ifndef VTKSMPMERGEPOINTS_H
#define VTKSMPMERGEPOINTS_H

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkMergePoints.h"

class vtkPointData;
class vtkMutexLock;

class VTKPARALLELSMP_EXPORT vtkSMPMergePoints : public vtkMergePoints
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

  // Replace InitPointInsertion for
  int InitLockInsertion(vtkPoints *newPts, const double bounds[6], vtkIdType estSize);
  void AddPointIdInBucket( vtkIdType ptId );
  int SetUniquePoint( const double x[3], vtkIdType& id );

  void Merge ( vtkSMPMergePoints* locator, vtkIdType idx, vtkPointData *outPd, vtkPointData *ptData, vtkIdList* idList );

  void FixSizeOfPointArray();
  virtual vtkIdType LocateBucketThatPointIsIn ( double x, double y, double z );
  vtkIdType GetNumberOfIdInBucket ( vtkIdType idx );
  vtkIdType GetNumberOfBuckets();

  void PrintSizeOfThis();

  void FreeSearchStructure();

private:
  vtkSMPMergePoints(const vtkSMPMergePoints&); // Not implemented
  void operator=(const vtkSMPMergePoints&); // Not implemented
};

#endif // VTKSMPMERGEPOINTS_H
