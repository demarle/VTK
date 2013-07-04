/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPAlgorithm.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSMPAlgorithm.h"
#include "vtkSMPPipeline.h"
#include "vtkThreadLocal.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkMultiPieceDataSet.h"


//----------------------------------------------------------------------------
vtkSMPAlgorithm::vtkSMPAlgorithm()
{
}

//----------------------------------------------------------------------------
vtkSMPAlgorithm::~vtkSMPAlgorithm()
{
}

//----------------------------------------------------------------------------
void vtkSMPAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an SMP style Execute method.
int vtkSMPAlgorithm::ProcessRequest(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  if(!request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
    }
  int numPorts = this->GetNumberOfOutputPorts();
  vtkThreadLocal<vtkDataObject>** outputs =
        new vtkThreadLocal<vtkDataObject>*[numPorts];
  int i;
  for (i = 0; i < numPorts; ++i)
    {
    outputs[i] = vtkThreadLocal<vtkDataObject>::New();
    vtkInformation* outInfo = outputVector->GetInformationObject(i);
    if (outInfo->Has(vtkSMPPipeline::DATA_OBJECT_CONCRETE_TYPE()))
      {
      outputs[i]->SetSpecificClassName(outInfo->Get(vtkSMPPipeline::DATA_OBJECT_CONCRETE_TYPE()));
      }
    }

  if (this->RequestData(request, inputVector, outputVector, outputs))
    {
    for (i = 0; i < numPorts; ++i)
      {
      vtkMultiPieceDataSet* ds = vtkMultiPieceDataSet::SafeDownCast(
            outputVector->GetInformationObject(i)->Get(vtkDataObject::DATA_OBJECT()));
      ds->SetNumberOfPieces(1);
      unsigned int pieceNum = 0;
      for (vtkThreadLocal<vtkDataObject>::iterator outIter = outputs[i]->Begin();
           outIter != outputs[i]->End(); ++outIter)
        {
        ds->SetPiece(pieceNum++, *outIter);
        }

      outputs[i]->FastDelete();
      }
    delete [] outputs;

    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
// The execute method created by the subclass.
int vtkSMPAlgorithm::RequestData(
  vtkInformation* request,
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector* outputVector,
  vtkThreadLocal<vtkDataObject>** vtkNotUsed( outputData ))
{
  return 1;
}

//----------------------------------------------------------------------------
void vtkSMPAlgorithm::SetExecutive(vtkExecutive *executive)
  {
  if (executive->IsA("vtkSMPPipeline"))
    {
    this->Superclass::SetExecutive(executive);
    }
  else
    {
    vtkWarningMacro(<< executive->GetClassName()
                    << " is not a suitable executive"
                    << " for vtkSMPAlgorithm filters."
                    << " Nothing set.");
    }
  }

//----------------------------------------------------------------------------
vtkExecutive* vtkSMPAlgorithm::CreateDefaultExecutive()
  {
  if (vtkAlgorithm::DefaultExecutivePrototype &&
      vtkAlgorithm::DefaultExecutivePrototype->IsA("vtkSMPPipeline"))
    {
    return vtkAlgorithm::DefaultExecutivePrototype->NewInstance();
    }
  return vtkSMPPipeline::New();
  }
