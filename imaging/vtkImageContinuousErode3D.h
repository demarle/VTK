/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageContinuousErode3D.h
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
// .NAME vtkImageContinuousErode3D - Erosion implemented as a minimum.
// .SECTION Description
// vtkImageContinuousErode3D replaces a pixel with the minimum over
// an elipsiodal neighborhood.  If KernelSize of an axis is 1, no processing
// is done on that axis.


#ifndef __vtkImageContinuousErode3D_h
#define __vtkImageContinuousErode3D_h


#include "vtkImageSpatialFilter.h"

class vtkImageEllipsoidSource;

class VTK_EXPORT vtkImageContinuousErode3D : public vtkImageSpatialFilter
{
public:

  // Description:
  // Construct an instance of vtkImageContinuousErode3D fitler.
  // By default zero values are eroded.
  vtkImageContinuousErode3D();

  ~vtkImageContinuousErode3D();
  static vtkImageContinuousErode3D *New() 
    {return new vtkImageContinuousErode3D;};
  const char *GetClassName() {return "vtkImageContinuousErode3D";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // This method sets the size of the neighborhood.  It also sets the 
  // default middle of the neighborhood and computes the eliptical foot print.
  void SetKernelSize(int size0, int size1, int size2);

  
protected:
  vtkImageEllipsoidSource *Ellipse;
    
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int extent[6], int id);
};

#endif



