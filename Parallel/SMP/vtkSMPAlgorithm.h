/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPAlgorithm.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPAlgorithm - Base class to derive for creating multi-threaded
// algorithms.
// .SECTION Description
// !!!!

#ifndef __vtkSMPAlgorithm_h
#define __vtkSMPAlgorithm_h

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkAlgorithm.h"

class vtkDataObject;
template<class T> class vtkThreadLocal;

class VTKPARALLELSMP_EXPORT vtkSMPAlgorithm : public vtkAlgorithm
{
public:
  vtkTypeMacro(vtkSMPAlgorithm, vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Overridden to do the work in many threads.
  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inInfo,
                             vtkInformationVector* outInfo);

protected:
  vtkSMPAlgorithm();
  ~vtkSMPAlgorithm();

  // Description:
  // Overridden to ...
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector,
                          vtkThreadLocal<vtkDataObject>** outputData);

  // Description:
  // Create a default executive.
  // Overridden to ...
  virtual vtkExecutive* CreateDefaultExecutive();

  // Description:
  // Overridden to ...
  virtual void SetExecutive(vtkExecutive* executive);

private:
  vtkSMPAlgorithm(const vtkSMPAlgorithm&);  // Not implemented.
  void operator=(const vtkSMPAlgorithm&);  // Not implemented.
};

#endif







