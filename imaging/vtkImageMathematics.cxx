/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageMathematics.cxx
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
#include "vtkImageRegion.h"
#include "vtkImageMathematics.h"
#include <math.h>



//----------------------------------------------------------------------------
vtkImageMathematics::vtkImageMathematics()
{
  this->SetExecutionAxes(VTK_IMAGE_X_AXIS, VTK_IMAGE_Y_AXIS);
  this->Operation = VTK_ADD;
}



//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
// Handles the one input operations
static void vtkImageMathematicsExecute1(vtkImageMathematics *self,
				vtkImageRegion *in1Region, float *in1Ptr,
				vtkImageRegion *outRegion, float *outPtr)
{
  int op;
  int min0, max0, min1, max1;
  int idx0, idx1;
  int in1Inc0, in1Inc1;
  int outInc0, outInc1;
  float *in1Ptr0, *in1Ptr1;
  float *outPtr0, *outPtr1;
  
  op = self->GetOperation();
  
  // Get information to march through data 
  in1Region->GetIncrements(in1Inc0, in1Inc1);
  outRegion->GetIncrements(outInc0, outInc1);
  outRegion->GetExtent(min0, max0, min1, max1);

  // Loop through ouput pixels
  in1Ptr1 = in1Ptr;
  outPtr1 = outPtr;
  for (idx1 = min1; idx1 <= max1; ++idx1)
    {
    outPtr0 = outPtr1;
    in1Ptr0 = in1Ptr1;
    for (idx0 = min0; idx0 <= max0; ++idx0)
      {
      
      // Pixel operaton
      switch (op)
	{
	case VTK_INVERT:
	  *outPtr0 = 1.0 / *in1Ptr0;
	  break;
	case VTK_SIN:
	  *outPtr0 = sin(*in1Ptr0);
	  break;
	case VTK_COS:
	  *outPtr0 = cos(*in1Ptr0);
	  break;
	case VTK_EXP:
	  *outPtr0 = exp(*in1Ptr0);
	  break;
	case VTK_LOG:
	  *outPtr0 = log(*in1Ptr0);
	  break;
	case VTK_ABS:
	  *outPtr0 = fabs(*in1Ptr0);
	  break;
	case VTK_SQR:
	  *outPtr0 = *in1Ptr0 * *in1Ptr0;
	  break;
	case VTK_SQRT:
	  *outPtr0 = sqrt(*in1Ptr0);
	  break;
	}
      
      outPtr0 += outInc0;
      in1Ptr0 += in1Inc0;
      }
    outPtr1 += outInc1;
    in1Ptr1 += in1Inc1;
    }
}



//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
// Handles the two input operations
template <class T>
static void vtkImageMathematicsExecute2(vtkImageMathematics *self,
				       vtkImageRegion *in1Region, T *in1Ptr,
				       vtkImageRegion *in2Region, T *in2Ptr,
				       vtkImageRegion *outRegion, T *outPtr)
{
  int op;
  int min0, max0, min1, max1;
  int idx0, idx1;
  int in1Inc0, in1Inc1;
  int in2Inc0, in2Inc1;
  int outInc0, outInc1;
  T *in1Ptr0, *in1Ptr1;
  T *in2Ptr0, *in2Ptr1;
  T *outPtr0, *outPtr1;
  
  op = self->GetOperation();
  
  // Get information to march through data 
  in1Region->GetIncrements(in1Inc0, in1Inc1);
  in2Region->GetIncrements(in2Inc0, in2Inc1);
  outRegion->GetIncrements(outInc0, outInc1);
  outRegion->GetExtent(min0, max0, min1, max1);

  // Loop through ouput pixels
  in1Ptr1 = in1Ptr;
  in2Ptr1 = in2Ptr;
  outPtr1 = outPtr;
  for (idx1 = min1; idx1 <= max1; ++idx1)
    {
    outPtr0 = outPtr1;
    in1Ptr0 = in1Ptr1;
    in2Ptr0 = in2Ptr1;
    for (idx0 = min0; idx0 <= max0; ++idx0)
      {
      
      // Pixel operaton
      switch (op)
	{
	case VTK_ADD:
	  *outPtr0 = *in1Ptr0 + *in2Ptr0;
	  break;
	case VTK_SUBTRACT:
	  *outPtr0 = *in1Ptr0 - *in2Ptr0;
	  break;
	case VTK_MULTIPLY:
	  *outPtr0 = *in1Ptr0 * *in2Ptr0;
	  break;
	case VTK_DIVIDE:
	  if (*in2Ptr0)
	    {
	    *outPtr0 = *in1Ptr0 / *in2Ptr0;
	    }
	  else
	    {
	    *outPtr0 = (T)(*in1Ptr0 / 0.00001);
	    }
	  break;
	case VTK_MIN:
	  if (*in1Ptr0 < *in2Ptr0)
	    {
	    *outPtr0 = *in1Ptr0;
	    }
	  else
	    {
	    *outPtr0 = *in2Ptr0;
	    }
	  break;
	case VTK_MAX:
	  if (*in1Ptr0 > *in2Ptr0)
	    {
	    *outPtr0 = *in1Ptr0;
	    }
	  else
	    {
	    *outPtr0 = *in2Ptr0;
	    }
	}
      
      
      outPtr0 += outInc0;
      in1Ptr0 += in1Inc0;
      in2Ptr0 += in2Inc0;
      }
    outPtr1 += outInc1;
    in1Ptr1 += in1Inc1;
    in2Ptr1 += in2Inc1;
    }
}



