/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageAccumulate.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/

#include "vtkImageAccumulate.h"
#include <math.h>
#include <stdlib.h>
#include "vtkObjectFactory.h"



//----------------------------------------------------------------------------
vtkImageAccumulate* vtkImageAccumulate::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageAccumulate");
  if(ret)
    {
    return (vtkImageAccumulate*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageAccumulate;
}





//----------------------------------------------------------------------------
// Constructor sets default values
vtkImageAccumulate::vtkImageAccumulate()
{
  int idx;
  
  for (idx = 0; idx < 3; ++idx)
    {
    this->ComponentSpacing[idx] = 1.0;
    this->ComponentOrigin[idx] = 0.0;
    this->ComponentExtent[idx*2] = 0;
    this->ComponentExtent[idx*2+1] = 0;
    }
  this->ComponentExtent[1] = 255;
}

//----------------------------------------------------------------------------
void vtkImageAccumulate::SetComponentExtent(int extent[6])
{
  int idx, modified = 0;
  
  for (idx = 0; idx < 6; ++idx)
    {
    if (this->ComponentExtent[idx] != extent[idx])
      {
      this->ComponentExtent[idx] = extent[idx];
      this->Modified();
      }
    }
  if (modified)
    {
    this->Modified();
    }
}


//----------------------------------------------------------------------------
void vtkImageAccumulate::SetComponentExtent(int minX, int maxX, 
					    int minY, int maxY,
					    int minZ, int maxZ)
{
  int extent[6];
  
  extent[0] = minX;  extent[1] = maxX;
  extent[2] = minY;  extent[3] = maxY;
  extent[4] = minZ;  extent[5] = maxZ;
  this->SetComponentExtent(extent);
}


//----------------------------------------------------------------------------
void vtkImageAccumulate::GetComponentExtent(int extent[6])
{
  int idx;
  
  for (idx = 0; idx < 6; ++idx)
    {
    extent[idx] = this->ComponentExtent[idx];
    }
}


//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageAccumulateExecute(vtkImageAccumulate *self,
			       vtkImageData *inData, T *inPtr,
			       vtkImageData *outData, int *outPtr)
{
  int min0, max0, min1, max1, min2, max2;
  int idx0, idx1, idx2, idxC;
  int inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2, *inPtrC;
  int *outPtrC;
  int numC, outIdx, *outExtent, *outIncs;
  float *origin, *spacing;
  unsigned long count = 0;
  unsigned long target;

  self = self;

  // Zero count in every bin
  outData->GetExtent(min0, max0, min1, max1, min2, max2);
  memset((void *)outPtr, 0, 
	 (max0-min0+1)*(max1-min1+1)*(max2-min2+1)*sizeof(int));
    
  // Get information to march through data 
  numC = inData->GetNumberOfScalarComponents();
  inData->GetExtent(min0, max0, min1, max1, min2, max2);
  inData->GetIncrements(inInc0, inInc1, inInc2);
  outExtent = outData->GetExtent();
  outIncs = outData->GetIncrements();
  origin = outData->GetOrigin();
  spacing = outData->GetSpacing();
  
  target = (unsigned long)((max2 - min2 + 1)*(max1 - min1 +1)/50.0);
  target++;

  inPtr2 = inPtr;
  for (idx2 = min2; idx2 <= max2; ++idx2)
    {
    inPtr1 = inPtr2;
    for (idx1 = min1; !self->AbortExecute && idx1 <= max1; ++idx1)
      {
      if (!(count%target))
	{
	self->UpdateProgress(count/(50.0*target));
	}
      count++;
      inPtr0  = inPtr1;
      for (idx0 = min0; idx0 <= max0; ++idx0)
	{
	inPtrC = inPtr0;
	// find the bin for this pixel.
	outPtrC = outPtr;
	for (idxC = 0; idxC < numC; ++idxC)
	  {
	  // compute the index
	  outIdx = (int)(((float)*inPtrC - origin[idxC]) / spacing[idxC]);
	  if (outIdx < outExtent[idxC*2] || outIdx > outExtent[idxC*2+1])
	    {
	    // Out of bin range
	    outPtrC = NULL;
	    break;
	    }
	  outPtrC += (outIdx - outExtent[idxC*2]) * outIncs[idxC];
	  ++inPtrC;
	  }
	if (outPtrC)
	  {
	  ++(*outPtrC);
	  }
	
	inPtr0 += inInc0;
	}
      inPtr1 += inInc1;
      }
    inPtr2 += inInc2;
    }
}

	

