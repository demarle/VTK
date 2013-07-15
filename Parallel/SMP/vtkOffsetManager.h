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
// .NAME vtkOffsetManager - ?
// .SECTION Description
// vtkOffsetManager is an internal functor used in mergeoperator
// used to deal with 4 cell arrays in vtkPolyData.

#ifndef _vtkOffsetManager_h_
#define _vtkOffsetManager_h_


#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"
#include <vector> // TODO: remove if possible No STL API in Core

class vtkCellArray;

class VTKPARALLELSMP_EXPORT vtkOffsetManager : public vtkObject
{
public:
  vtkTypeMacro(vtkOffsetManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkOffsetManager* New();

  // Description:
  // ?
  void InitManageValues ();

  // Description:
  // ?
  void ManageNextValue ( vtkCellArray* ca );

  // Description:
  // ?
  //TODO: vtk practice is to get/set the same named member
  vtkIdType GetNumberOfCells() { return CellsOffset; }

  // Description:
  // ?
  //TODO: vtk practice is to get/set the same named member
  vtkIdType GetNumberOfTuples() { return TuplesOffset; }

  // Description:
  // ?
  std::vector<vtkIdType>::iterator GetCellsOffset ( )
  {
    return cells.begin();
  }
  std::vector<vtkIdType>::iterator GetTuplesOffset ( )
  {
    return tuples.begin();
  }

  //TODO: Can be private?
  std::vector<vtkIdType> cells;
  std::vector<vtkIdType> tuples;
  vtkIdType CellsOffset;
  vtkIdType TuplesOffset;
  std::vector<vtkIdType>::iterator itCells;
  std::vector<vtkIdType>::iterator itTuples;

protected:
  vtkOffsetManager();
  ~vtkOffsetManager() { }

private:
  vtkOffsetManager(const vtkOffsetManager&); // Not implemented
  void operator=(const vtkOffsetManager&); // Not implemented
};

#endif //_vtkOffsetManager_h_
