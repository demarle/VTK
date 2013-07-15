/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPClipDataSet.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPContourFilter - multithreaded vtkContourFilter
// .SECTION Description
// Just like parent, but uses the SMP framework to do work on many threads.

#ifndef __vtkSMPClipDataSet_h
#define __vtkSMPClipDataSet_h

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkClipDataSet.h"

class VTKPARALLELSMP_EXPORT vtkSMPClipDataSet : public vtkClipDataSet
{
public:
  vtkTypeMacro(vtkSMPClipDataSet,vtkClipDataSet);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkSMPClipDataSet *New();

protected:
  vtkSMPClipDataSet();
  ~vtkSMPClipDataSet();

  // Description:
  // Overridden to do the work in many threads.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkSMPClipDataSet(const vtkSMPClipDataSet&);  // Not implemented.
  void operator=(const vtkSMPClipDataSet&);  // Not implemented.
};

#endif
