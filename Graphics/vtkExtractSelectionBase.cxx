/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractSelectionBase.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkExtractSelectionBase.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkExtractSelectionBase, "1.1");
//----------------------------------------------------------------------------
vtkExtractSelectionBase::vtkExtractSelectionBase()
{
  this->PreserveTopology = 0;
  this->SetNumberOfInputPorts(2);
}

//----------------------------------------------------------------------------
vtkExtractSelectionBase::~vtkExtractSelectionBase()
{
}

//----------------------------------------------------------------------------
int vtkExtractSelectionBase::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port==0)
    {
    // Can work with composite datasets.
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet"); 
    }
  else
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

//----------------------------------------------------------------------------
// Needed because parent class sets output type to input type
// and we sometimes want to change it to make an UnstructuredGrid regardless of
// input type
int vtkExtractSelectionBase::RequestDataObject(
  vtkInformation*,
  vtkInformationVector** inputVector ,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  if (!inInfo)
    {
    return 0;
    }

  vtkDataSet *input = vtkDataSet::GetData(inInfo);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (input)
    {
    int passThrough = this->PreserveTopology? 1 : 0;

    vtkDataSet *output = vtkDataSet::GetData(outInfo);
    if (!output ||
      (passThrough && !output->IsA(input->GetClassName())) ||
      (!passThrough && !output->IsA("vtkUnstructuredGrid")))
      {
      vtkDataSet* newOutput = NULL;
      if (!passThrough)
        {
        // The mesh will be modified. 
        newOutput = vtkUnstructuredGrid::New();
        }
      else
        {
        // The mesh will not be modified.
        newOutput = input->NewInstance();
        }
      newOutput->SetPipelineInformation(outInfo);
      newOutput->Delete();
      this->GetOutputPortInformation(0)->Set(
        vtkDataObject::DATA_EXTENT_TYPE(), newOutput->GetExtentType());
      }
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkExtractSelectionBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PreserveTopology: " << this->PreserveTopology << endl;
}

