/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageExtractComponents.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageExtractComponents - Outputs a single component
// .SECTION Description
// vtkImageExtractComponents takes an input with any number of components
// and outputs some of them.  It does involve a copy of the data.

// .SECTION See Also
// vtkImageAppendComponents

#ifndef __vtkImageExtractComponents_h
#define __vtkImageExtractComponents_h


#include "vtkImageToImageFilter.h"

class VTK_IMAGING_EXPORT vtkImageExtractComponents : public vtkImageToImageFilter
{
public:
  static vtkImageExtractComponents *New();
  vtkTypeRevisionMacro(vtkImageExtractComponents,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the components to extract.
  void SetComponents(int c1);
  void SetComponents(int c1, int c2);
  void SetComponents(int c1, int c2, int c3);
  vtkGetVector3Macro(Components,int);
  
  // Description:
  // Get the number of components to extract. This is set implicitly by the 
  // SetComponents() method.
  vtkGetMacro(NumberOfComponents,int);

protected:
  vtkImageExtractComponents();
  ~vtkImageExtractComponents() {};

  int NumberOfComponents;
  int Components[3];

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
                       int ext[6], int id);
private:
  vtkImageExtractComponents(const vtkImageExtractComponents&);  // Not implemented.
  void operator=(const vtkImageExtractComponents&);  // Not implemented.
};

#endif










