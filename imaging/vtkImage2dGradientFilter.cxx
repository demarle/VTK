/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImage2dGradientFilter.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

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
#include <math.h>
#include "vtkImage2dGradientFilter.h"


//----------------------------------------------------------------------------
// Description:
// Construct an instance of vtkImage2dGradientFilter fitler.
vtkImage2dGradientFilter::vtkImage2dGradientFilter()
{
  this->KernelSize[0] = 3;
  this->KernelSize[1] = 3;

  this->KernelMiddle[0] = 1;
  this->KernelMiddle[1] = 1;
  
  this->SetAxes3d(VTK_IMAGE_X_AXIS,VTK_IMAGE_Y_AXIS, VTK_IMAGE_COMPONENT_AXIS);
  this->SetOutputDataType(VTK_IMAGE_FLOAT);
  
  this->UseExecuteCenterOff();
}


//----------------------------------------------------------------------------
void vtkImage2dGradientFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkImageSpatialFilter::PrintSelf(os, indent);
}



//----------------------------------------------------------------------------
// Description:
// Add Component as the third axis.
void vtkImage2dGradientFilter::SetAxes2d(int axis0, int axis1)
{
  if (axis0 == VTK_IMAGE_COMPONENT_AXIS || axis1 == VTK_IMAGE_COMPONENT_AXIS)
    {
    vtkErrorMacro(<< "SetAxes2d: Cannot use Component as an axis");
    return;
    }
  this->SetAxes3d(axis0, axis0, VTK_IMAGE_COMPONENT_AXIS);
}


//----------------------------------------------------------------------------
// Description:
// All components will be generated.
void vtkImage2dGradientFilter::InterceptCacheUpdate(vtkImageRegion *region)
{
  int bounds[6];
  
  region->GetBounds3d(bounds);
  bounds[4] = 0;
  bounds[5] = 2;
  region->SetBounds3d(bounds);
}



//----------------------------------------------------------------------------
// Description:
// This method is passed a region that holds the image bounds of this filters
// input, and changes the region to hold the image bounds of this filters
// output.
void vtkImage2dGradientFilter::ComputeOutputImageInformation(
		    vtkImageRegion *inRegion, vtkImageRegion *outRegion)
{
  int bounds[8];
  int idx;

  inRegion->GetImageBounds4d(bounds);
  if ( ! this->HandleBoundaries)
    {
    // shrink output image bounds.
    for (idx = 0; idx < 4; ++idx)
      {
      bounds[idx*2] += this->KernelMiddle[idx];
      bounds[idx*2+1] -= (this->KernelSize[idx] - 1) - this->KernelMiddle[idx];
      }
    }
  
  // from 0 to 2 components
  bounds[4] = 0;
  bounds[5] = 2;

  outRegion->SetImageBounds4d(bounds);
}



