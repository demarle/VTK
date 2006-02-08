/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBiDimensionalWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBiDimensionalWidget - measure the bi-dimensional lengths of an object 
// .SECTION Description 
// The vtkBiDimensionalWidget is used to measure the bi-dimensional length of
// an object. The bi-dimensional measure is defined by two finite,
// orthogonal lines that intersect within the finite extent of both lines.
// The lengths of these two lines gives the bi-dimensional measure. Each line
// is defined by two handle widgets at the end points of each line.
//
// The orthognal constraint on the two lines limits how the four end points
// can be positioned. The first two points can be placed arbitrarily to
// define the first line (similar to vtkDistanceWidget). The placement of the
// third point is limited by the finite extent of the first line. As the
// third point is placed, the fourth point is placed on the opposite side of
// the first line. Once the third point is placed, the second line is defined
// since the fourth point is defined at the same time, but the fourth point
// can be moved along the second line (i.e., maintaining the orthogonal
// relationship between the two lines). Onced defined, any of the four points
// can be moved along their constraint lines. Also, each line can be translated
// along the other line (in an orthogonal direction), and the whole
// bi-dimensional widget can be rotated about its center point (see the description
// of the event bindings). Finally, selecting the point where the two orthogonal
// axes intersect, the entire widget can be translated in any direction.
//
// Placement of any point results in a special PlacePointEvent invocation so
// that special operations may be performed to reposition the point. Motion
// of any point, moving the lines, or rotating the widget cause
// InteractionEvents to be invoked. Note that the widget has two fundamental
// modes: a define mode (when initially placing the points) and a manipulate
// mode (after the points are placed). Line translation and rotation are only
// possible in manipulate mode.
// 
// To use this widget, specify an instance of vtkBiDimensionalWidget and a
// representation (e.g., vtkBiDimensionalRepresentation2D). The widget is
// implemented using four instances of vtkHandleWidget which are used to
// position the end points of the two intersecting lines. The representations
// for these handle widgets are provided by the
// vtkBiDimensionalRepresentation2D class.
//
// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
//   LeftButtonPressEvent - define a point or manipulate a handle, line,
//                          perform rotation or translate the widget.
//   MouseMoveEvent - position the points, move a line, rotate or translate the widget
//   LeftButtonReleaseEvent - release the selected handle and end interaction
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events 
// into the vtkBiDimensionalWidget's widget events:
// <pre>
//   vtkWidgetEvent::AddPoint -- (In Define mode:) Add one point; depending on the 
//                               state it may the first, second, third or fourth 
//                               point added. (In Manipulate mode:) If near a handle, 
//                               select the handle. Or if near a line, select the line.
//   vtkWidgetEvent::Move -- (In Define mode:) Position the second, third or fourth 
//                           point. (In Manipulate mode:) Move the handle, line or widget.
//   vtkWidgetEvent::EndSelect -- the manipulation process has completed.
// </pre>
//
// This widget invokes the following VTK events on itself (which observers
// can listen for):
// <pre>
//   vtkCommand::StartInteractionEvent (beginning to interact)
//   vtkCommand::EndInteractionEvent (completing interaction)
//   vtkCommand::InteractionEvent (moving a handle, line or performing rotation)
//   vtkCommand::PlacePointEvent (after a point is positioned; 
//                                call data includes handle id (0,1,2,4))
// </pre>

// .SECTION See Also
// vtkHandleWidget vtkDistanceWidget


#ifndef __vtkBiDimensionalWidget_h
#define __vtkBiDimensionalWidget_h

#include "vtkAbstractWidget.h"

class vtkBiDimensionalRepresentation2D;
class vtkHandleWidget;
class vtkBiDimensionalWidgetCallback;


class VTK_WIDGETS_EXPORT vtkBiDimensionalWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate this class.
  static vtkBiDimensionalWidget *New();

  // Description:
  // Standard methods for a VTK class.
  vtkTypeRevisionMacro(vtkBiDimensionalWidget,vtkAbstractWidget);
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
  void SetRepresentation(vtkBiDimensionalRepresentation2D *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  
  // Description:
  // Create the default widget representation if one is not set. 
  void CreateDefaultRepresentation();

  // Description:
  // A flag indicates whether the bi-dimensional measure is valid. The widget
  // becomes valid after two of the four points are placed.
  int IsMeasureValid();

protected:
  vtkBiDimensionalWidget();
  ~vtkBiDimensionalWidget();

  // The state of the widget
//BTX
  enum {Start=0,Define,Manipulate};
//ETX
  int WidgetState;
  int CurrentHandle;
  int HandleSelected;
  int LineSelected;
  int CenterSelected;

  // Callback interface to capture events when
  // placing the widget.
  static void AddPointAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  
  // The positioning handle widgets
  vtkHandleWidget *Point1Widget;
  vtkHandleWidget *Point2Widget;
  vtkHandleWidget *Point3Widget;
  vtkHandleWidget *Point4Widget;
  vtkBiDimensionalWidgetCallback *BiDimensionalWidgetCallback1;
  vtkBiDimensionalWidgetCallback *BiDimensionalWidgetCallback2;
  vtkBiDimensionalWidgetCallback *BiDimensionalWidgetCallback3;
  vtkBiDimensionalWidgetCallback *BiDimensionalWidgetCallback4;
  
  // Methods invoked when the handles at the
  // end points of the widget are manipulated
  void StartBiDimensionalInteraction();
  void EndBiDimensionalInteraction();
  
//BTX
  friend class vtkBiDimensionalWidgetCallback;
//ETX  

private:
  vtkBiDimensionalWidget(const vtkBiDimensionalWidget&);  //Not implemented
  void operator=(const vtkBiDimensionalWidget&);  //Not implemented
};

#endif
