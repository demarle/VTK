/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImage2dGradientDerivativeFilter.h
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
// .NAME vtkImage2dGradientDerivativeFilter - Derivative of gradient images.
// .SECTION Description
// vtkImage2dGradientDerivativeFilter Operates only on gradient images
// because it needs the phase in the second component.  It Takes 
// a derivative along the gradient. Output has only one component.



#ifndef __vtkImage2dGradientDerivativeFilter_h
#define __vtkImage2dGradientDerivativeFilter_h


#include "vtkImageSpatialFilter.h"

class vtkImage2dGradientDerivativeFilter : public vtkImageSpatialFilter
{
public:
  vtkImage2dGradientDerivativeFilter();
  char *GetClassName() {return "vtkImage2dGradientDerivativeFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetAxes2d(int axis0, int axis1);
  void InterceptCacheUpdate(vtkImageRegion *region);

  // Description:
  // Magnitude has to be above this threshold to register.
  vtkSetMacro(LowerThreshold, float);
  vtkGetMacro(LowerThreshold, float);
  
  
protected:
  float LowerThreshold;
  
  void ExecuteCenter3d(vtkImageRegion *inRegion, vtkImageRegion *outRegion);
  void ExecuteBoundary3d(vtkImageRegion *inRegion, vtkImageRegion *outRegion);

};

#endif



