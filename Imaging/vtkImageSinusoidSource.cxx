/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageSinusoidSource.cxx
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
#include "vtkImageSinusoidSource.h"
#include "vtkObjectFactory.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageSinusoidSource, "1.32");
vtkStandardNewMacro(vtkImageSinusoidSource);

//----------------------------------------------------------------------------
vtkImageSinusoidSource::vtkImageSinusoidSource()
{
  this->Direction[0] = 1.0;
  this->Direction[1] = 0.0;
  this->Direction[2] = 0.0;
  
  this->Amplitude = 255.0;
  this->Phase = 0.0;
  this->Period = 20.0;

  this->WholeExtent[0] = 0;  this->WholeExtent[1] = 255;
  this->WholeExtent[2] = 0;  this->WholeExtent[3] = 255;
  this->WholeExtent[4] = 0;  this->WholeExtent[5] = 0;
  
}

void vtkImageSinusoidSource::SetDirection(float v[3])
{
  this->SetDirection(v[0],v[1],v[2]);
}

void vtkImageSinusoidSource::SetDirection(float v0, float v1, float v2)
{
  float sum;

  sum = v0*v0 + v1*v1 + v2*v2;

  if (sum == 0.0)
    {
    vtkErrorMacro("Zero direction vector");
    return;
    }
  
  // normalize
  sum = 1.0 / sqrt(sum);
  v0 *= sum;
  v1 *= sum;
  v2 *= sum;
  
  if (this->Direction[0] == v0 && this->Direction[1] == v1 
      && this->Direction[2] == v2)
    {
    return;
    }
  
  this->Direction[0] = v0;
  this->Direction[1] = v1;
  this->Direction[2] = v2;
  
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkImageSinusoidSource::SetWholeExtent(int xMin, int xMax, 
                                            int yMin, int yMax,
                                            int zMin, int zMax)
{
  int modified = 0;
  
  if (this->WholeExtent[0] != xMin)
    {
    modified = 1;
    this->WholeExtent[0] = xMin ;
    }
  if (this->WholeExtent[1] != xMax)
    {
    modified = 1;
    this->WholeExtent[1] = xMax ;
    }
  if (this->WholeExtent[2] != yMin)
    {
    modified = 1;
    this->WholeExtent[2] = yMin ;
    }
  if (this->WholeExtent[3] != yMax)
    {
    modified = 1;
    this->WholeExtent[3] = yMax ;
    }
  if (this->WholeExtent[4] != zMin)
    {
    modified = 1;
    this->WholeExtent[4] = zMin ;
    }
  if (this->WholeExtent[5] != zMax)
    {
    modified = 1;
    this->WholeExtent[5] = zMax ;
    }
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImageSinusoidSource::ExecuteInformation()
{
  vtkImageData *output = this->GetOutput();

  output->SetWholeExtent(this->WholeExtent);
  output->SetScalarType(VTK_FLOAT);
  output->SetNumberOfScalarComponents(1);
}

void vtkImageSinusoidSource::ExecuteData(vtkDataObject *output)
{
  vtkImageData *data = this->AllocateOutputData(output);
  float *outPtr;
  int idxX, idxY, idxZ;
  int maxX, maxY, maxZ;
  int outIncX, outIncY, outIncZ;
  int *outExt;
  float sum;
  float yContrib, zContrib, xContrib;
  unsigned long count = 0;
  unsigned long target;
  
  if (data->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro("Execute: This source only outputs floats");
    }
  
  outExt = data->GetExtent();
  
  // find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  
  // Get increments to march through data 
  data->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  outPtr = (float *) data->GetScalarPointer(outExt[0],outExt[2],outExt[4]);

  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;

  // Loop through ouput pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    zContrib = this->Direction[2] * (idxZ + outExt[4]);
    for (idxY = 0; !this->AbortExecute && idxY <= maxY; idxY++)
      {
      if (!(count%target))
        {
        this->UpdateProgress(count/(50.0*target));
        }
      count++;
      yContrib = this->Direction[1] * (idxY + outExt[2]);
      for (idxX = 0; idxX <= maxX; idxX++)
        {
        xContrib = this->Direction[0] * (float)(idxX + outExt[0]);
        // find dot product
        sum = zContrib + yContrib + xContrib;
        
        *outPtr = this->Amplitude * 
          cos((6.2831853 * sum / this->Period) - this->Phase);
        outPtr++;
        }
      outPtr += outIncY;
      }
    outPtr += outIncZ;
    }
}

void vtkImageSinusoidSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Period: " << this->Period << "\n";
  os << indent << "Phase: " << this->Phase << "\n";
  os << indent << "Amplitude: " << this->Amplitude << "\n";
  os << indent << "Direction: ( "
     << this->Direction[0] << ", "
     << this->Direction[1] << ", "
     << this->Direction[2] << " )\n";

}

