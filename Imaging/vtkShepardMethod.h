/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkShepardMethod.h
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
// .NAME vtkShepardMethod - sample unstructured points onto structured points using the method of Shepard
// .SECTION Description
// vtkShepardMethod is a filter used to visualize unstructured point data using
// Shepard's method. The method works by resampling the unstructured points 
// onto a structured points set. The influence functions are described as 
// "inverse distance weighted". Once the structured points are computed, the 
// usual visualization techniques (e.g., iso-contouring or volume rendering)
// can be used visualize the structured points.
// .SECTION Caveats
// The input to this filter is any dataset type. This filter can be used 
// to resample any form of data, i.e., the input data need not be 
// unstructured. 
//
// The bounds of the data (i.e., the sample space) is automatically computed
// if not set by the user.
//
// If you use a maximum distance less than 1.0, some output points may
// never receive a contribution. The final value of these points can be 
// specified with the "NullValue" instance variable.

#ifndef __vtkShepardMethod_h
#define __vtkShepardMethod_h

#include "vtkDataSetToStructuredPointsFilter.h"

class VTK_IMAGING_EXPORT vtkShepardMethod : public vtkDataSetToStructuredPointsFilter 
{
public:
  vtkTypeRevisionMacro(vtkShepardMethod,vtkDataSetToStructuredPointsFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with sample dimensions=(50,50,50) and so that model bounds are
  // automatically computed from input. Null value for each unvisited output 
  // point is 0.0. Maximum distance is 0.25.
  static vtkShepardMethod *New();
  
  // Description:
  // Compute ModelBounds from input geometry.
  float ComputeModelBounds(float origin[3], float ar[3]);

  // Description:
  // Specify i-j-k dimensions on which to sample input points.
  vtkGetVectorMacro(SampleDimensions,int,3);
  
  // Description:
  // Set the i-j-k dimensions on which to sample the distance function.
  void SetSampleDimensions(int i, int j, int k);

  // Description:
  // Set the i-j-k dimensions on which to sample the distance function.
  void SetSampleDimensions(int dim[3]);

  // Description:
  // Specify influence distance of each input point. This distance is a 
  // fraction of the length of the diagonal of the sample space. Thus, values 
  // of 1.0 will cause each input point to influence all points in the 
  // structured point dataset. Values less than 1.0 can improve performance
  // significantly.
  vtkSetClampMacro(MaximumDistance,float,0.0,1.0);
  vtkGetMacro(MaximumDistance,float);

  // Description:
  // Specify the position in space to perform the sampling.
  vtkSetVector6Macro(ModelBounds,float);
  vtkGetVectorMacro(ModelBounds,float,6);

  // Description:
  // Set the Null value for output points not receiving a contribution from the
  // input points.
  vtkSetMacro(NullValue,float);
  vtkGetMacro(NullValue,float);

protected:
  vtkShepardMethod();
  ~vtkShepardMethod() {};

  void Execute();

  int SampleDimensions[3];
  float MaximumDistance;
  float ModelBounds[6];
  float NullValue;
private:
  vtkShepardMethod(const vtkShepardMethod&);  // Not implemented.
  void operator=(const vtkShepardMethod&);  // Not implemented.
};

#endif