//----------------------------------------------------------------------------
// Description:
// This execute method handles boundaries.
// it handles boundaries. Pixels are just replicated to get values 
// out of bounds.
template <class T>
void vtkImage2dGradientFilterExecute(vtkImage2dGradientFilter *self,
				     vtkImageRegion *inRegion, T *inPtr, 
				     vtkImageRegion *outRegion, float *outPtr)
{
  float d0, d1;
  float r0, r1;
  float temp;
  // For looping though output (and input) pixels.
  int outMin0, outMax0, outMin1, outMax1;
  int outIdx0, outIdx1;
  int outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  int inInc0, inInc1;
  T *inPtr0, *inPtr1;
  // Boundary of input image
  int inImageMin0, inImageMax0, inImageMin1, inImageMax1;
  
  self = self;
  // Get boundary information
  inRegion->GetImageBounds2d(inImageMin0,inImageMax0, inImageMin1,inImageMax1);
  
  // Get information to march through data
  inRegion->GetIncrements2d(inInc0, inInc1); 
  outRegion->GetIncrements3d(outInc0, outInc1, outInc2); 
  outRegion->GetBounds2d(outMin0, outMax0, outMin1, outMax1);
  
  // We want the input pixel to correspond to output
  inPtr = (T *)(inRegion->GetVoidPointer2d(outMin0, outMin1));

  // The aspect ratio is important for computing the gradient.
  inRegion->GetAspectRatio2d(r0, r1);
  r0 = 1.0 / r0;
  r1 = 1.0 / r1;
  
  // loop through pixels of output
  outPtr1 = outPtr;
  inPtr1 = inPtr;
  for (outIdx1 = outMin1; outIdx1 <= outMax1; ++outIdx1)
    {
    outPtr0 = outPtr1;
    inPtr0 = inPtr1;
    for (outIdx0 = outMin0; outIdx0 <= outMax0; ++outIdx0)
      {
	
      // Compute gradient using central differences (if in bounds).
      d0 = ((outIdx0 + 1) > inImageMax0) ? *inPtr0 : inPtr0[inInc0];
      d1 = ((outIdx1 + 1) > inImageMax1) ? *inPtr0 : inPtr0[inInc1];
      d0 -= ((outIdx0 - 1) < inImageMin0) ? *inPtr0 : inPtr0[-inInc0];
      d1 -= ((outIdx1 - 1) < inImageMin1) ? *inPtr0 : inPtr0[-inInc1];
      d0 *= r0;
      d1 *= r1;
      
      // Set the magnitude
      outPtr2 = outPtr0;
      *outPtr2 = (float)(hypot(d0, d1));
      temp = 1.0 / *outPtr2;
      // Set the vector
      outPtr2 += outInc2;
      *outPtr2 = d0 * temp;
      outPtr2 += outInc2;
      *outPtr2 = d1 * temp;
      
      outPtr0 += outInc0;
      inPtr0 += inInc0;
      }
    outPtr1 += outInc1;
    inPtr1 += inInc1;
    }
}

//----------------------------------------------------------------------------
// Description:
// This method contains a switch statement that calls the correct
// templated function for the input region type.  The output region
// must be of type float.  This method does handle boundary conditions.
// The third axis is the component axis for the output.
void vtkImage2dGradientFilter::Execute3d(vtkImageRegion *inRegion, 
					 vtkImageRegion *outRegion)
{
  void *inPtr = inRegion->GetVoidPointer3d();
  void *outPtr = outRegion->GetVoidPointer3d();
  
  // this filter expects that output is type float.
  if (outRegion->GetDataType() != VTK_IMAGE_FLOAT)
    {
    vtkErrorMacro(<< "Execute3d: output DataType, "
                  << vtkImageDataTypeNameMacro(outRegion->GetDataType())
                  << ", must be float");
    return;
    }
  
  switch (inRegion->GetDataType())
    {
    case VTK_IMAGE_FLOAT:
      vtkImage2dGradientFilterExecute(this, 
			  inRegion, (float *)(inPtr), 
			  outRegion, (float *)(outPtr));
      break;
    case VTK_IMAGE_INT:
      vtkImage2dGradientFilterExecute(this, 
			  inRegion, (int *)(inPtr), 
			  outRegion, (float *)(outPtr));
      break;
    case VTK_IMAGE_SHORT:
      vtkImage2dGradientFilterExecute(this, 
			  inRegion, (short *)(inPtr), 
			  outRegion, (float *)(outPtr));
      break;
    case VTK_IMAGE_UNSIGNED_SHORT:
      vtkImage2dGradientFilterExecute(this, 
			  inRegion, (unsigned short *)(inPtr), 
			  outRegion, (float *)(outPtr));
      break;
    case VTK_IMAGE_UNSIGNED_CHAR:
      vtkImage2dGradientFilterExecute(this, 
			  inRegion, (unsigned char *)(inPtr), 
			  outRegion, (float *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute3d: Unknown DataType");
      return;
    }
}






