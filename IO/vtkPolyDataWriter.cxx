/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPolyDataWriter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPolyDataWriter.h"

#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
# include <unistd.h> /* unlink */
#else
# include <io.h> /* unlink */
#endif

vtkCxxRevisionMacro(vtkPolyDataWriter, "1.24");
vtkStandardNewMacro(vtkPolyDataWriter);

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkPolyDataWriter::SetInput(vtkPolyData *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkPolyData *vtkPolyDataWriter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkPolyData *)(this->Inputs[0]);
}

void vtkPolyDataWriter::WriteData()
{
  ostream *fp;
  vtkPolyData *input = this->GetInput();

  vtkDebugMacro(<<"Writing vtk polygonal data...");

  if ( !(fp=this->OpenVTKFile()) || !this->WriteHeader(fp) )
    {
    if (fp)
      {
      if(this->FileName)
        {
        vtkErrorMacro("Ran out of disk space; deleting file: "
                      << this->FileName);
        this->CloseVTKFile(fp);
        unlink(this->FileName);
        }
      else
        {
        this->CloseVTKFile(fp);
        vtkErrorMacro("Could not read memory header. ");
        }
      }
    return;
    }
  //
  // Write polygonal data specific stuff
  //
  *fp << "DATASET POLYDATA\n"; 
  
  //
  // Write data owned by the dataset
  int errorOccured = 0;
  if (!this->WriteDataSetData(fp, input))
    {
    errorOccured = 1;
    }
  if (!errorOccured && !this->WritePoints(fp, input->GetPoints()))
    {
    errorOccured = 1;
    }

  if (!errorOccured && input->GetVerts())
    {
    if (!this->WriteCells(fp, input->GetVerts(),"VERTICES"))
      {
      errorOccured = 1;
      }
    }
  if (!errorOccured && input->GetLines())
    {
    if (!this->WriteCells(fp, input->GetLines(),"LINES"))
      {
      errorOccured = 1;
      }
    }
  if (!errorOccured && input->GetPolys())
    {
    if (!this->WriteCells(fp, input->GetPolys(),"POLYGONS"))
      {
      errorOccured = 1;
      }
    }
  if (!errorOccured && input->GetStrips())
    {
    if (!this->WriteCells(fp, input->GetStrips(),"TRIANGLE_STRIPS"))
      {
      errorOccured = 1;
      }
    }

  if (!errorOccured && !this->WriteCellData(fp, input))
    {
    errorOccured = 1;
    }
  if (!errorOccured && !this->WritePointData(fp, input))
    {
    errorOccured = 1;
    }

  if(errorOccured)
    {
    if(this->FileName)
      {
      vtkErrorMacro("Ran out of disk space; deleting file: " << this->FileName);
      this->CloseVTKFile(fp);
      unlink(this->FileName);
      }
    else
      {
      vtkErrorMacro("Error writting data set to memory");
      this->CloseVTKFile(fp);
      }
    return;
    }
  this->CloseVTKFile(fp);
}

void vtkPolyDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
