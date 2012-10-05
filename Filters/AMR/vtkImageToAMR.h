/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageToAMR - filter to convert any vtkImageData to a
// vtkOverlappingAMR.
// .SECTION Description
// vtkImageToAMR is a simple filter that converts any vtkImageData to a
// vtkOverlappingAMR dataset. The input vtkImageData is treated as the highest
// refinement available for the highest level. The lower refinements and the
// number of blocks is controlled properties specified on the filter.

#ifndef __vtkImageToAMR_h
#define __vtkImageToAMR_h

#include "vtkOverlappingAMRAlgorithm.h"
#include "vtkFiltersAMRModule.h" // For export macro

class VTKFILTERSAMR_EXPORT vtkImageToAMR : public vtkOverlappingAMRAlgorithm
{
public:
  static vtkImageToAMR* New();
  vtkTypeMacro(vtkImageToAMR, vtkOverlappingAMRAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the maximum number of levels in the generated Overlapping-AMR.
  vtkSetClampMacro(NumberOfLevels, int, 1, VTK_INT_MAX);
  vtkGetMacro(NumberOfLevels, int);

  // Description:
  // Set the refinement ratio for levels. This refinement ratio is used for all
  // levels.
  vtkSetClampMacro(RefinementRatio, int, 2, VTK_INT_MAX);
  vtkGetMacro(RefinementRatio, int);

  // Description:
  // Set the maximun number of blocks in the output
  vtkSetClampMacro(MaximumNumberOfBlocks, int, 1, VTK_INT_MAX);
  vtkGetMacro(MaximumNumberOfBlocks, int);


//BTX
protected:
  vtkImageToAMR();
  ~vtkImageToAMR();

  // Description:
  // Fill the input port information objects for this algorithm.  This
  // is invoked by the first call to GetInputPortInformation for each
  // port so subclasses can specify what they can handle.
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  int NumberOfLevels;
  int MaximumNumberOfBlocks;
  int RefinementRatio;


private:
  vtkImageToAMR(const vtkImageToAMR&); // Not implemented.
  void operator=(const vtkImageToAMR&); // Not implemented.
//ETX
};

#endif
