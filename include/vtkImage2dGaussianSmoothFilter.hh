/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImage2dGaussianSmoothFilter.hh
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
// .NAME vtkImage2dGaussianSmoothFilter - smooths on a 2D plane.
// .SECTION Description
// vtkImage2dGaussianSmoothFilter implements a 2d Gaussian smoothing on an axis
// aligned plane.  It really consists of two 1d Gaussian filters.


#ifndef __vtkImage2dGaussianSmoothFilter_h
#define __vtkImage2dGaussianSmoothFilter_h


#include "vtkImageFilter.hh"
#include "vtkImage1dGaussianSmoothFilter.hh"

class vtkImage2dGaussianSmoothFilter : public vtkImageFilter
{
public:
  vtkImage2dGaussianSmoothFilter();
  ~vtkImage2dGaussianSmoothFilter();
  char *GetClassName() {return "vtkImage2dGaussianSmoothFilter";};

  // Forward Object messages to filter1 and fitler2
  void DebugOn();
  void Modified();
  // Foward Source messages to fitler2
  void GenerateRegion(int *outOffset, int *outSize);
  void SetCache(vtkImageCache *cache);
  vtkImageCache *GetCache();
  vtkImageSource *GetOutput();
  void GetBoundary(int *offset, int *size);
  unsigned long GetPipelineMTime();
  // Foward filter messages to fitler1
  void SetInput(vtkImageSource *Input);

  void SetAxes(int axis1, int axis2);
  void SetGauss(float Std, int Radius);

protected:

  vtkImage1dGaussianSmoothFilter *Filter1;
  vtkImage1dGaussianSmoothFilter *Filter2;

  int Axis1;
  int Axis2;
};

#endif