//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output regions, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageMathematics::Execute(vtkImageRegion *inRegion1, 
				  vtkImageRegion *inRegion2, 
				  vtkImageRegion *outRegion)
{
  void *inPtr1 = inRegion1->GetScalarPointer();
  void *outPtr = outRegion->GetScalarPointer();

  if (this->Operation == VTK_ADD || this->Operation == VTK_SUBTRACT || 
      this->Operation == VTK_MULTIPLY || this->Operation == VTK_DIVIDE ||
      this->Operation == VTK_MIN || this->Operation == VTK_MAX) 
    {
    void *inPtr2 = inRegion2->GetScalarPointer();
    // this filter expects that inputs are the same type as output.
    if (inRegion1->GetScalarType() != outRegion->GetScalarType() ||
	inRegion2->GetScalarType() != outRegion->GetScalarType())
      {
      vtkErrorMacro(<< "Execute: input ScalarTypes, " 
        << inRegion1->GetScalarType() << " and " << inRegion2->GetScalarType()
        << ", must match out ScalarType " << outRegion->GetScalarType());
      return;
    }
    
    switch (inRegion1->GetScalarType())
      {
      case VTK_FLOAT:
	vtkImageMathematicsExecute2(this, 
				   inRegion1, (float *)(inPtr1), 
				   inRegion2, (float *)(inPtr2), 
				   outRegion, (float *)(outPtr));
	break;
      case VTK_INT:
	vtkImageMathematicsExecute2(this, 
				   inRegion1, (int *)(inPtr1), 
				   inRegion2, (int *)(inPtr2), 
				   outRegion, (int *)(outPtr));
	break;
      case VTK_SHORT:
	vtkImageMathematicsExecute2(this, 
				   inRegion1, (short *)(inPtr1), 
				   inRegion2, (short *)(inPtr2), 
				   outRegion, (short *)(outPtr));
	break;
      case VTK_UNSIGNED_SHORT:
	vtkImageMathematicsExecute2(this, 
				   inRegion1, (unsigned short *)(inPtr1), 
				   inRegion2, (unsigned short *)(inPtr2), 
				   outRegion, (unsigned short *)(outPtr));
	break;
      case VTK_UNSIGNED_CHAR:
	vtkImageMathematicsExecute2(this, 
				   inRegion1, (unsigned char *)(inPtr1), 
				   inRegion2, (unsigned char *)(inPtr2), 
				   outRegion, (unsigned char *)(outPtr));
	break;
      default:
	vtkErrorMacro(<< "Execute: Unknown ScalarType");
	return;
      }
    }
  else
    {
    // this filter expects that inputs are the same type as output.
    if (inRegion1->GetScalarType() != VTK_FLOAT ||
	outRegion->GetScalarType() != VTK_FLOAT)
      {
      vtkErrorMacro(<< "Execute: input ScalarType, "
        << vtkImageScalarTypeNameMacro(inRegion1->GetScalarType()) 
        << ", and out ScalarType "
        << vtkImageScalarTypeNameMacro(outRegion->GetScalarType())
        << " must be float for choosen operation");
      return;
      }
    
    vtkImageMathematicsExecute1(this, inRegion1, (float *)(inPtr1), 
				outRegion, (float *)(outPtr));
    }
}
















