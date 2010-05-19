/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTransformCoordinateSystems.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkTransformCoordinateSystems - transform points into different coordinate systems
// .SECTION Description
// This filter transforms points from one coordinate system to another. The user
// must specify the coordinate systems in which the input and output are
// specified. The user must also specify the VTK viewport (i.e., renderer) in
// which the transformation occurs.
//
// .SECTION See Also
// vtkCoordinate vtkTransformFilter vtkTransformPolyData vtkPolyDataMapper2D

#ifndef __vtkTransformCoordinateSystems_h
#define __vtkTransformCoordinateSystems_h

#include "vtkPointSetAlgorithm.h"
#include "vtkCoordinate.h" //to get the defines in vtkCoordinate

class VTK_GRAPHICS_EXPORT vtkTransformCoordinateSystems : public vtkPointSetAlgorithm
{
public:
  // Description:
  // Standard methods for type information and printing.
  vtkTypeMacro(vtkTransformCoordinateSystems,vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description
  // Instantiate this class. By default no transformation is specified and
  // the input and output is identical.
  static vtkTransformCoordinateSystems *New();

  // Description:
  // Set/get the coordinate system in which the input is specified.
  // The current options are World, Viewport, and Display. By default the
  // input coordinate system is World.
  vtkSetMacro(InputCoordinateSystem, int);
  vtkGetMacro(InputCoordinateSystem, int);
  void SetInputCoordinateSystemToDisplay() 
    {this->SetInputCoordinateSystem(VTK_DISPLAY);}
  void SetInputCoordinateSystemToViewport() 
    {this->SetInputCoordinateSystem(VTK_VIEWPORT);}
  void SetInputCoordinateSystemToWorld() 
    {this->SetInputCoordinateSystem(VTK_WORLD);}
    
  // Description:
  // Set/get the coordinate system to which to transform the output.
  // The current options are World, Viewport, and Display. By default the
  // output coordinate system is Display.
  vtkSetMacro(OutputCoordinateSystem, int);
  vtkGetMacro(OutputCoordinateSystem, int);
  void SetOutputCoordinateSystemToDisplay() 
    {this->SetOutputCoordinateSystem(VTK_DISPLAY);}
  void SetOutputCoordinateSystemToViewport() 
    {this->SetOutputCoordinateSystem(VTK_VIEWPORT);}
  void SetOutputCoordinateSystemToWorld() 
    {this->SetOutputCoordinateSystem(VTK_WORLD);}
    
  // Description:
  // Return the MTime also considering the instance of vtkCoordinate.
  unsigned long GetMTime();

  // Description:
  // In order for successful coordinate transformation to occur, an
  // instance of vtkViewport (e.g., a VTK renderer) must be specified.
  // NOTE: this is a raw pointer, not a weak pointer not a reference counted
  // object to avoid reference cycle loop between rendering classes and filter
  // classes.
  void SetViewport(vtkViewport *viewport);
  vtkGetObjectMacro(Viewport,vtkViewport);

protected:
  vtkTransformCoordinateSystems();
  ~vtkTransformCoordinateSystems();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int InputCoordinateSystem;
  int OutputCoordinateSystem;
  vtkViewport *Viewport;

  vtkCoordinate *TransformCoordinate;

private:
  vtkTransformCoordinateSystems(const vtkTransformCoordinateSystems&);  // Not implemented.
  void operator=(const vtkTransformCoordinateSystems&);  // Not implemented.
};

#endif
