/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProbeFilter.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkProbeFilter.h"
#include "vtkObjectFactory.h"



//----------------------------------------------------------------------------
vtkProbeFilter* vtkProbeFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkProbeFilter");
  if(ret)
    {
    return (vtkProbeFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkProbeFilter;
}




//----------------------------------------------------------------------------
vtkProbeFilter::vtkProbeFilter()
{
  this->SpatialMatch = 0;
}

//----------------------------------------------------------------------------
vtkProbeFilter::~vtkProbeFilter()
{
}


//----------------------------------------------------------------------------
void vtkProbeFilter::SetSource(vtkDataSet *input)
{
  this->vtkProcessObject::SetNthInput(1, input);
}

//----------------------------------------------------------------------------
vtkDataSet *vtkProbeFilter::GetSource()
{
  if (this->NumberOfInputs < 2)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Inputs[1]);
}


//----------------------------------------------------------------------------
void vtkProbeFilter::Execute()
{
  int ptId;
  float *x, tol2;
  vtkCell *cell;
  vtkPointData *pd, *outPD;
  int numPts, subId;
  vtkDataSet *source = this->GetSource();
  vtkDataSet *input = this->GetInput();
  vtkDataSet *output= this->GetOutput();
  float pcoords[3], *weights;
  float fastweights[256];

  vtkDebugMacro(<<"Probing data");

  if (source == NULL)
    {
    vtkErrorMacro (<< "Source is NULL.");
    return;
    }

  pd = source->GetPointData();
  if (pd == NULL)
    {
    vtkErrorMacro(<< "PointData is NULL.");
    return;
    }

  // lets use a stack allocated array if possible for performance reasons
  int mcs = source->GetMaxCellSize();
  if (mcs<=256)
    {
    weights = fastweights;
    }
  else
    {
    weights = new float[mcs];
    }

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  numPts = input->GetNumberOfPoints();
  //
  // Allocate storage for output PointData
  //
  outPD = output->GetPointData();
  outPD->InterpolateAllocate(pd);
  //
  // Use tolerance as a function of size of source data
  //
  tol2 = source->GetLength();
  tol2 = tol2*tol2 / 1000.0;
  //
  // Loop over all input points, interpolating source data
  //
  for (ptId=0; ptId < numPts; ptId++)
    {
    // Get the xyz coordinate of the point in the input dataset
    x = input->GetPoint(ptId);

    // Find the cell that contains xyz and get it
    cell = source->FindAndGetCell(x,NULL,-1,tol2,subId,pcoords,weights);
    if (cell)
      {
      // Interpolate the point data
      outPD->InterpolatePoint(pd,ptId,cell->PointIds,weights);
      }
    else
      {
      outPD->NullPoint(ptId);
      }
    }
  if (mcs>256)
    {
    delete [] weights;
    }
}

//----------------------------------------------------------------------------
void vtkProbeFilter::ExecuteInformation()
{
  if (this->GetInput() == NULL || this->GetSource() == NULL)
    {
    vtkErrorMacro("Missing input or source");
    return;
    }
  this->GetOutput()->SetMaximumNumberOfPieces(
                        this->GetInput()->GetMaximumNumberOfPieces());
}


//----------------------------------------------------------------------------
void vtkProbeFilter::ComputeInputUpdateExtents( vtkDataObject *output )
{
  vtkDataObject *input = this->GetInput();
  vtkDataObject *source = this->GetSource();

  // What ever happend to CopyUpdateExtent in vtkDataObject?
  // Copying both piece and extent could be bad.  Setting the piece
  // of a structured data set will affect the extent.

  if ( ! this->SpatialMatch)
    {
    source->SetUpdateExtent(0, 1, 0);
    }
  else
    {
    source->SetUpdateExtent(output->GetUpdatePiece(), 
                            output->GetUpdateNumberOfPieces(),
                            output->GetUpdateGhostLevel());
    source->SetUpdateExtent(output->GetUpdateExtent()); 
    }
  
  input->SetUpdateExtent(output->GetUpdatePiece(), 
                         output->GetUpdateNumberOfPieces(),
                         output->GetUpdateGhostLevel());
  input->SetUpdateExtent(output->GetUpdateExtent()); 

}

//----------------------------------------------------------------------------
void vtkProbeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSet *source = this->GetSource();

  vtkDataSetToDataSetFilter::PrintSelf(os,indent);
  os << indent << "Source: " << source << "\n";
  if (this->SpatialMatch)
    {
    os << indent << "SpatialMatchOn\n";
    }
  else
    {
    os << indent << "SpatialMatchOff\n";
    }
}
