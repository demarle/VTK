/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBlankStructuredGrid.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBlankStructuredGrid.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkBlankStructuredGrid, "1.6");
vtkStandardNewMacro(vtkBlankStructuredGrid);

// Construct object to extract all of the input data.
vtkBlankStructuredGrid::vtkBlankStructuredGrid()
{
  this->MinBlankingValue = VTK_LARGE_FLOAT;
  this->MaxBlankingValue = VTK_LARGE_FLOAT;
  this->ArrayName = NULL;
  this->ArrayId = -1;
  this->Component = 0;
}

vtkBlankStructuredGrid::~vtkBlankStructuredGrid()
{
  if ( this->ArrayName )
    {
    delete [] this->ArrayName;
    this->ArrayName = NULL;
    }
}


template <class T>
void vtkBlankStructuredGridExecute(vtkBlankStructuredGrid *vtkNotUsed(self),
                                   T *dptr, int numPts, int numComp,
                                   int comp, float min, float max,
                                   vtkUnsignedCharArray *blanking)
{
  T compValue;
  dptr += comp;

  for ( int ptId=0; ptId < numPts; ptId++, dptr+=numComp)
    {
    compValue = *dptr;
    if ( compValue >= min && compValue <= max )
      {
      blanking->SetValue(ptId,0); //make it invisible
      }
    else
      {
      blanking->SetValue(ptId,1);
      }
    }
}


void vtkBlankStructuredGrid::Execute()
{
  vtkStructuredGrid *input= this->GetInput();
  vtkPointData *pd=input->GetPointData();
  vtkCellData *cd=input->GetCellData();
  vtkStructuredGrid *output= this->GetOutput();
  vtkPointData *outPD=output->GetPointData();
  vtkCellData *outCD=output->GetCellData();
  int numPts = input->GetNumberOfPoints();
  vtkDataArray *dataArray=NULL;
  int numComp;

  vtkDebugMacro(<< "Blanking Grid");

  // Pass input to output
  //
  output->CopyStructure(input);
  outPD->PassData(pd);
  outCD->PassData(cd);

  // Get the appropriate data array
  //
  if ( this->ArrayName != NULL )
    {
    dataArray = pd->GetArray(this->ArrayName);
    }
  else if ( this->ArrayId >= 0 )
    {
    dataArray = pd->GetArray(this->ArrayId);
    }
    
  if ( !dataArray || 
       (numComp=dataArray->GetNumberOfComponents()) <= this->Component )
    {
    vtkWarningMacro(<<"Data array not found");
    return;
    }
  void *dptr = dataArray->GetVoidPointer(0);
  
  // Loop over the data array setting anything within the data range specified
  // to be blanked.
  //
  vtkUnsignedCharArray *blanking = vtkUnsignedCharArray::New();
  blanking->SetNumberOfValues(numPts);

  // call templated function
  switch (dataArray->GetDataType())
    {
    vtkTemplateMacro8(vtkBlankStructuredGridExecute, this, (VTK_TT *)(dptr), numPts,
                      numComp, this->Component, this->MinBlankingValue, 
                      this->MaxBlankingValue, blanking);
    default:
      break;
    }

  // Clean up and get out
  output->SetPointVisibility(blanking);
  blanking->Delete();
  output->BlankingOn();
}


void vtkBlankStructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Min Blanking Value: " << this->MinBlankingValue << "\n";
  os << indent << "Max Blanking Value: " << this->MaxBlankingValue << "\n";
  os << indent << "Array Name: ";
  if ( this->ArrayName )
    {
    os << this->ArrayName << "\n";
    }
  else
    {
    os << "(none)\n";
    }
  os << indent << "Array ID: " << this->ArrayId << "\n";
  os << indent << "Component: " << this->Component << "\n";
  
}


