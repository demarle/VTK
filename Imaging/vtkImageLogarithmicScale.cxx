/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageLogarithmicScale.cxx
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
#include "vtkImageLogarithmicScale.h"
#include "vtkObjectFactory.h"
#include "vtkImageProgressIterator.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageLogarithmicScale, "1.20");
vtkStandardNewMacro(vtkImageLogarithmicScale);

//----------------------------------------------------------------------------
// Constructor sets default values
vtkImageLogarithmicScale::vtkImageLogarithmicScale()
{
  this->Constant = 10.0;
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageLogarithmicScaleExecute(vtkImageLogarithmicScale *self,
                                            vtkImageData *inData,
                                            vtkImageData *outData, 
                                            int outExt[6], int id, T *)
{
  vtkImageIterator<T> inIt(inData, outExt);
  vtkImageProgressIterator<T> outIt(outData, outExt, self, id);
  float c;

  c = self->GetConstant();

  // Loop through ouput pixels
  while (!outIt.IsAtEnd())
    {
    T* inSI = inIt.BeginSpan();
    T* outSI = outIt.BeginSpan();
    T* outSIEnd = outIt.EndSpan();
    while (outSI != outSIEnd)
      {
      // Pixel operation
      if (*inSI > 0)
        {
        *outSI = (T)(c*log((double)(*inSI)+1.0));
        }
      else
        {
        *outSI = (T)(-c*log(1.0-(double)(*inSI)));
        }
      
      outSI++;
      inSI++;
      }
    inIt.NextSpan();
    outIt.NextSpan();
    }
}


//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageLogarithmicScale::ThreadedExecute(vtkImageData *inData, 
                                               vtkImageData *outData,
                                               int outExt[6], int id)
{
  vtkDebugMacro(<< "Execute: inData = " << inData 
  << ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
    << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  switch (inData->GetScalarType())
    {
    vtkTemplateMacro6(vtkImageLogarithmicScaleExecute, this, inData, 
                      outData, outExt, id,  static_cast<VTK_TT *>(0));
    default:
      vtkErrorMacro(<< "Execute: Unknown input ScalarType");
      return;
    }
}

void vtkImageLogarithmicScale::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Constant: " << this->Constant << "\n";
}

