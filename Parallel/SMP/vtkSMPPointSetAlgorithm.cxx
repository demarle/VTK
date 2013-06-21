/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPPointSetAlgorithm.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMPPointSetAlgorithm.h"

#include "vtkCommand.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(vtkSMPPointSetAlgorithm);

//----------------------------------------------------------------------------
// Instantiate object so that cell data is not passed to output.
vtkSMPPointSetAlgorithm::vtkSMPPointSetAlgorithm()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkPointSet* vtkSMPPointSetAlgorithm::GetOutput()
{
  return this->GetOutput(0);
}

//----------------------------------------------------------------------------
vtkPointSet* vtkSMPPointSetAlgorithm::GetOutput(int port)
{
  return vtkPointSet::SafeDownCast(this->GetOutputDataObject(port));
}

//----------------------------------------------------------------------------
// Get the output as vtkPolyData.
vtkPolyData *vtkSMPPointSetAlgorithm::GetPolyDataOutput()
{
  return vtkPolyData::SafeDownCast(this->GetOutput());
}

//----------------------------------------------------------------------------
// Get the output as vtkStructuredGrid.
vtkStructuredGrid *vtkSMPPointSetAlgorithm::GetStructuredGridOutput()
{
  return vtkStructuredGrid::SafeDownCast(this->GetOutput());
}

//----------------------------------------------------------------------------
// Get the output as vtkUnstructuredGrid.
vtkUnstructuredGrid *vtkSMPPointSetAlgorithm::GetUnstructuredGridOutput()
{
  return vtkUnstructuredGrid::SafeDownCast(this->GetOutput());
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::SetInputData(vtkDataObject* input)
{
  this->SetInputData(0, input);
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::SetInputData(int index, vtkDataObject* input)
{
  this->SetInputDataInternal(index, input);
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::SetInputData(vtkPointSet* input)
{
  this->SetInputData(0, static_cast<vtkDataObject*>(input));
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::SetInputData(int index, vtkPointSet* input)
{
  this->SetInputData(index, static_cast<vtkDataObject*>(input));
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::AddInputData(vtkDataObject* input)
{
  this->AddInputData(0, input);
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::AddInputData(int index, vtkDataObject* input)
{
  this->AddInputDataInternal(index, input);
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::AddInputData(vtkPointSet* input)
{
  this->AddInputData(0, static_cast<vtkDataObject*>(input));
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::AddInputData(int index, vtkPointSet* input)
{
  this->AddInputData(index, static_cast<vtkDataObject*>(input));
}

//----------------------------------------------------------------------------
vtkDataObject* vtkSMPPointSetAlgorithm::GetInput()
{
  return this->GetExecutive()->GetInputData(0, 0);
}

//----------------------------------------------------------------------------
int vtkSMPPointSetAlgorithm::ProcessRequest(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // create the output
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
    return this->RequestDataObject(request, inputVector, outputVector);
    }

  // execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    return this->ExecuteInformation(request, inputVector, outputVector);
    }

  // set update extent
 if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    return this->ComputeInputUpdateExtent(request, inputVector, outputVector);
    }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkSMPPointSetAlgorithm::RequestDataObject(
  vtkInformation*,
  vtkInformationVector** inputVector ,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  if (!inInfo)
    {
    return 0;
    }
  vtkPointSet *input = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (input)
    {
    // for each output
    for(int i=0; i < this->GetNumberOfOutputPorts(); ++i)
      {
      vtkInformation* info = outputVector->GetInformationObject(i);
      vtkPointSet *output = vtkPointSet::SafeDownCast(
        info->Get(vtkDataObject::DATA_OBJECT()));

      if (!output || !output->IsA(input->GetClassName()))
        {
        output = input->NewInstance();
        info->Set(vtkDataObject::DATA_OBJECT(), output);
        output->Delete();
        }
      }
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkSMPPointSetAlgorithm::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  // now add our info
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPointSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkSMPPointSetAlgorithm::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkSMPPointSetAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
