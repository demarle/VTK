/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOffsetManager.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOffsetManager.h"

#include "vtkCellArray.h"
#include "vtkObjectFactory.h"

extern int vtkSMPInternalGetNumberOfThreads();
extern int vtkSMPInternalGetTid();

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkOffsetManager);

//------------------------------------------------------------------------------
vtkOffsetManager::vtkOffsetManager() :
  vtkObject(),
  cells(vtkSMPInternalGetNumberOfThreads(), 0),
  tuples(vtkSMPInternalGetNumberOfThreads(), 0)
{ }

//------------------------------------------------------------------------------
void vtkOffsetManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Offsets for cells index: " << endl;
  for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < cells.size(); ++i )
    {
    os << indent.GetNextIndent() << "Id " << i << ": " << cells[i];
    }
  os << indent << "Offsets for tuples: " << endl;
  for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < tuples.size(); ++i )
    {
    os << indent.GetNextIndent() << "Id " << i << ": " << tuples[i];
    }
  os << indent << "Number of cells: " << CellsOffset << endl;
  os << indent << "Number of tuples: " << TuplesOffset << endl;
}

//------------------------------------------------------------------------------
void vtkOffsetManager::InitManageValues ()
{
  CellsOffset = 0;
  TuplesOffset = 0;
  itCells = cells.begin();
  itTuples = tuples.begin();
}

//------------------------------------------------------------------------------
void vtkOffsetManager::ManageNextValue ( vtkCellArray* ca )
{
  *itCells = CellsOffset;
  *itTuples = TuplesOffset;
  if ( ca )
    {
    CellsOffset += ca->GetNumberOfCells();
    TuplesOffset += ca->GetData()->GetNumberOfTuples();
    }
  ++itCells;
  ++itTuples;
}
