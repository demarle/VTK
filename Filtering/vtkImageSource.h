/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageSource.h
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
// .NAME vtkImageSource - Source of data for the imaging pipeline
// .SECTION Description
// vtkImageSource is the superclass for all imaging sources and filters.
// The method Update(), called by the cache, is the major interface
// to the source.

// .SECTION See Also
// vtkImageToImageFilter


#ifndef __vtkImageSource_h
#define __vtkImageSource_h

#include "vtkSource.h"

class vtkImageData;

class VTK_FILTERING_EXPORT vtkImageSource : public vtkSource
{
public:
  vtkTypeRevisionMacro(vtkImageSource,vtkSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output of this source.
  void SetOutput(vtkImageData *output);
  vtkImageData *GetOutput();
  vtkImageData *GetOutput(int idx);
  
protected:
  vtkImageSource();
  ~vtkImageSource() {};

  void Execute();
  virtual void Execute(vtkImageData *data);

  // a helper method that sets the extent and allocates the output 
  // passed into it and returns it as an image data
  virtual vtkImageData *AllocateOutputData(vtkDataObject *out);

  void ComputeRequiredInputUpdateExtent( int *vtkNotUsed(in), 
                                         int *vtkNotUsed(out) ) 
    {VTK_LEGACY_METHOD(ComputeRequiredInputUpdateExtent,"3.2");}
  
private:
  vtkImageSource(const vtkImageSource&);  // Not implemented.
  void operator=(const vtkImageSource&);  // Not implemented.
};


#endif


