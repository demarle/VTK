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
// .NAME vtkSMPAlgorithm - !!!!
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

  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inInfo,
                             vtkInformationVector* outInfo);

protected:
  vtkSMPAlgorithm();
  ~vtkSMPAlgorithm();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  // Use the inputVector to populate the outputData, the
  // superclass will be responsible for merging each local
  // output into the outputVector.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector,
                          vtkThreadLocal<vtkDataObject>** outputData);

  // Description:
  // Create a default executive.
  // If the DefaultExecutivePrototype is set and is a subclass
  // of vtkSMPPipeline, a copy of it is created
  // in CreateDefaultExecutive() using NewInstance().
  // Otherwise, vtkSMPPipeline is created.
  virtual vtkExecutive* CreateDefaultExecutive();
  // Description:
  // Set this algorithm's executive.  This algorithm is removed from
  // any executive to which it has previously been assigned and then
  // assigned to the given executive.
  // If the given executive is not a subclass of vtkSMPPipeline,
  // SetExecutive() is not doing anything.
  virtual void SetExecutive(vtkExecutive* executive);

private:
  vtkSMPAlgorithm(const vtkSMPAlgorithm&);  // Not implemented.
  void operator=(const vtkSMPAlgorithm&);  // Not implemented.
};

#endif