//----------------------------------------------------------------------------
// This method is passed a input and output Data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the Datas data types.
void vtkImageAccumulate::Execute(vtkImageData *inData, 
				 vtkImageData *outData)
{
  void *inPtr;
  void *outPtr;
  
  inPtr = inData->GetScalarPointer();
  outPtr = outData->GetScalarPointer();
  
  // Components turned into x, y and z
  if (this->GetInput()->GetNumberOfScalarComponents() > 3)
    {
    vtkErrorMacro("This filter can handle upto 3 components");
    return;
    }
  
  // this filter expects that output is type int.
  if (outData->GetScalarType() != VTK_INT)
    {
    vtkErrorMacro(<< "Execute: out ScalarType " << outData->GetScalarType()
		  << " must be int\n");
    return;
    }
  
  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageAccumulateExecute(this, 
			  inData, (double *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_FLOAT:
      vtkImageAccumulateExecute(this, 
			  inData, (float *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_LONG:
      vtkImageAccumulateExecute(this, 
			  inData, (long *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageAccumulateExecute(this, 
			  inData, (unsigned long *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_INT:
      vtkImageAccumulateExecute(this, 
			  inData, (int *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_UNSIGNED_INT:
      vtkImageAccumulateExecute(this, 
			  inData, (unsigned int *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_SHORT:
      vtkImageAccumulateExecute(this, 
			  inData, (short *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageAccumulateExecute(this, 
			  inData, (unsigned short *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_CHAR:
      vtkImageAccumulateExecute(this, 
			  inData, (char *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageAccumulateExecute(this, 
			  inData, (unsigned char *)(inPtr), 
			  outData, (int *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}


//----------------------------------------------------------------------------
void vtkImageAccumulate::ExecuteInformation(vtkImageData *vtkNotUsed(input), 
					    vtkImageData *output)
{
  output->SetWholeExtent(this->ComponentExtent);
  output->SetOrigin(this->ComponentOrigin);
  output->SetSpacing(this->ComponentSpacing);
  output->SetNumberOfScalarComponents(1);
  output->SetScalarType(VTK_INT);
}

//----------------------------------------------------------------------------
// Get ALL of the input.
void vtkImageAccumulate::ComputeRequiredInputUpdateExtent(int inExt[6], 
							  int outExt[6])
{
  int *wholeExtent;

  outExt = outExt;
  wholeExtent = this->GetInput()->GetWholeExtent();
  memcpy(inExt, wholeExtent, 6*sizeof(int));
}

//----------------------------------------------------------------------------
void vtkImageAccumulate:: ModifyOutputUpdateExtent()
{
  int wholeExtent[8];
  
  // Filter superclass has no control of intercept cache update.
  // a work around
  if (this->Bypass)
    {
    return;
    }
  
  this->GetOutput()->GetWholeExtent(wholeExtent);
  this->GetOutput()->SetUpdateExtent(wholeExtent);
}

void vtkImageAccumulate::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "ComponentOrigin: ( "
     << this->ComponentOrigin[0] << ", "
     << this->ComponentOrigin[1] << ", "
     << this->ComponentOrigin[2] << " )\n";

  os << indent << "ComponentSpacing: ( "
     << this->ComponentSpacing[0] << ", "
     << this->ComponentSpacing[1] << ", "
     << this->ComponentSpacing[2] << " )\n";
}

