/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPolyDataPainter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*
 * Copyright 2004 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */

// .NAME vtkPolyDataPainter - Abstract class for drawing poly data.
//
// .SECTION Description
// vtkPolyDataPainter encapsulates a method of drawing poly data.  This is a subset
// of what a mapper does.  The painter does no maintenance of the rendering
// state (camera, lights, etc.).  It is solely responsible for issuing
// rendering commands that build graphics primitives.
//
// To simplify coding, an implementation of vtkPolyDataPainter is allowed to support
// only certain types of poly data or certain types of primitives. 
//
// .SECTION See Also
// vtkDefaultPainter
// vtkStandardPainter
// vtkPainterDeviceAdapter
//

#ifndef __vtkPolyDataPainter_h
#define __vtkPolyDataPainter_h

#include "vtkPainter.h"

class vtkInformationIntegerKey;
class vtkPolyData;

class VTK_RENDERING_EXPORT vtkPolyDataPainter : public vtkPainter
{
public:
  vtkTypeRevisionMacro(vtkPolyDataPainter, vtkPainter);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Get/set the poly data to render.
  vtkGetObjectMacro(PolyData, vtkPolyData);
  virtual void SetPolyData(vtkPolyData *arg);

  // Description:
  // Keys used to specify control the behaviour of the painter.
  // When on, the painter creates normals when none are available in the
  // polydata. On by default.
  static vtkInformationIntegerKey* BUILD_NORMALS();

  // Description:
  // Overridden to stop the render call if input polydata is not set,
  // since PolyDataPainter cannot paint without any polydata input.
  virtual void Render(vtkRenderer* renderer, vtkActor* actor, 
    unsigned long typeflags);

protected:
  vtkPolyDataPainter();
  ~vtkPolyDataPainter();

  vtkPolyData *PolyData;
  int BuildNormals; // ivar synchornized with this->Information before
    // RenderInternal() is called. The ivar are purposefully protected,
    // the only way to affecting these from outside should be using 
    // the information object.
  vtkSetMacro(BuildNormals, int);

  // Description:
  // Pass on the information and data (output) from the current
  // this to the argument painter.
  virtual void PassInformation(vtkPainter* toPainter);

  // Description:
  // Take part in garbage collection.
  virtual void ReportReferences(vtkGarbageCollector *collector);

  // Description:
  // Called before RenderInternal() if the Information has been changed
  // since the last time this method was called.
  virtual void ProcessInformation(vtkInformation*);

  // Description:
  // Get the output polydata from this Painter. The default 
  // implementation forwards the input polydata as the output.
  virtual vtkPolyData* GetOutputData()  { return this->PolyData; }

private:
  vtkPolyDataPainter(const vtkPolyDataPainter &); // Not implemented.
  void operator=(const vtkPolyDataPainter &); // Not implemented.
};

#endif //_vtkPolyDataPainter_h
