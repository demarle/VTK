/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPProbeFilter.cxx
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
#include "vtkPProbeFilter.h"

#include "vtkIdTypeArray.h"
#include "vtkMultiProcessController.h"
#include "vtkSocketController.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"

vtkCxxRevisionMacro(vtkPProbeFilter, "1.5");
vtkStandardNewMacro(vtkPProbeFilter);

vtkCxxSetObjectMacro(vtkPProbeFilter, Controller, vtkMultiProcessController);
vtkCxxSetObjectMacro(vtkPProbeFilter, SocketController, vtkSocketController);

//----------------------------------------------------------------------------
vtkPProbeFilter::vtkPProbeFilter()
{
  this->Controller = 0;
  this->SocketController = 0;
}

//----------------------------------------------------------------------------
vtkPProbeFilter::~vtkPProbeFilter()
{
  this->SetController(0);
  this->SetSocketController(0);
}

//----------------------------------------------------------------------------
void vtkPProbeFilter::ExecuteInformation()
{
  vtkDataSet *output = this->GetOutput();
  output->SetMaximumNumberOfPieces(-1);
}

//----------------------------------------------------------------------------
void vtkPProbeFilter::ExecuteData(vtkDataObject *)
{
  vtkDataSet *output = this->GetOutput();
  //vtkDataObject *input = this->GetInput();
  //vtkDataObject *source = this->GetSource();
  
  this->vtkProbeFilter::Execute();
  int procid = 0;
  int numProcs = 1;
  if ( this->Controller )
    {
    procid = this->Controller->GetLocalProcessId();
    numProcs = this->Controller->GetNumberOfProcesses();
    }

  vtkIdType numPoints = this->GetValidPoints()->GetMaxId() + 1;
  if ( procid )
    {
    // Satellite node
    this->Controller->Send(&numPoints, 1, 0, 1970);
    if ( numPoints > 0 )
      {
      this->Controller->Send(this->GetValidPoints(), 0, 1971);
      this->Controller->Send(output, 0, 1972);      
      }
    output->ReleaseData();
    }
  else if ( numProcs > 1 )
    {
    vtkIdType numRemotePoints = 0;
    vtkIdTypeArray *validPoints = vtkIdTypeArray::New();
    vtkDataSet *remoteProbeOutput = output->NewInstance();
    vtkPointData *remotePointData;
    vtkPointData *pointData = output->GetPointData();
    vtkIdType i;
    vtkIdType j;
    vtkIdType k;
    vtkIdType pointId;
    vtkIdType numComponents = pointData->GetNumberOfComponents();
    float *tuple = new float[numComponents];
    for (i = 1; i < numProcs; i++)
      {
      this->Controller->Receive(&numRemotePoints, 1, i, 1970);
      if (numRemotePoints > 0)
        {
        this->Controller->Receive(validPoints, i, 1971);
        this->Controller->Receive(remoteProbeOutput, i, 1972);
      
        remotePointData = remoteProbeOutput->GetPointData();
        for (j = 0; j < numRemotePoints; j++)
          {
          pointId = validPoints->GetValue(j);
        
          remotePointData->GetTuple(pointId, tuple);
        
          for (k = 0; k < numComponents; k++)
            {
            output->GetPointData()->SetComponent(pointId, k, tuple[k]);
            }
          }
        }
      }
    validPoints->Delete();
    remoteProbeOutput->Delete();
    delete [] tuple;
    }

  // Handle client server.
  // Client only had Socket controller.
  // Server node zero has Socket controller and default controller.
  if (this->SocketController)
    {
    if (this->Controller && procid == 0)
      {
      this->SocketController->Send(output, 1, 1973);
      }
    else if (this->Controller == NULL)
      {   // Client
      vtkDataSet *remoteProbeOutput = output->NewInstance();
      this->SocketController->Receive(remoteProbeOutput, 1, 1973);
      output->CopyStructure(remoteProbeOutput);
      output->GetPointData()->PassData(remoteProbeOutput->GetPointData());
      output->GetCellData()->PassData(remoteProbeOutput->GetCellData());
      output->GetFieldData()->PassData(remoteProbeOutput->GetFieldData());
      remoteProbeOutput->Delete();
      }
    }
}

//----------------------------------------------------------------------------
void vtkPProbeFilter::ComputeInputUpdateExtents( vtkDataObject *output )
{
  vtkDataObject *input = this->GetInput();
  vtkDataObject *source = this->GetSource();

  if ( !input || !source || !output )
    {
    return;
    }

  input->SetUpdateExtent(0, 1, 0);
  source->SetUpdateExtent(output->GetUpdatePiece(),
                          output->GetUpdateNumberOfPieces(),
                          output->GetUpdateGhostLevel());
}

//----------------------------------------------------------------------------
void vtkPProbeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Controller " << this->Controller << endl;
  os << indent << "SocketController " << this->SocketController << endl;
}
