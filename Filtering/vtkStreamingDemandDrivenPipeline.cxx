/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStreamingDemandDrivenPipeline.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkExtentTranslator.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInformationRequestKey.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkCxxRevisionMacro(vtkStreamingDemandDrivenPipeline, "1.34");
vtkStandardNewMacro(vtkStreamingDemandDrivenPipeline);

vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, CONTINUE_EXECUTING, Integer);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, EXACT_EXTENT, Integer);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, REQUEST_UPDATE_EXTENT, Request);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, MAXIMUM_NUMBER_OF_PIECES, Integer);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, UPDATE_EXTENT_INITIALIZED, Integer);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, UPDATE_PIECE_NUMBER, Integer);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, UPDATE_NUMBER_OF_PIECES, Integer);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, UPDATE_NUMBER_OF_GHOST_LEVELS, Integer);
vtkInformationKeyRestrictedMacro(vtkStreamingDemandDrivenPipeline, WHOLE_EXTENT, IntegerVector, 6);
vtkInformationKeyRestrictedMacro(vtkStreamingDemandDrivenPipeline, UPDATE_EXTENT, IntegerVector, 6);
vtkInformationKeyRestrictedMacro(vtkStreamingDemandDrivenPipeline,
                                 EXTENT_TRANSLATOR, ObjectBase,
                                 "vtkExtentTranslator");
vtkInformationKeyRestrictedMacro(vtkStreamingDemandDrivenPipeline, WHOLE_BOUNDING_BOX, DoubleVector, 6);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, TIME_STEPS, DoubleVector);
vtkInformationKeyMacro(vtkStreamingDemandDrivenPipeline, UPDATE_TIME_INDEX, Integer);

//----------------------------------------------------------------------------
class vtkStreamingDemandDrivenPipelineToDataObjectFriendship
{
public:
  static void Crop(vtkDataObject* obj)
    {
    obj->Crop();
    }
};

//----------------------------------------------------------------------------
vtkStreamingDemandDrivenPipeline::vtkStreamingDemandDrivenPipeline()
{
  this->ContinueExecuting = 0;
  this->UpdateExtentRequest = 0;
  this->LastPropogateUpdateExtentShortCircuited = 0;
}

