/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkContextItem.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkContextItem - base class for items that are part of a vtkContextScene.
//
// .SECTION Description
// Derive from this class to create custom items that can be added to a
// vtkContextScene.

#ifndef __vtkContextItem_h
#define __vtkContextItem_h

#include "vtkAbstractContextItem.h"
#include "vtkWeakPointer.h" // Needed for weak pointer references to the scene

class vtkContext2D;
class vtkContextScene;
class vtkTransform2D;
class vtkContextMouseEvent;

class VTK_CHARTS_EXPORT vtkContextItem : public vtkAbstractContextItem
{
public:
  vtkTypeMacro(vtkContextItem, vtkAbstractContextItem);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

//BTX
  // Description:
  // Return true if the supplied x, y coordinate is inside the item.
  virtual bool Hit(const vtkContextMouseEvent &mouse);

  // Description:
  // Mouse enter event.
  // Return true if the item holds the event, false if the event can be
  // propagated to other items.
  virtual bool MouseEnterEvent(const vtkContextMouseEvent &mouse);

  // Description:
  // Mouse move event.
  // Return true if the item holds the event, false if the event can be
  // propagated to other items.
  virtual bool MouseMoveEvent(const vtkContextMouseEvent &mouse);

  // Description:
  // Mouse leave event.
  // Return true if the item holds the event, false if the event can be
  // propagated to other items.
  virtual bool MouseLeaveEvent(const vtkContextMouseEvent &mouse);

  // Description:
  // Mouse button down event
  // Return true if the item holds the event, false if the event can be
  // propagated to other items.
  virtual bool MouseButtonPressEvent(const vtkContextMouseEvent &mouse);

  // Description:
  // Mouse button release event.
  // Return true if the item holds the event, false if the event can be
  // propagated to other items.
  virtual bool MouseButtonReleaseEvent(const vtkContextMouseEvent &mouse);

  // Description:
  // Mouse wheel event, positive delta indicates forward movement of the wheel.
  // Return true if the item holds the event, false if the event can be
  // propagated to other items.
  virtual bool MouseWheelEvent(const vtkContextMouseEvent &mouse, int delta);
//ETX

  // Description:
  // Get the visibility of the item (should it be drawn).
  vtkGetMacro(Visible, bool);

  // Description:
  // Set the visibility of the item (should it be drawn).
  vtkSetMacro(Visible, bool);

  // Description:
  // Get the opacity of the item.
  vtkGetMacro(Opacity, double);

  // Description:
  // Set the opacity of the item.
  vtkSetMacro(Opacity, double);

  // Description:
  // Set the vtkContextScene for the item, always set for an item in a scene.
  virtual void SetScene(vtkContextScene *scene);

  // Description:
  // Get the vtkContextScene for the item, always set for an item in a scene.
  vtkContextScene* GetScene();

  // Description:
  // Release graphics resources hold by the item. The default implementation
  // is empty.
  virtual void ReleaseGraphicsResources();

//BTX
protected:
  vtkContextItem();
  ~vtkContextItem();

  vtkWeakPointer<vtkContextScene> Scene;

  bool Visible;

  double Opacity;

private:
  vtkContextItem(const vtkContextItem &); // Not implemented.
  void operator=(const vtkContextItem &);   // Not implemented.
//ETX
};

#endif //__vtkContextItem_h
