/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPPipeline.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPPipeline - Enables multi-core processing of blocks
// .SECTION Description
// vtkSMPPipeline is a vtkExectutive that operates on different blocks
// simultaneously on different cores.

#ifndef VTKSMPPIPELINE_H
#define VTKSMPPIPELINE_H

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkCompositeDataPipeline.h"

class VTKPARALLELSMP_EXPORT vtkSMPPipeline : public vtkCompositeDataPipeline
{
protected:
  vtkSMPPipeline();
  ~vtkSMPPipeline();

  // Description:
  // Overridden to...
  virtual int ExecuteData(vtkInformation* request,
                          vtkInformationVector** inInfoVec,
                          vtkInformationVector* outInfoVec);

  // Description:
  // Overridden to...
  virtual void ExecuteSimpleAlgorithm(vtkInformation* request,
                                      vtkInformationVector** inInfoVec,
                                      vtkInformationVector* outInfoVec,
                                      int compositePort);

  // Description:
  // Check whether the data object in the pipeline information for an
  // output port exists and has a valid type.
  virtual int CheckDataObject(int port, vtkInformationVector* outInfo);

public:
  friend class ParallelFilterExecutor;

  vtkTypeMacro(vtkSMPPipeline, vtkCompositeDataPipeline);
  static vtkSMPPipeline* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Key defining the concrete type of output data to make sure
  // the vtkSMPAlgorithm will produce the right type of temp data.
  static vtkInformationStringKey* DATA_OBJECT_CONCRETE_TYPE();

private:
  vtkSMPPipeline(const vtkSMPPipeline&); // Not implemented
  void operator =(const vtkSMPPipeline&); // Not implemented

};

#endif // VTKSMPPIPELINE_H
