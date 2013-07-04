/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOffsetManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOffsetManager - !!!!
// .SECTION Description
// !!!!

#ifndef _vtkOffsetManager_h_
#define _vtkOffsetManager_h_

//internal functor used in mergeoperator used to deal with 4 cell arrays in vtkPolyData

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"
#include <vector> // TODO: remove if possible

class vtkCellArray;

class VTKPARALLELSMP_EXPORT vtkOffsetManager : public vtkObject
{
  std::vector<vtkIdType> cells;
  std::vector<vtkIdType> tuples;
  vtkIdType CellsOffset;
  vtkIdType TuplesOffset;
  std::vector<vtkIdType>::iterator itCells;
  std::vector<vtkIdType>::iterator itTuples;

protected:
  vtkOffsetManager();
  ~vtkOffsetManager() { }
public:
  vtkTypeMacro(vtkOffsetManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkOffsetManager* New();

  void InitManageValues ();

  void ManageNextValue ( vtkCellArray* ca );

  vtkIdType GetNumberOfCells() { return CellsOffset; }
  vtkIdType GetNumberOfTuples() { return TuplesOffset; }

  std::vector<vtkIdType>::iterator GetCellsOffset ( ) { return cells.begin(); }
  std::vector<vtkIdType>::iterator GetTuplesOffset ( ) { return tuples.begin(); }

private:
  vtkOffsetManager(const vtkOffsetManager&); // Not implemented
  void operator=(const vtkOffsetManager&); // Not implemented
};

#endif //_vtkOffsetManager_h_
