/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDistanceWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDistanceWidget - measure the distance between two points
// .SECTION Description
// The vtkDistanceWidget is used to measure the distance between two points.
// The two end points can be positioned independently, and when they are
// released, a special PlacePointEvent is invoked so that special operations
// may be take to reposition the point (snap to grid, etc.)
// 
// To use this widget, specify an instance of vtkDistanceWidget and a
// representation (a subclass of vtkDistanceRepresentation). The widget is
// implemented using two instances of vtkHandleWidget which are used to
// position the end points of the line. The representations for these two
// handle widgets are provided by the vtkDistanceRepresentation.
//
// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
//   LeftButtonPressEvent - add a point or select a handle 
//   MouseMoveEvent - position the second point or move a handle
//   LeftButtonReleaseEvent - release the handle
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events 
// into the vtkSliderWidget's widget events:
// <pre>
//   vtkWidgetEvent::AddPoint -- add one point; depending on the state
//                               it may the first or second point added. Or,
//                               if near handle, select handle.
//   vtkWidgetEvent::Move -- move the second point or handle depending on the state.
//   vtkWidgetEvent::EndSelect -- the handle manipulation process has completed.
// </pre>
//
// This widget invokes the following VTK events on itself (which observers
// can listen for):
// <pre>
//   vtkCommand::StartInteractionEvent (beginning to interact)
//   vtkCommand::EndInteractionEvent (completing interaction)
//   vtkCommand::InteractionEvent (moving after selecting something)
//   vtkCommand::PlacePointEvent (after point is positioned; 
//                                call data includes handle id (0,1))
// </pre>

// .SECTION See Also
// vtkHandleWidget 


#ifndef __vtkDistanceWidget_h
#define __vtkDistanceWidget_h

#include "vtkAbstractWidget.h"

class vtkDistanceRepresentation;
class vtkHandleWidget;
class vtkDistanceWidgetCallback;


class VTK_WIDGETS_EXPORT vtkDistanceWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate this class.
  static vtkDistanceWidget *New();

  // Description:
  // Standard methods for a VTK class.
  vtkTypeRevisionMacro(vtkDistanceWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The method for activiating and deactiviating this widget. This method
  // must be overridden because it is a composite widget and does more than
  // its superclasses' vtkAbstractWidget::SetEnabled() method.
  virtual void SetEnabled(int);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkDistanceRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  vtkDistanceRepresentation *GetRepresentation()
    {
      this->CreateDefaultRepresentation();
      return reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep);
    }
  
  // Description:
  // Create the default widget representation if one is not set. 
  void CreateDefaultRepresentation();

protected:
  vtkDistanceWidget();
  ~vtkDistanceWidget();

  // The state of the widget
//BTX
  enum {Start=0,Define,Manipulate};
//ETX
  int WidgetState;
  int CurrentHandle;

  // Callback interface to capture events when
  // placing the widget.
  static void AddPointAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  
  // The positioning handle widgets
  vtkHandleWidget *Point1Widget;
  vtkHandleWidget *Point2Widget;
  vtkDistanceWidgetCallback *MeasureWidgetCallback1;
  vtkDistanceWidgetCallback *MeasureWidgetCallback2;
  
  // Methods invoked when the handles at the
  // end points of the widget are manipulated
  void StartMeasureInteraction(int handleNum);
  void MeasureInteraction(int handleNum);
  void EndMeasureInteraction(int handleNum);
  
//BTX
  friend class vtkDistanceWidgetCallback;
//ETX  

private:
  vtkDistanceWidget(const vtkDistanceWidget&);  //Not implemented
  void operator=(const vtkDistanceWidget&);  //Not implemented
};

#endif
