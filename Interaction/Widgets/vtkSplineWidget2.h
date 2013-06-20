/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSplineWidget2.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSplineWidget2 - widget for vtkSplineRepresentation.
// .SECTION Description
// vtkSplineWidget2 is the vtkAbstractWidget subclass for
// vtkSplineRepresentation which manages the interactions with
// vtkSplineRepresentation. This is based on vtkSplineWidget.
// .SECTION See Also
// vtkSplineRepresentation, vtkSplineWidget2

#ifndef __vtkSplineWidget2_h
#define __vtkSplineWidget2_h

#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkAbstractWidget.h"

class vtkSplineRepresentation;

class VTKINTERACTIONWIDGETS_EXPORT vtkSplineWidget2 : public vtkAbstractWidget
{
public:
  static vtkSplineWidget2* New();
  vtkTypeMacro(vtkSplineWidget2, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of
  // vtkProp so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkSplineRepresentation *r)
    {
    this->Superclass::SetWidgetRepresentation(
      reinterpret_cast<vtkWidgetRepresentation*>(r));
    }

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkSplineRepresentation class.
  void CreateDefaultRepresentation();
//BTX
protected:
  vtkSplineWidget2();
  ~vtkSplineWidget2();

  int WidgetState;
  enum _WidgetState {Start=0,Active};

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void TranslateAction(vtkAbstractWidget*);
  static void ScaleAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);

private:
  vtkSplineWidget2(const vtkSplineWidget2&); // Not implemented.
  void operator=(const vtkSplineWidget2&); // Not implemented.
//ETX
};

#endif


