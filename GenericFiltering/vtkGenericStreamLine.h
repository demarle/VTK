/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGenericStreamLine.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGenericStreamLine - generate streamline in arbitrary dataset
// .SECTION Description
// vtkGenericStreamLine is a filter that generates a streamline for an arbitrary 
// dataset. A streamline is a line that is everywhere tangent to the vector
// field. Scalar values also are calculated along the streamline and can be 
// used to color the line. Streamlines are calculated by integrating from
// a starting point through the vector field. Integration can be performed
// forward in time (see where the line goes), backward in time (see where the
// line came from), or in both directions. It also is possible to compute
// vorticity along the streamline. Vorticity is the projection (i.e., dot
// product) of the flow rotation on the velocity vector, i.e., the rotation
// of flow around the streamline.
//
// vtkGenericStreamLine defines the instance variable StepLength. This parameter 
// controls the time increment used to generate individual points along
// the streamline(s). Smaller values result in more line 
// primitives but smoother streamlines. The StepLength instance variable is 
// defined in terms of time (i.e., the distance that the particle travels in
// the specified time period). Thus, the line segments will be smaller in areas
// of low velocity and larger in regions of high velocity. (NOTE: This is
// different than the IntegrationStepLength defined by the superclass
// vtkStreamer. IntegrationStepLength is used to control integration step 
// size and is expressed as a fraction of the cell length.) The StepLength
// instance variable is important because subclasses of vtkGenericStreamLine (e.g.,
// vtkDashedStreamLine) depend on this value to build their representation.

// .SECTION See Also
// vtkStreamer vtkDashedStreamLine vtkStreamPoints

#ifndef __vtkGenericStreamLine_h
#define __vtkGenericStreamLine_h

#include "vtkGenericStreamer.h"

class VTK_GENERIC_FILTERING_EXPORT vtkGenericStreamLine : public vtkGenericStreamer
{
public:
  vtkTypeRevisionMacro(vtkGenericStreamLine,vtkGenericStreamer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with step size set to 1.0.
  static vtkGenericStreamLine *New();

  // Description:
  // Specify the length of a line segment. The length is expressed in terms of
  // elapsed time. Smaller values result in smoother appearing streamlines, but
  // greater numbers of line primitives.
  vtkSetClampMacro(StepLength,double,0.000001,VTK_DOUBLE_MAX);
  vtkGetMacro(StepLength,double);

protected:
  vtkGenericStreamLine();
  ~vtkGenericStreamLine() {};

  // Convert streamer array into vtkPolyData
  void Execute();

  // the length of line primitives
  double StepLength;

private:
  vtkGenericStreamLine(const vtkGenericStreamLine&);  // Not implemented.
  void operator=(const vtkGenericStreamLine&);  // Not implemented.
};

#endif


