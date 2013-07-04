/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLockPointMerger.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLockPointMerger - !!!!
// .SECTION Description
// !!!!

#ifndef _vtkLockPointMerger_h_
#define _vtkLockPointMerger_h_

//extra synchonization locks for case when you lack allocators per CPU

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkFunctor.h"

class vtkPoints;
class vtkPointData;
class vtkDummyMergeFunctor;

class VTKPARALLELSMP_EXPORT vtkLockPointMerger : public vtkFunctor
{
public:
  vtkDummyMergeFunctor* Functor;
  vtkIdType NumberOfPointsFirstThread;

  vtkTypeMacro(vtkLockPointMerger,vtkFunctor);
  static vtkLockPointMerger* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void operator()( vtkIdType id ) const;

protected:
  vtkLockPointMerger() {}
  ~vtkLockPointMerger() {}
private:
  vtkLockPointMerger(const vtkLockPointMerger&); // Not implemented
  void operator=(const vtkLockPointMerger&); // Not implemented
};

#endif //_vtkLockPointMerger_h_
