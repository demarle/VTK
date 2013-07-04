/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPContourFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPContourFilter - !!!!
// .SECTION Description
// !!!!

#ifndef __vtkSMPContourFilter_h
#define __vtkSMPContourFilter_h

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkContourFilter.h"

class VTKPARALLELSMP_EXPORT vtkSMPContourFilter : public vtkContourFilter
{
public:
  vtkTypeMacro(vtkSMPContourFilter,vtkContourFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkSMPContourFilter *New();

protected:
  vtkSMPContourFilter();
  ~vtkSMPContourFilter();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkSMPContourFilter(const vtkSMPContourFilter&);  // Not implemented.
  void operator=(const vtkSMPContourFilter&);  // Not implemented.
};

#endif
