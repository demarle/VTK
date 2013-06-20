/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageDotProduct.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageDotProduct - Dot product of two vector images.
// .SECTION Description
// vtkImageDotProduct interprets the scalar components of two images
// as vectors and takes the dot product vector by vector (pixel by pixel).

#ifndef __vtkSMPImageDotProduct_h
#define __vtkSMPImageDotProduct_h



#include "vtkThreadedImageAlgorithm.h"

class VTK_SMP_EXPORT vtkSMPImageDotProduct : public vtkImageAlgorithm
{
public:
  static vtkSMPImageDotProduct *New();
  vtkTypeMacro(vtkSMPImageDotProduct,vtkThreadedImageAlgorithm);

  // Description:
  // Set the two inputs to this filter
  virtual void SetInput1(vtkDataObject *in) { this->SetInput(0,in); }
  virtual void SetInput2(vtkDataObject *in) { this->SetInput(1,in); }

protected:
  vtkSMPImageDotProduct();
  ~vtkSMPImageDotProduct() {};

  virtual int RequestInformation (vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *);

  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

private:
  vtkSMPImageDotProduct(const vtkSMPImageDotProduct&);  // Not implemented.
  void operator=(const vtkSMPImageDotProduct&);  // Not implemented.
};

#endif