//----------------------------------------------------------------------------
vtkStreamingDemandDrivenPipeline::~vtkStreamingDemandDrivenPipeline()
{
  if (this->UpdateExtentRequest)
    {
    this->UpdateExtentRequest->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkStreamingDemandDrivenPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::ProcessRequest(vtkInformation* request,
                 int forward,
                 vtkInformationVector** inInfoVec,
                 vtkInformationVector* outInfoVec)
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("ProcessRequest", request))
    {
    return 0;
    }

  // Look for specially supported requests.
  if(request->Has(REQUEST_UPDATE_EXTENT()))
    {
    // Get the output port from which the request was made.
    this->LastPropogateUpdateExtentShortCircuited = 1;
    int outputPort = -1;
    if(request->Has(FROM_OUTPUT_PORT()))
      {
      outputPort = request->Get(FROM_OUTPUT_PORT());
      }
    
    // Make sure the information on the output port is valid.
    if(!this->VerifyOutputInformation(outputPort,inInfoVec,outInfoVec))
      {
      return 0;
      }

    // If we need to execute, propagate the update extent.
    int result = 1;
    if(this->NeedToExecuteData(outputPort,inInfoVec,outInfoVec))
      {
      // Make sure input types are valid before algorithm does anything.
      if(!this->InputCountIsValid(inInfoVec) || 
         !this->InputTypeIsValid(inInfoVec))
        {
        return 0;
        }

      // Invoke the request on the algorithm.
      this->LastPropogateUpdateExtentShortCircuited = 0;
      result = this->CallAlgorithm(request, vtkExecutive::RequestUpstream,
                                   inInfoVec, outInfoVec);
      
      // Propagate the update extent to all inputs.
      if(result && forward)
        {
        result = this->ForwardUpstream(request);
        }
      result = 1;
      }
    return result;
    }

  if(request->Has(REQUEST_DATA()))
    {
    // Let the superclass handle the request first.
    if(this->Superclass::ProcessRequest(request, forward, 
                                        inInfoVec, outInfoVec))
      {
      // Crop the output if the exact extent flag is set.
      for(int i=0; i < outInfoVec->GetNumberOfInformationObjects(); ++i)
        {
        vtkInformation* info = outInfoVec->GetInformationObject(i);
        if(info->Has(EXACT_EXTENT()) && info->Get(EXACT_EXTENT()))
          {
          vtkDataObject* data = info->Get(vtkDataObject::DATA_OBJECT());
          vtkStreamingDemandDrivenPipelineToDataObjectFriendship::Crop(data);
          }
        }
      return 1;
      }
    return 0;
    }

  // Let the superclass handle other requests.
  return this->Superclass::ProcessRequest(request, forward, 
                                          inInfoVec, outInfoVec);
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::Update()
{
  return this->Superclass::Update();
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::Update(int port)
{
  if(!this->UpdateInformation())
    {
    return 0;
    }
  if(port >= -1 && port < this->Algorithm->GetNumberOfOutputPorts())
    {
    int retval = 1;
    // some streaming filters can request that the pipeline execute multiple
    // times for a single update
    do 
      {
      retval = retval && this->PropagateUpdateExtent(port);
      if (retval && !this->LastPropogateUpdateExtentShortCircuited)
        {
        retval = retval && this->UpdateData(port);
        }
      }
    while (this->ContinueExecuting);
    return retval;
    }
  else
    {
    return 1;
    }
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::UpdateWholeExtent()
{
  this->UpdateInformation();
  // if we have an output then set the UE to WE for it
  if (this->Algorithm->GetNumberOfOutputPorts())
    {
    this->SetUpdateExtentToWholeExtent
      (this->GetOutputInformation()->GetInformationObject(0));
    }
  // otherwise do it for the inputs
  else
    {
    // Loop over all input ports.
    for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
      {
      // Loop over all connections on this input port.
      int numInConnections = this->Algorithm->GetNumberOfInputConnections(i);
      for (int j=0; j<numInConnections; j++)
        {
        // Get the pipeline information for this input connection.
        vtkInformation* inInfo = this->GetInputInformation(i, j);
        this->SetUpdateExtentToWholeExtent(inInfo);
        }
      }
    }
  return this->Update();
}

//----------------------------------------------------------------------------
int
vtkStreamingDemandDrivenPipeline
::ExecuteInformation(vtkInformation* request,
                     vtkInformationVector** inInfoVec,
                     vtkInformationVector* outInfoVec)
{
  // Let the superclass make the request to the algorithm.
  if(this->Superclass::ExecuteInformation(request,inInfoVec,outInfoVec))
    {
    for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
      {
      vtkInformation* info = outInfoVec->GetInformationObject(i);
      vtkDataObject* data = info->Get(vtkDataObject::DATA_OBJECT());

      if (!data)
        {
        return 0;
        }
      // Set default maximum request.
      if(data->GetExtentType() == VTK_PIECES_EXTENT)
        {
        if(!info->Has(MAXIMUM_NUMBER_OF_PIECES()))
          {
          info->Set(MAXIMUM_NUMBER_OF_PIECES(), -1);
          }
        }
      else if(data->GetExtentType() == VTK_3D_EXTENT)
        {
        if(!info->Has(WHOLE_EXTENT()))
          {
          int extent[6] = {0,-1,0,-1,0,-1};
          info->Set(WHOLE_EXTENT(), extent, 6);
          }
        }

      // Make sure an update request exists.
      if(!info->Has(UPDATE_EXTENT_INITIALIZED()) ||
         !info->Get(UPDATE_EXTENT_INITIALIZED()))
        {
        // Request all data by default.
        this->SetUpdateExtentToWholeExtent
          (outInfoVec->GetInformationObject(i));
        }
      }
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void
vtkStreamingDemandDrivenPipeline
::CopyDefaultInformation(vtkInformation* request, int direction,
                         vtkInformationVector** inInfoVec,
                         vtkInformationVector* outInfoVec)
{
  // Let the superclass copy first.
  this->Superclass::CopyDefaultInformation(request, direction,
                                           inInfoVec, outInfoVec);
  
  if(request->Has(REQUEST_INFORMATION()))
    {
    if(this->GetNumberOfInputPorts() > 0)
      {
      if(vtkInformation* inInfo = inInfoVec[0]->GetInformationObject(0))
        {
        // Copy information from the first input to all outputs.
        for(int i=0; i < outInfoVec->GetNumberOfInformationObjects(); ++i)
          {
          vtkInformation* outInfo = outInfoVec->GetInformationObject(i);
          outInfo->CopyEntry(inInfo, WHOLE_BOUNDING_BOX());
          outInfo->CopyEntry(inInfo, WHOLE_EXTENT());
          outInfo->CopyEntry(inInfo, MAXIMUM_NUMBER_OF_PIECES());
          outInfo->CopyEntry(inInfo, EXTENT_TRANSLATOR());
          outInfo->CopyEntry(inInfo, TIME_STEPS());
          }
        }
      }

    // Setup default information for the outputs.
    for(int i=0; i < outInfoVec->GetNumberOfInformationObjects(); ++i)
      {
      vtkInformation* outInfo = outInfoVec->GetInformationObject(i);

      // The data object will exist because UpdateDataObject has already
      // succeeded. Except when this method is called by a subclass
      // that does not provide this key in certain cases.
      vtkDataObject* dataObject = outInfo->Get(vtkDataObject::DATA_OBJECT());
      if (!dataObject)
        {
        continue;
        }
      vtkInformation* dataInfo = dataObject->GetInformation();
      if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_PIECES_EXTENT)
        {
        if (!outInfo->Has(MAXIMUM_NUMBER_OF_PIECES()))
          {
          if (this->GetNumberOfInputPorts() > 0)
            {
            // must have structured input; MAXIMUM_NUMBER_OF_PIECES will
            // not be copied above (CopyEntry does nothing since key not set
            // in inInfo); set to -1
            outInfo->Set(MAXIMUM_NUMBER_OF_PIECES(), -1);
            }
          else
            {
            // Since most unstructured filters in VTK generate all their
            // data once, set the default maximum number of pieces to 1.
            outInfo->Set(MAXIMUM_NUMBER_OF_PIECES(), 1);
            }
          }
        }
      else if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_3D_EXTENT)
        {
        if(!outInfo->Has(EXTENT_TRANSLATOR()) ||
           !outInfo->Get(EXTENT_TRANSLATOR()))
          {
          // Create a default extent translator.
          vtkExtentTranslator* translator = vtkExtentTranslator::New();
          outInfo->Set(EXTENT_TRANSLATOR(), translator);
          translator->Delete();
          }
        }
      }
    }
  if(request->Has(REQUEST_UPDATE_EXTENT()))
    {
    // Get the output port from which to copy the extent.
    int outputPort = -1;
    if(request->Has(FROM_OUTPUT_PORT()))
      {
      outputPort = request->Get(FROM_OUTPUT_PORT());
      }

    // Setup default information for the inputs.
    if(outInfoVec->GetNumberOfInformationObjects() > 0)
      {
      // Copy information from the output port that made the request.
      // Since VerifyOutputInformation has already been called we know
      // there is output information with a data object.
      vtkInformation* outInfo =
        outInfoVec->GetInformationObject((outputPort >= 0)? outputPort : 0);
      vtkDataObject* outData = outInfo->Get(vtkDataObject::DATA_OBJECT());

      // Loop over all input ports.
      for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
        {
        // Loop over all connections on this input port.
        int numInConnections = inInfoVec[i]->GetNumberOfInformationObjects();
        for (int j=0; j<numInConnections; j++)
          {
          // Get the pipeline information for this input connection.
          vtkInformation* inInfo = inInfoVec[i]->GetInformationObject(j);

          // Copy the time request
          if ( outInfo->Has(UPDATE_TIME_INDEX()) )
            {
            inInfo->CopyEntry(outInfo, UPDATE_TIME_INDEX());
            }

          // Get the input data object for this connection.  It should
          // have already been created by the UpdateDataObject pass.
          vtkDataObject* inData = inInfo->Get(vtkDataObject::DATA_OBJECT());
          if(!inData)
            {
            vtkErrorMacro("Cannot copy default update request from output port "
                          << outputPort << " on algorithm "
                          << this->Algorithm->GetClassName()
                          << "(" << this->Algorithm << ") to input connection "
                          << j << " on input port " << i
                          << " because there is no data object.");
            continue;
            }

          // Consider all combinations of extent types.
          if(inData->GetExtentType() == VTK_PIECES_EXTENT)
            {
            if(outData->GetExtentType() == VTK_PIECES_EXTENT)
              {
              if (outInfo->Get(UPDATE_PIECE_NUMBER()) < 0)
                {
                return;
                }
              inInfo->CopyEntry(outInfo, UPDATE_PIECE_NUMBER());
              inInfo->CopyEntry(outInfo, UPDATE_NUMBER_OF_PIECES());
              inInfo->CopyEntry(outInfo, UPDATE_NUMBER_OF_GHOST_LEVELS());
              inInfo->CopyEntry(outInfo, UPDATE_EXTENT_INITIALIZED());
              }
            else if(outData->GetExtentType() == VTK_3D_EXTENT)
              {
              // The conversion from structrued requests to
              // unstrcutured requests is always to request the whole
              // extent.
              this->SetUpdateExtentToWholeExtent(inInfo);
              }
            }
          else if(inData->GetExtentType() == VTK_3D_EXTENT)
            {
            if(outData->GetExtentType() == VTK_PIECES_EXTENT)
              {
              int piece = outInfo->Get(UPDATE_PIECE_NUMBER());
              int numPieces = outInfo->Get(UPDATE_NUMBER_OF_PIECES());
              int ghostLevel = outInfo->Get(UPDATE_NUMBER_OF_GHOST_LEVELS());
              if (piece >= 0)
                {
                this->SetUpdateExtent(inInfo, piece, numPieces, ghostLevel);
                }
              }
            else if(outData->GetExtentType() == VTK_3D_EXTENT)
              {
              inInfo->CopyEntry(outInfo, UPDATE_EXTENT());
              inInfo->CopyEntry(outInfo, UPDATE_EXTENT_INITIALIZED());
              }
            }
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void
vtkStreamingDemandDrivenPipeline
::ResetPipelineInformation(int port, vtkInformation* info)
{
  this->Superclass::ResetPipelineInformation(port, info);
  info->Remove(WHOLE_EXTENT());
  info->Remove(MAXIMUM_NUMBER_OF_PIECES());
  info->Remove(EXTENT_TRANSLATOR());
  info->Remove(UPDATE_EXTENT_INITIALIZED());
  info->Remove(UPDATE_EXTENT());
  info->Remove(UPDATE_PIECE_NUMBER());
  info->Remove(UPDATE_NUMBER_OF_PIECES());
  info->Remove(UPDATE_NUMBER_OF_GHOST_LEVELS());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::PropagateUpdateExtent(int outputPort)
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("PropagateUpdateExtent", 0))
    {
    return 0;
    }

  // Range check.
  if(outputPort < -1 ||
     outputPort >= this->Algorithm->GetNumberOfOutputPorts())
    {
    vtkErrorMacro("PropagateUpdateExtent given output port index "
                  << outputPort << " on an algorithm with "
                  << this->Algorithm->GetNumberOfOutputPorts()
                  << " output ports.");
    return 0;
    }

  // Setup the request for update extent propagation.
  if (!this->UpdateExtentRequest)
    {
    this->UpdateExtentRequest = vtkInformation::New();
    this->UpdateExtentRequest->Set(REQUEST_UPDATE_EXTENT());
    // The request is forwarded upstream through the pipeline.
    this->UpdateExtentRequest->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);
    // Algorithms process this request before it is forwarded.
    this->UpdateExtentRequest->Set(vtkExecutive::ALGORITHM_BEFORE_FORWARD(), 1);
    }
  
  this->UpdateExtentRequest->Set(FROM_OUTPUT_PORT(), outputPort);

  // Send the request.
  return this->ProcessRequest(this->UpdateExtentRequest,1,this->GetInputInformation(),
                              this->GetOutputInformation());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::VerifyOutputInformation(int outputPort,
                          vtkInformationVector** inInfoVec,
                          vtkInformationVector* outInfoVec)
{
  // If no port is specified, check all ports.
  if(outputPort < 0)
    {
    for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
      {
      if(!this->VerifyOutputInformation(i,inInfoVec,outInfoVec))
        {
        return 0;
        }
      }
    return 1;
    }

  // Get the information object to check.
  vtkInformation* outInfo = outInfoVec->GetInformationObject(outputPort);

  // Make sure there is a data object.  It is supposed to be created
  // by the UpdateDataObject step.
  vtkDataObject* dataObject = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if(!dataObject)
    {
    vtkErrorMacro("No data object has been set in the information for "
                  "output port " << outputPort << ".");
    return 0;
    }

  // Check extents.
  vtkInformation* dataInfo = dataObject->GetInformation();
  if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_PIECES_EXTENT)
    {
    // For an unstructured extent, make sure the update request
    // exists.  We do not need to check if it is valid because
    // out-of-range requests produce empty data.
    if(!outInfo->Has(MAXIMUM_NUMBER_OF_PIECES()))
      {
      vtkErrorMacro("No maximum number of pieces has been set in the "
                    "information for output port " << outputPort
                    << " on algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ").");
      return 0;
      }
    if(!outInfo->Has(UPDATE_PIECE_NUMBER()))
      {
      vtkErrorMacro("No update piece number has been set in the "
                    "information for output port " << outputPort
                    << " on algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ").");
      return 0;
      }
    if(!outInfo->Has(UPDATE_NUMBER_OF_PIECES()))
      {
      vtkErrorMacro("No update number of pieces has been set in the "
                    "information for output port " << outputPort
                    << " on algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ").");
      return 0;
      }
    if(!outInfo->Has(UPDATE_NUMBER_OF_GHOST_LEVELS()))
      {
      // Use zero ghost levels by default.
      outInfo->Set(UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
      }
    }
  else if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_3D_EXTENT)
    {
    // For a structured extent, make sure the update request
    // exists.
    if(!outInfo->Has(WHOLE_EXTENT()))
      {
      vtkErrorMacro("No whole extent has been set in the "
                    "information for output port " << outputPort
                    << " on algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ").");
      return 0;
      }
    if(!outInfo->Has(UPDATE_EXTENT()))
      {
      vtkErrorMacro("No update extent has been set in the "
                    "information for output port " << outputPort
                    << " on algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ").");
      return 0;
      }
    // Make sure the update request is inside the whole extent.
    int wholeExtent[6];
    int updateExtent[6];
    outInfo->Get(WHOLE_EXTENT(), wholeExtent);
    outInfo->Get(UPDATE_EXTENT(), updateExtent);
    if((updateExtent[0] < wholeExtent[0] ||
        updateExtent[1] > wholeExtent[1] ||
        updateExtent[2] < wholeExtent[2] ||
        updateExtent[3] > wholeExtent[3] ||
        updateExtent[4] < wholeExtent[4] ||
        updateExtent[5] > wholeExtent[5]) &&
       (updateExtent[0] <= updateExtent[1] &&
        updateExtent[2] <= updateExtent[3] &&
        updateExtent[4] <= updateExtent[5]))
      {
      // Update extent is outside the whole extent and is not empty.
      vtkErrorMacro("The update extent specified in the "
                    "information for output port " << outputPort
                    << " on algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ") is "
                    << updateExtent[0] << " " << updateExtent[1] << " "
                    << updateExtent[2] << " " << updateExtent[3] << " "
                    << updateExtent[4] << " " << updateExtent[5]
                    << ", which is outside the whole extent "
                    << wholeExtent[0] << " " << wholeExtent[1] << " "
                    << wholeExtent[2] << " " << wholeExtent[3] << " "
                    << wholeExtent[4] << " " << wholeExtent[5] << ".");
      return 0;
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void
vtkStreamingDemandDrivenPipeline
::ExecuteDataStart(vtkInformation* request,
                   vtkInformationVector** inInfoVec,
                   vtkInformationVector* outInfoVec)
{
  // Preserve the execution continuation flag in the request across
  // iterations of the algorithm.  Perform start operations only if
  // not in an execute continuation.
  if(this->ContinueExecuting)
    {
    request->Set(CONTINUE_EXECUTING(), 1);
    }
  else
    {
    request->Remove(CONTINUE_EXECUTING());
    this->Superclass::ExecuteDataStart(request,inInfoVec,outInfoVec);
    }
}

//----------------------------------------------------------------------------
void
vtkStreamingDemandDrivenPipeline
::ExecuteDataEnd(vtkInformation* request,
                 vtkInformationVector** inInfoVec,
                 vtkInformationVector* outInfoVec)
{
  // Preserve the execution continuation flag in the request across
  // iterations of the algorithm.  Perform start operations only if
  // not in an execute continuation.
  if(request->Get(CONTINUE_EXECUTING()))
    {
    this->ContinueExecuting = 1;
    }
  else
    {
    this->ContinueExecuting = 0;
    this->Superclass::ExecuteDataEnd(request,inInfoVec,outInfoVec);
    }
}

//----------------------------------------------------------------------------
void
vtkStreamingDemandDrivenPipeline
::MarkOutputsGenerated(vtkInformation* request,
                       vtkInformationVector** inInfoVec,
                       vtkInformationVector* outInfoVec)
{
  // Tell outputs they have been generated.
  this->Superclass::MarkOutputsGenerated(request,inInfoVec,outInfoVec);

  // Compute ghost level arrays for generated outputs.
  for(int i=0; i < outInfoVec->GetNumberOfInformationObjects(); ++i)
    {
    vtkInformation* outInfo = outInfoVec->GetInformationObject(i);
    vtkDataObject* data = outInfo->Get(vtkDataObject::DATA_OBJECT());
    if(data && !outInfo->Get(DATA_NOT_GENERATED()))
      {
      if(vtkDataSet* ds = vtkDataSet::SafeDownCast(data))
        {
        ds->GenerateGhostLevelArray();
        }
      }
    }
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::NeedToExecuteData(int outputPort,
                    vtkInformationVector** inInfoVec,
                    vtkInformationVector* outInfoVec)
{
  // Has the algorithm asked to be executed again?
  if(this->ContinueExecuting)
    {
    return 1;
    }

  // If no port is specified, check all ports.  This behavior is
  // implemented by the superclass.
  if(outputPort < 0)
    {
    return this->Superclass::NeedToExecuteData(outputPort,
                                               inInfoVec,outInfoVec);
    }

  // Does the superclass want to execute?
  if(this->Superclass::NeedToExecuteData(outputPort,inInfoVec,outInfoVec))
    {
    return 1;
    }

  // We need to check the requested update extent.  Get the output
  // port information and data information.  We do not need to check
  // existence of values because it has already been verified by
  // VerifyOutputInformation.
  vtkInformation* outInfo = outInfoVec->GetInformationObject(outputPort);
  vtkDataObject* dataObject = outInfo->Get(vtkDataObject::DATA_OBJECT());
  vtkInformation* dataInfo = dataObject->GetInformation();
  if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_PIECES_EXTENT)
    {
    // Check the unstructured extent.  If we do not have the requested
    // piece, we need to execute.
    int updateNumberOfPieces = outInfo->Get(UPDATE_NUMBER_OF_PIECES());
    int dataNumberOfPieces = dataInfo->Get(vtkDataObject::DATA_NUMBER_OF_PIECES());
    if(dataNumberOfPieces != updateNumberOfPieces)
      {
      return 1;
      }
    int dataGhostLevel = dataInfo->Get(vtkDataObject::DATA_NUMBER_OF_GHOST_LEVELS());
    int updateGhostLevel = outInfo->Get(UPDATE_NUMBER_OF_GHOST_LEVELS());
    if(dataGhostLevel != updateGhostLevel)
      {
      return 1;
      }
    if (dataNumberOfPieces != 1)
      {
      int dataPiece = dataInfo->Get(vtkDataObject::DATA_PIECE_NUMBER());
      int updatePiece = outInfo->Get(UPDATE_PIECE_NUMBER());
      if (dataPiece != updatePiece)
        {
        return 1;
        }
      }
    }
  else if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_3D_EXTENT)
    {
    // Check the structured extent.  If the update extent is outside
    // of the extent and not empty, we need to execute.
    int dataExtent[6];
    int updateExtent[6];
    outInfo->Get(UPDATE_EXTENT(), updateExtent);
    dataInfo->Get(vtkDataObject::DATA_EXTENT(), dataExtent);
    // if the ue is out side the de
    if((updateExtent[0] < dataExtent[0] ||
        updateExtent[1] > dataExtent[1] ||
        updateExtent[2] < dataExtent[2] ||
        updateExtent[3] > dataExtent[3] ||
        updateExtent[4] < dataExtent[4] ||
        updateExtent[5] > dataExtent[5]) &&
       // and the ue is set
       (updateExtent[0] <= updateExtent[1] &&
        updateExtent[2] <= updateExtent[3] &&
        updateExtent[4] <= updateExtent[5]))
      {
      return 1;
      }
    }

  // if we are requesting a particular update time index, check
  // if we have the desired time index
  if ( outInfo->Has(UPDATE_TIME_INDEX()) )
    {
    if (!dataInfo->Has(vtkDataObject::DATA_TIME_INDEX()) ||
      dataInfo->Get(vtkDataObject::DATA_TIME_INDEX()) !=
      outInfo->Get(UPDATE_TIME_INDEX()))
      {
      return 1;
      }
    }

  // We do not need to execute.
  return 0;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetMaximumNumberOfPieces(vtkInformation *info, int n)
{
  if(!info)
    {
    vtkErrorMacro("SetMaximumNumberOfPieces on invalid output");
    return 0;
    }
  if(this->GetMaximumNumberOfPieces(info) != n)
    {
    info->Set(MAXIMUM_NUMBER_OF_PIECES(), n);
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::GetMaximumNumberOfPieces(vtkInformation *info)
{
  if(!info)
    {
    vtkErrorMacro("GetMaximumNumberOfPieces on invalid output");
    return 0;
    }
  if(!info->Has(MAXIMUM_NUMBER_OF_PIECES()))
    {
    info->Set(MAXIMUM_NUMBER_OF_PIECES(), -1);
    }
  return info->Get(MAXIMUM_NUMBER_OF_PIECES());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetWholeExtent(vtkInformation *info, int extent[6])
{
  if(!info)
    {
    vtkErrorMacro("SetWholeExtent on invalid output");
    return 0;
    }
  int modified = 0;
  int oldExtent[6];
  this->GetWholeExtent(info, oldExtent);
  if(oldExtent[0] != extent[0] || oldExtent[1] != extent[1] ||
     oldExtent[2] != extent[2] || oldExtent[3] != extent[3] ||
     oldExtent[4] != extent[4] || oldExtent[5] != extent[5])
    {
    modified = 1;
    info->Set(WHOLE_EXTENT(), extent, 6);
    }
  return modified;
}

//----------------------------------------------------------------------------
void vtkStreamingDemandDrivenPipeline
::GetWholeExtent(vtkInformation *info, int extent[6])
{
  static int emptyExtent[6] = {0,-1,0,-1,0,-1};
  if(!info)
    {
    memcpy(extent, emptyExtent, sizeof(int)*6);
    return;
    }
  if(!info->Has(WHOLE_EXTENT()))
    {
    info->Set(WHOLE_EXTENT(), emptyExtent, 6);
    }
  info->Get(WHOLE_EXTENT(), extent);
}

//----------------------------------------------------------------------------
int* vtkStreamingDemandDrivenPipeline::GetWholeExtent(vtkInformation* info)
{
  static int emptyExtent[6] = {0,-1,0,-1,0,-1};
  if(!info)
    {
    return emptyExtent;
    }
  if(!info->Has(WHOLE_EXTENT()))
    {
    info->Set(WHOLE_EXTENT(), emptyExtent, 6);
    }
  return info->Get(WHOLE_EXTENT());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetUpdateExtentToWholeExtent(vtkInformation *info)
{
  // Request all data.
  int modified = 0;
  if(vtkDataObject* data = info->Get(vtkDataObject::DATA_OBJECT()))
    {
    if(data->GetExtentType() == VTK_PIECES_EXTENT)
      {
      modified |= this->SetUpdatePiece(info, 0);
      modified |= this->SetUpdateNumberOfPieces(info, 1);
      modified |= this->SetUpdateGhostLevel(info, 0);
      }
    else if(data->GetExtentType() == VTK_3D_EXTENT)
      {
      int extent[6] = {0,-1,0,-1,0,-1};
      info->Get(WHOLE_EXTENT(), extent);
      modified |= this->SetUpdateExtent(info, extent);
      }
    }
  else
    {
    vtkErrorMacro("SetUpdateExtentToWholeExtent called with no data object.");
    }

  // Make sure the update extent will remain the whole extent until
  // the update extent is explicitly set by the caller.
  info->Set(UPDATE_EXTENT_INITIALIZED(), 0);

  return modified;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetUpdateExtent(vtkInformation *info, int extent[6])
{
  if(!info)
    {
    vtkErrorMacro("SetUpdateExtent on invalid output");
    return 0;
    }
  int modified = 0;
  int oldExtent[6];
  this->GetUpdateExtent(info, oldExtent);
  if(oldExtent[0] != extent[0] || oldExtent[1] != extent[1] ||
     oldExtent[2] != extent[2] || oldExtent[3] != extent[3] ||
     oldExtent[4] != extent[4] || oldExtent[5] != extent[5])
    {
    modified = 1;
    info->Set(UPDATE_EXTENT(), extent, 6);
    }
  info->Set(UPDATE_EXTENT_INITIALIZED(), 1);
  return modified;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetUpdateExtent(vtkInformation *info, int piece,
                  int numPieces,
                  int ghostLevel)
{
  if(!info)
    {
    vtkErrorMacro("SetUpdateExtent on invalid output");
    return 0;
    }
  int modified = 0;
  modified |= this->SetUpdatePiece(info, piece);
  modified |= this->SetUpdateNumberOfPieces(info, numPieces);
  modified |= this->SetUpdateGhostLevel(info, ghostLevel);
  if(vtkDataObject* data = info->Get(vtkDataObject::DATA_OBJECT()))
    {
    if(data->GetExtentType() == VTK_3D_EXTENT)
      {
      if(vtkExtentTranslator* translator = this->GetExtentTranslator(info))
        {
        int wholeExtent[6];
        this->GetWholeExtent(info, wholeExtent);
        translator->SetWholeExtent(wholeExtent);
        translator->SetPiece(piece);
        translator->SetNumberOfPieces(numPieces);
        translator->SetGhostLevel(ghostLevel);
        translator->PieceToExtent();
        modified |= this->SetUpdateExtent(info, translator->GetExtent());
        }
      else
        {
        vtkErrorMacro("Cannot translate unstructured extent to structured "
                      "for algorithm "
                      << this->Algorithm->GetClassName() << "("
                      << this->Algorithm << ").");
        }
      }
    }
  return modified;
}

//----------------------------------------------------------------------------
void vtkStreamingDemandDrivenPipeline
::GetUpdateExtent(vtkInformation *info, int extent[6])
{
  static int emptyExtent[6] = {0,-1,0,-1,0,-1};
  if(!info)
    {
    vtkErrorMacro("GetUpdateExtent on invalid output");
    memcpy(extent, emptyExtent, sizeof(int)*6);
    return;
    }
  if(!info->Has(UPDATE_EXTENT()))
    {
    info->Set(UPDATE_EXTENT(), emptyExtent, 6);
    info->Set(UPDATE_EXTENT_INITIALIZED(), 0);
    }
  info->Get(UPDATE_EXTENT(), extent);
}

//----------------------------------------------------------------------------
int* vtkStreamingDemandDrivenPipeline
::GetUpdateExtent(vtkInformation *info)
{
  static int emptyExtent[6] = {0,-1,0,-1,0,-1};
  if(!info)
    {
    vtkErrorMacro("GetUpdateExtent on invalid output");
    return emptyExtent;
    }
  if(!info->Has(UPDATE_EXTENT()))
    {
    info->Set(UPDATE_EXTENT(), emptyExtent, 6);
    info->Set(UPDATE_EXTENT_INITIALIZED(), 0);
    }
  return info->Get(UPDATE_EXTENT());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetUpdatePiece(vtkInformation *info, int piece)
{
  if(!info)
    {
    vtkErrorMacro("SetUpdatePiece on invalid output");
    return 0;
    }
  int modified = 0;
  if(this->GetUpdatePiece(info) != piece)
    {
    info->Set(UPDATE_PIECE_NUMBER(), piece);
    modified = 1;
    }
  info->Set(UPDATE_EXTENT_INITIALIZED(), 1);
  return modified;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::GetUpdatePiece(vtkInformation *info)
{
  if(!info)
    {
    vtkErrorMacro("GetUpdatePiece on invalid output");
    return 0;
    }
  if(!info->Has(UPDATE_PIECE_NUMBER()))
    {
    info->Set(UPDATE_PIECE_NUMBER(), 0);
    }
  return info->Get(UPDATE_PIECE_NUMBER());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetUpdateNumberOfPieces(vtkInformation *info, int n)
{
  if(!info)
    {
    vtkErrorMacro("SetUpdateNumberOfPieces on invalid output");
    return 0;
    }
  int modified = 0;
  if(this->GetUpdateNumberOfPieces(info) != n)
    {
    info->Set(UPDATE_NUMBER_OF_PIECES(), n);
    modified = 1;
    }
  info->Set(UPDATE_EXTENT_INITIALIZED(), 1);
  return modified;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::GetUpdateNumberOfPieces(vtkInformation *info)
{
  if(!info)
    {
    vtkErrorMacro("GetUpdateNumberOfPieces on invalid output");
    return 1;
    }
  if(!info->Has(UPDATE_NUMBER_OF_PIECES()))
    {
    info->Set(UPDATE_NUMBER_OF_PIECES(), 1);
    }
  return info->Get(UPDATE_NUMBER_OF_PIECES());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::SetUpdateGhostLevel(vtkInformation *info, int n)
{
  if(!info)
    {
    vtkErrorMacro("SetUpdateGhostLevel on invalid output");
    return 0;
    }
  if(this->GetUpdateGhostLevel(info) != n)
    {
    info->Set(UPDATE_NUMBER_OF_GHOST_LEVELS(), n);
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline
::GetUpdateGhostLevel(vtkInformation *info)
{
  if(!info)
    {
    vtkErrorMacro("GetUpdateGhostLevel on invalid output");
    return 0;
    }
  if(!info->Has(UPDATE_NUMBER_OF_GHOST_LEVELS()))
    {
    info->Set(UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
    }
  return info->Get(UPDATE_NUMBER_OF_GHOST_LEVELS());
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::SetRequestExactExtent(int port, int flag)
{
  if(!this->OutputPortIndexInRange(port, "set request exact extent flag on"))
    {
    return 0;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  if(this->GetRequestExactExtent(port) != flag)
    {
    info->Set(EXACT_EXTENT(), flag);
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::GetRequestExactExtent(int port)
{
  if(!this->OutputPortIndexInRange(port, "get request exact extent flag from"))
    {
    return 0;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  if(!info->Has(EXACT_EXTENT()))
    {
    info->Set(EXACT_EXTENT(), 0);
    }
  return info->Get(EXACT_EXTENT());
}

//----------------------------------------------------------------------------
int
vtkStreamingDemandDrivenPipeline
::SetExtentTranslator(vtkInformation *info, vtkExtentTranslator* translator)
{
  if(!info)
    {
    vtkErrorMacro("Attempt to set translator for invalid output");
    return 0;
    }
  vtkExtentTranslator* oldTranslator =
    vtkExtentTranslator::SafeDownCast(info->Get(EXTENT_TRANSLATOR()));
  if(translator != oldTranslator)
    {
    info->Set(EXTENT_TRANSLATOR(), translator);
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkExtentTranslator*
vtkStreamingDemandDrivenPipeline::GetExtentTranslator(vtkInformation *info)
{
  if(!info)
    {
    vtkErrorMacro("Attempt to get translator for invalid output");
    return 0;
    }
  vtkExtentTranslator* translator =
    vtkExtentTranslator::SafeDownCast(info->Get(EXTENT_TRANSLATOR()));
  if(!translator)
    {
    translator = vtkExtentTranslator::New();
    info->Set(EXTENT_TRANSLATOR(), translator);
    translator->Delete();
    }
  return translator;
}

//----------------------------------------------------------------------------
int vtkStreamingDemandDrivenPipeline::SetWholeBoundingBox(int port, 
                                                          double extent[6])
{
  if(!this->OutputPortIndexInRange(port, "set whole bounding box on"))
    {
    return 0;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  int modified = 0;
  double oldBoundingBox[6];
  this->GetWholeBoundingBox(port, oldBoundingBox);
  if(oldBoundingBox[0] != extent[0] || oldBoundingBox[1] != extent[1] ||
     oldBoundingBox[2] != extent[2] || oldBoundingBox[3] != extent[3] ||
     oldBoundingBox[4] != extent[4] || oldBoundingBox[5] != extent[5])
    {
    modified = 1;
    info->Set(WHOLE_BOUNDING_BOX(), extent, 6);
    }
  return modified;
}

//----------------------------------------------------------------------------
void vtkStreamingDemandDrivenPipeline::GetWholeBoundingBox(int port, double extent[6])
{
  static double emptyBoundingBox[6] = {0,-1,0,-1,0,-1};
  if(!this->OutputPortIndexInRange(port, "get whole bounding box from"))
    {
    memcpy(extent, emptyBoundingBox, sizeof(double)*6);
    return;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  if(!info->Has(WHOLE_BOUNDING_BOX()))
    {
    info->Set(WHOLE_BOUNDING_BOX(), emptyBoundingBox, 6);
    }
  info->Get(WHOLE_BOUNDING_BOX(), extent);
}

//----------------------------------------------------------------------------
double* vtkStreamingDemandDrivenPipeline::GetWholeBoundingBox(int port)
{
  static double emptyBoundingBox[6] = {0,-1,0,-1,0,-1};
  if(!this->OutputPortIndexInRange(port, "get whole bounding box from"))
    {
    return emptyBoundingBox;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  if(!info->Has(WHOLE_BOUNDING_BOX()))
    {
    info->Set(WHOLE_BOUNDING_BOX(), emptyBoundingBox, 6);
    }
  return info->Get(WHOLE_BOUNDING_BOX());
}
