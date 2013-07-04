/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelCellMerger.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParallelCellMerger - !!!!
// .SECTION Description
// !!!!

#ifndef _vtkParallelCellMerger_h_
#define _vtkParallelCellMerger_h_

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkTask.h"

class vtkCellData;
class vtkCellArray;
class vtkDummyMergeFunctor;

class VTKPARALLELSMP_EXPORT vtkParallelCellMerger : public vtkTask
{
public:
  vtkDummyMergeFunctor* self;

  vtkTypeMacro(vtkParallelCellMerger,vtkTask);
  static vtkParallelCellMerger* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void Execute( vtkIdList* map,
                vtkCellData* clData,
                vtkCellArray* verts,
                vtkCellArray* lines,
                vtkCellArray* polys,
                vtkCellArray* strips,
                vtkIdType vertCellOffset,
                vtkIdType vertTupleOffset,
                vtkIdType lineCellOffset,
                vtkIdType lineTupleOffset,
                vtkIdType polyCellOffset,
                vtkIdType polyTupleOffset,
                vtkIdType stripCellOffset,
                vtkIdType stripTupleOffset ) const;
protected:
  vtkParallelCellMerger() {}
  ~vtkParallelCellMerger() {}
private:
  vtkParallelCellMerger(const vtkParallelCellMerger&); // Not implemented
  void operator =(const vtkParallelCellMerger&); // Not implemented
};

#endif //_vtkParallelCellMerger_h_
