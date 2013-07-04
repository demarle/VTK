/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelPointMerger.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParallelPointMerger - !!!!
// .SECTION Description
// !!!!

#ifndef _vtkParallelPointMerger_h_
#define _vtkParallelPointMerger_h_

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkTask.h"

class vtkDummyMergeFunctor;
class vtkSMPMergePoints;

class VTKPARALLELSMP_EXPORT vtkParallelPointMerger : public vtkTask
{
public:
  vtkTypeMacro(vtkParallelPointMerger,vtkTask);
  static vtkParallelPointMerger* New();
  void PrintSelf(ostream &os, vtkIndent indent);
  void Execute( vtkSMPMergePoints* locator ) const;

  void SetUsefullData(vtkDummyMergeFunctor* f, vtkIdType** t);

protected:
  vtkParallelPointMerger();
  ~vtkParallelPointMerger();

  int MustTreatBucket( vtkIdType idx ) const;
  vtkIdType** TreatedTable;
  vtkDummyMergeFunctor* self;

private:
  vtkParallelPointMerger(const vtkParallelPointMerger&); // Not implemented
  void operator=(const vtkParallelPointMerger&); // Not implemented
};

#endif //_vtkParallelPointMerger_h_
