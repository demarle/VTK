/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGLUTesselatorTriangleFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGLUTesselatorTriangleFilter - create triangle polygons from input polygons and triangle strips
// .SECTION Description
// vtkGLUTesselatorTriangleFilter generates triangles from input polygons and
// triangle strips. The filter also will pass through vertices and lines, if
// requested. 
//
// This filter is a specialized version of vtkTriangleFilter. It uses
// glu code to perform the tessellation. To use this filter, you need to
// have a GLU library.
//
// .SECTION See Also
// vtkTriangleFilter

#ifndef __vtkGLUTesselatorTriangleFilter_h
#define __vtkGLUTesselatorTriangleFilter_h

#include "vtkPolyDataToPolyDataFilter.h"
#include <GL/glu.h> // Needed for GLUtesselator

class VTK_HYBRID_EXPORT vtkGLUTesselatorTriangleFilter : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkGLUTesselatorTriangleFilter *New();
  vtkTypeRevisionMacro(vtkGLUTesselatorTriangleFilter,vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Turn on/off passing vertices through filter.
  vtkBooleanMacro(PassVerts,int);
  vtkSetMacro(PassVerts,int);
  vtkGetMacro(PassVerts,int);

  // Description:
  // Turn on/off passing lines through filter.
  vtkBooleanMacro(PassLines,int);
  vtkSetMacro(PassLines,int);
  vtkGetMacro(PassLines,int);

protected:
  vtkGLUTesselatorTriangleFilter();
  ~vtkGLUTesselatorTriangleFilter();

  // Usual data generation method
  void Execute();

        // Data
  int PassVerts;
  int PassLines;
  GLUtesselator *GLUTesselator;

private:
  vtkGLUTesselatorTriangleFilter(const vtkGLUTesselatorTriangleFilter&);  // Not implemented.
  void operator=(const vtkGLUTesselatorTriangleFilter&);  // Not implemented.
};

#endif


