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
// .NAME vtkSMPMergePoints - multithreaded vtkMergePoints
// .SECTION Description
// Just like parent, but uses the SMP framework to do work on many threads.
// This is used frequently in multithreaded algorithms because the many
// threads need to gather their results together at in the end.
#ifndef VTKSMPMERGEPOINTS_H
#define VTKSMPMERGEPOINTS_H

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkMergePoints.h"

class vtkPointData;
class vtkMutexLock;

class VTKPARALLELSMP_EXPORT vtkSMPMergePoints : public vtkMergePoints
{
public:
  vtkTypeMacro(vtkSMPMergePoints, vtkMergePoints);
  static vtkSMPMergePoints* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  // Replace InitPointInsertion for ?
  int InitLockInsertion(vtkPoints *newPts, const double bounds[6], vtkIdType estSize);

  //Description:
  //?
  void AddPointIdInBucket( vtkIdType ptId );

  //Description:
  //?
  int SetUniquePoint( const double x[3], vtkIdType& id );

  //Description:
  //?
  void Merge ( vtkSMPMergePoints* locator, vtkIdType idx,
      vtkPointData *outPd, vtkPointData *ptData, vtkIdList* idList );

  //Description:
  //?
  void FixSizeOfPointArray();

  //Description:
  //?
  virtual vtkIdType LocateBucketThatPointIsIn ( double x, double y, double z );

  //Description:
  //?
  vtkIdType GetNumberOfIdInBucket ( vtkIdType idx );

  //Description:
  //?
  vtkIdType GetNumberOfBuckets();

  //Description:
  //?
  void PrintSizeOfThis();

  //Description:
  //?
  void FreeSearchStructure();

  //TODO Can be private?
  typedef vtkMutexLock *vtkMutexLockPtr;
  vtkMutexLock** LockTable;
  vtkMutexLock* CreatorLock;

protected:
  vtkSMPMergePoints();
  ~vtkSMPMergePoints();

private:
  vtkSMPMergePoints(const vtkSMPMergePoints&); // Not implemented
  void operator=(const vtkSMPMergePoints&); // Not implemented
};

#endif // VTKSMPMERGEPOINTS_H
