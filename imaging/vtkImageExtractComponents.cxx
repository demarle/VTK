/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageExtractComponents.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
#include <math.h>

#include "vtkImageExtractComponents.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageExtractComponents* vtkImageExtractComponents::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageExtractComponents");
  if(ret)
    {
    return (vtkImageExtractComponents*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageExtractComponents;
}





//----------------------------------------------------------------------------
vtkImageExtractComponents::vtkImageExtractComponents()
{
  this->Components[0] = 0;
  this->Components[1] = 1;
  this->Components[2] = 2;
  this->NumberOfComponents = 1;
}

//----------------------------------------------------------------------------
void vtkImageExtractComponents::SetComponents(int c1, int c2, int c3)
{
  int modified = 0;
  
  if (this->Components[0] != c1)
    {
    this->Components[0] = c1;
    modified = 1;
    }
  if (this->Components[1] != c2)
    {
    this->Components[1] = c2;
    modified = 1;
    }
  if (this->Components[2] != c3)
    {
    this->Components[2] = c3;
    modified = 1;
    }
  
  if (modified || this->NumberOfComponents != 3)
    {
    this->NumberOfComponents = 3;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImageExtractComponents::SetComponents(int c1, int c2)
{
  int modified = 0;
  
  if (this->Components[0] != c1)
    {
    this->Components[0] = c1;
    modified = 1;
    }
  if (this->Components[1] != c2)
    {
    this->Components[1] = c2;
    modified = 1;
    }
  
  if (modified || this->NumberOfComponents != 2)
    {
    this->NumberOfComponents = 2;
    this->Modified();
    }
}
							
//----------------------------------------------------------------------------
void vtkImageExtractComponents::SetComponents(int c1)
{
  int modified = 0;
  
  if (this->Components[0] != c1)
    {
    this->Components[0] = c1;
    modified = 1;
    }
  
  if (modified || this->NumberOfComponents != 1)
    {
    this->NumberOfComponents = 1;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
// This method tells the superclass that only one component will remain.
void vtkImageExtractComponents::ExecuteInformation(
                   vtkImageData *vtkNotUsed(inData), vtkImageData *outData)
{
  outData->SetNumberOfScalarComponents(this->NumberOfComponents);
}

//----------------------------------------------------------------------------
template <class T>
static void vtkImageExtractComponentsExecute(vtkImageExtractComponents *self,
					     vtkImageData *inData, T *inPtr,
					     vtkImageData *outData, T *outPtr,
					     int outExt[6], int id)
{
  int idxR, idxY, idxZ;
  int maxX, maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int cnt, inCnt;
  int offset1, offset2, offset3;
  unsigned long count = 0;
  unsigned long target;
  
  // find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;
  
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  cnt = outData->GetNumberOfScalarComponents();
  inCnt = inData->GetNumberOfScalarComponents();
  
  // Loop through output pixels
  offset1 = self->GetComponents()[0];
  offset2 = self->GetComponents()[1];
  offset3 = self->GetComponents()[2];
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
      {
      if (!id) 
	{
	if (!(count%target))
	  {
	  self->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      // handle inner loop based on number of components extracted
      switch (cnt)
	{
	case 1:
	  for (idxR = 0; idxR <= maxX; idxR++)
	    {
	    // Pixel operation
	    *outPtr = *(inPtr + offset1);
	    outPtr++;
	    inPtr += inCnt;
	    }
	  break;
	case 2:
	  for (idxR = 0; idxR <= maxX; idxR++)
	    {
	    // Pixel operation
	    *outPtr = *(inPtr + offset1);
	    outPtr++;
	    *outPtr = *(inPtr + offset2);
	    outPtr++;
	    inPtr += inCnt;
	    }
	  break;
	case 3:
	  for (idxR = 0; idxR <= maxX; idxR++)
	    {
	    // Pixel operation
	    *outPtr = *(inPtr + offset1);
	    outPtr++;
	    *outPtr = *(inPtr + offset2);
	    outPtr++;
	    *outPtr = *(inPtr + offset3);
	    outPtr++;
	    inPtr += inCnt;
	    }
	  break;
	}
      outPtr += outIncY;
      inPtr += inIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    }
}


//----------------------------------------------------------------------------
// This method is passed input and output datas, and executes the
// ExtractComponents function on each line.  
void vtkImageExtractComponents::ThreadedExecute(vtkImageData *inData, 
						vtkImageData *outData,
						int outExt[6], int id)
{
  int max, idx;
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
		<< ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
    << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  // make sure we can get all of the components.
  max = inData->GetNumberOfScalarComponents();
  for (idx = 0; idx < this->NumberOfComponents; ++idx)
    {
    if (this->Components[idx] > max)
      {
      vtkErrorMacro("Execute: Component " << this->Components[idx]
		    << " is not in input.");
      return;
      }
    }
  
  // choose which templated function to call.
  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageExtractComponentsExecute(this, inData, (double *)(inPtr),
				       outData, (double *)(outPtr),
				       outExt, id);
      break;
    case VTK_FLOAT:
      vtkImageExtractComponentsExecute(this, inData, (float *)(inPtr),
				       outData, (float *)(outPtr),
				       outExt, id);
      break;
    case VTK_LONG:
      vtkImageExtractComponentsExecute(this, inData, (long *)(inPtr),
				       outData, (long *)(outPtr),
				       outExt, id);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageExtractComponentsExecute(this,inData,(unsigned long *)(inPtr),
				       outData, (unsigned long *)(outPtr),
				       outExt, id);
      break;
    case VTK_INT:
      vtkImageExtractComponentsExecute(this, inData, (int *)(inPtr),
				       outData, (int *)(outPtr),
				       outExt, id);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageExtractComponentsExecute(this,inData,(unsigned int *)(inPtr),
				       outData, (unsigned int *)(outPtr),
				       outExt, id);
      break;
    case VTK_SHORT:
      vtkImageExtractComponentsExecute(this, inData, (short *)(inPtr),
				       outData, (short *)(outPtr),
				       outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageExtractComponentsExecute(this,inData,(unsigned short *)(inPtr),
				       outData, (unsigned short *)(outPtr),
				       outExt, id);
      break;
    case VTK_CHAR:
      vtkImageExtractComponentsExecute(this, inData, (char *)(inPtr),
				       outData, (char *)(outPtr),
				       outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageExtractComponentsExecute(this, inData, (unsigned char *)(inPtr),
				       outData, (unsigned char *)(outPtr),
				       outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}

void vtkImageExtractComponents::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "NumberOfComponents: " << this->NumberOfComponents << vtkEndl;
  os << indent << "Components: ( "
     << this->Components[0] << ", "
     << this->Components[1] << ", "
     << this->Components[2] << " )\n";

}

