/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtk3DWidget.h
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
// .NAME vtk3DWidget - an abstract superclass for 3D widgets
// .SECTION Description
// vtk3DWidget is an abstract superclass for 3D interactor observers. These
// 3D widgets represent themselves in the scene, and have special callbacks
// associated with them that allows interactive manipulation of the widget.
// Inparticular, the difference between a vtk3DWidget and its abstract
// superclass vtkInteractorObserver is that vtk3DWidgets are "placed" in 3D
// space.  vtkInteractorObservers have no notion of where they are placed,
// and may not exist in 3D space at all.  3D widgets also provide auxiliary
// functions like producing a transformation, creating polydata (for seeding
// streamlines, probes, etc.) or creating implicit functions. See the
// concrete subclasses for particulars.
//
// Typically the widget is used by specifying a vtkProp3D or VTK dataset as
// input, and then invoking the "On" method to activate it. (You can also
// specify a bounding box to help position the widget.) Prior to invoking the
// On() method, the user may also wish to use the PlaceWidget() to initially
// position it. The 'W' (for "widget") keypresses also can be used to
// turn the widgets on and off (methods exist to change the key value
// and enable keypress activiation).
// 
// To support interactive manipulation of objects, this class (and
// subclasses) invoke the events StartInteractionEvent, InteractionEvent, and
// EndInteractionEvent.  These events are invoked when the vtk3DWidget enters
// a state where rapid response is desired: mouse motion, etc. The events can
// be used, for example, to set the desired update frame rate
// (StartInteractionEvent), operate on the vtkProp3D or other object
// (InteractionEvent), and set the desired frame rate back to normal values
// (EndInteractionEvent).

// .SECTION See Also
// vtkBoxWidget vtkLineWidget

#ifndef __vtk3DWidget_h
#define __vtk3DWidget_h

#include "vtkInteractorObserver.h"

class VTK_EXPORT vtk3DWidget : public vtkInteractorObserver
{
public:
  vtkTypeRevisionMacro(vtk3DWidget,vtkInteractorObserver);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method is used to initially place the widget.  The placement of the
  // widget depends on whether a Prop3D or input dataset is provided. If one
  // of these two is provided, they will be used to obtain a bounding box,
  // around which the widget is placed. Otherwise, you can manually specify a
  // bounds with the PlaceWidget(bounds) method.
  virtual void PlaceWidget();

  // Description:
  // Place the widget within the bounding box provided.
  // Subclasses must provide this method.
  virtual void PlaceWidget(float bounds[6]) = 0;

  // Description:
  // Specify a vtkProp3D around which to place the widget. This 
  // is not required, but if supplied, it is used to initially 
  // position the widget.
  vtkSetObjectMacro(Prop3D,vtkProp3D);
  vtkGetObjectMacro(Prop3D,vtkProp3D);
  
  // Description:
  // Specify the input dataset. This is not required, but if supplied,
  // and no vtkProp3D is specified, it is used to initially position 
  // the widget.
  vtkSetObjectMacro(Input,vtkDataSet);
  vtkGetObjectMacro(Input,vtkDataSet);
  
protected:
  vtk3DWidget();
  ~vtk3DWidget();

  // Used to position and scale the widget initially
  vtkProp3D *Prop3D;
  vtkDataSet *Input;
  
  //has the widget ever been placed
  int Placed; 
  
private:
  vtk3DWidget(const vtk3DWidget&);  // Not implemented.
  void operator=(const vtk3DWidget&);  // Not implemented.
  
};

#endif
