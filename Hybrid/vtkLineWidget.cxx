/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLineWidget.cxx
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
#include "vtkLineWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSphereSource.h"
#include "vtkRenderWindow.h"
#include "vtkPointWidget.h"
#include "vtkCommand.h"

vtkCxxRevisionMacro(vtkLineWidget, "1.32");
vtkStandardNewMacro(vtkLineWidget);

// This class is used to coordinate the interaction between the point widget
// at the center of the line and the line widget. When the line is selected
// (as compared to the handles), a point widget appears at the selection
// point, which can be manipulated in the usual way.
class vtkPWCallback : public vtkCommand
{
public:
  static vtkPWCallback *New() 
    { return new vtkPWCallback; }
  virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
    {
      float x[3], p1[3], p2[3], v[3];
      this->LineWidget->GetPoint1(p1);
      this->LineWidget->GetPoint2(p2);
      this->PointWidget->GetPosition(x);
      v[0] = x[0] - this->LastPosition[0];
      v[1] = x[1] - this->LastPosition[1];
      v[2] = x[2] - this->LastPosition[2];
      this->LineWidget->SetPoint1(p1[0]+v[0],p1[1]+v[1],p1[2]+v[2]);
      this->LineWidget->SetPoint2(p2[0]+v[0],p2[1]+v[1],p2[2]+v[2]);
      this->LastPosition[0] = x[0];
      this->LastPosition[1] = x[1];
      this->LastPosition[2] = x[2];
    }
  vtkPWCallback():LineWidget(0),PointWidget(0) {}
  vtkLineWidget  *LineWidget;
  vtkPointWidget *PointWidget;
  float          LastPosition[3];
};

// This class is used to coordinate the interaction between the point widget
// (point 1) and the line widget.
class vtkPW1Callback : public vtkCommand
{
public:
  static vtkPW1Callback *New() 
    { return new vtkPW1Callback; }
  virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
    {
      this->LineWidget->SetPoint1(this->PointWidget->GetPosition());
    }
  vtkPW1Callback():LineWidget(0),PointWidget(0) {}
  vtkLineWidget  *LineWidget;
  vtkPointWidget *PointWidget;
};

// This class is used to coordinate the interaction between the point widget
// (point 2) and the line widget.
class vtkPW2Callback : public vtkCommand
{
public:
  static vtkPW2Callback *New() 
    { return new vtkPW2Callback; }
  virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
    {
      this->LineWidget->SetPoint2(this->PointWidget->GetPosition());
    }
  vtkPW2Callback():LineWidget(0),PointWidget(0) {}
  vtkLineWidget *LineWidget;
  vtkPointWidget *PointWidget;
};

// Begin the definition of the vtkLineWidget methods
//
vtkLineWidget::vtkLineWidget()
{
  this->State = vtkLineWidget::Start;
  this->EventCallbackCommand->SetCallback(vtkLineWidget::ProcessEvents);
  
  this->Align = vtkLineWidget::XAxis;

  //Build the representation of the widget
  int i;
  // Represent the line
  this->LineSource = vtkLineSource::New();
  this->LineSource->SetResolution(5);
  this->LineMapper = vtkPolyDataMapper::New();
  this->LineMapper->SetInput(this->LineSource->GetOutput());
  this->LineActor = vtkActor::New();
  this->LineActor->SetMapper(this->LineMapper);

  // Create the handles
  this->Handle = new vtkActor* [2];
  this->HandleMapper = new vtkPolyDataMapper* [2];
  this->HandleGeometry = new vtkSphereSource* [2];
  for (i=0; i<2; i++)
    {
    this->HandleGeometry[i] = vtkSphereSource::New();
    this->HandleGeometry[i]->SetThetaResolution(16);
    this->HandleGeometry[i]->SetPhiResolution(8);
    this->HandleMapper[i] = vtkPolyDataMapper::New();
    this->HandleMapper[i]->SetInput(this->HandleGeometry[i]->GetOutput());
    this->Handle[i] = vtkActor::New();
    this->Handle[i]->SetMapper(this->HandleMapper[i]);
    }
  
  // Define the point coordinates
  float bounds[6];
  bounds[0] = -0.5;
  bounds[1] = 0.5;
  bounds[2] = -0.5;
  bounds[3] = 0.5;
  bounds[4] = -0.5;
  bounds[5] = 0.5;

  // Initial creation of the widget, serves to initialize it
  this->PlaceWidget(bounds);

  //Manage the picking stuff
  this->HandlePicker = vtkCellPicker::New();
  this->HandlePicker->SetTolerance(0.001);
  for (i=0; i<2; i++)
    {
    this->HandlePicker->AddPickList(this->Handle[i]);
    }
  this->HandlePicker->PickFromListOn();

  this->LinePicker = vtkCellPicker::New();
  this->LinePicker->SetTolerance(0.005); //need some fluff
  this->LinePicker->AddPickList(this->LineActor);
  this->LinePicker->PickFromListOn();
  
  this->CurrentHandle = NULL;

  // Set up the initial properties
  this->CreateDefaultProperties();
  
  // Create the point widgets and associated callbacks
  this->PointWidget  = vtkPointWidget::New();
  this->PointWidget->AllOff();

  this->PointWidget1 = vtkPointWidget::New();
  this->PointWidget1->AllOff();

  this->PointWidget2 = vtkPointWidget::New();
  this->PointWidget2->AllOff();

  this->PWCallback = vtkPWCallback::New();
  this->PWCallback->LineWidget = this;
  this->PWCallback->PointWidget = this->PointWidget;
  this->PW1Callback = vtkPW1Callback::New();
  this->PW1Callback->LineWidget = this;
  this->PW1Callback->PointWidget = this->PointWidget1;
  this->PW2Callback = vtkPW2Callback::New();
  this->PW2Callback->LineWidget = this;
  this->PW2Callback->PointWidget = this->PointWidget2;  

  // Very tricky, the point widgets watch for their own
  // interaction events.
  this->PointWidget->AddObserver(vtkCommand::InteractionEvent,
                                  this->PWCallback, 0.0);
  this->PointWidget1->AddObserver(vtkCommand::InteractionEvent,
                                  this->PW1Callback, 0.0);
  this->PointWidget2->AddObserver(vtkCommand::InteractionEvent,
                                  this->PW2Callback, 0.0);
  this->CurrentPointWidget = NULL;
}

vtkLineWidget::~vtkLineWidget()
{
  this->LineActor->Delete();
  this->LineMapper->Delete();
  this->LineSource->Delete();

  for (int i=0; i<2; i++)
    {
    this->HandleGeometry[i]->Delete();
    this->HandleMapper[i]->Delete();
    this->Handle[i]->Delete();
    }
  delete [] this->Handle;
  delete [] this->HandleMapper;
  delete [] this->HandleGeometry;
  
  this->HandlePicker->Delete();
  this->LinePicker->Delete();

  this->HandleProperty->Delete();
  this->SelectedHandleProperty->Delete();
  this->LineProperty->Delete();
  this->SelectedLineProperty->Delete();

  this->PointWidget->RemoveObserver(this->PWCallback);
  this->PointWidget1->RemoveObserver(this->PW1Callback);
  this->PointWidget2->RemoveObserver(this->PW2Callback);
  this->PointWidget->Delete();
  this->PointWidget1->Delete();
  this->PointWidget2->Delete();
  this->PWCallback->Delete();
  this->PW1Callback->Delete();
  this->PW2Callback->Delete();
}

void vtkLineWidget::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }

  if ( enabling ) //-----------------------------------------------------------
    {
    vtkDebugMacro(<<"Enabling line widget");

    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }
    
    this->CurrentRenderer = this->Interactor->FindPokedRenderer(this->Interactor->GetLastEventPosition()[0],this->Interactor->GetLastEventPosition()[1]);
    if (this->CurrentRenderer == NULL)
      {
      return;
      }

    this->Enabled = 1;

    // listen for the following events
    vtkRenderWindowInteractor *i = this->Interactor;
    i->AddObserver(vtkCommand::MouseMoveEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::MiddleButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::MiddleButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::RightButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::RightButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);

    // Add the line
    this->CurrentRenderer->AddActor(this->LineActor);
    this->LineActor->SetProperty(this->LineProperty);

    // turn on the handles
    for (int j=0; j<2; j++)
      {
      this->CurrentRenderer->AddActor(this->Handle[j]);
      this->Handle[j]->SetProperty(this->HandleProperty);
      }

    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
  
  else //disabling----------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling line widget");

    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }
    
    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);

    // turn off the line
    this->CurrentRenderer->RemoveActor(this->LineActor);

    // turn off the handles
    for (int i=0; i<2; i++)
      {
      this->CurrentRenderer->RemoveActor(this->Handle[i]);
      }

    this->CurrentHandle = NULL;
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    }

  this->Interactor->Render();
}

void vtkLineWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                  unsigned long event,
                                  void* clientdata, 
                                  void* vtkNotUsed(calldata))
{
  vtkLineWidget* self = reinterpret_cast<vtkLineWidget *>( clientdata );

  //okay, let's do the right thing
  switch(event)
    {
    case vtkCommand::LeftButtonPressEvent:
      self->OnLeftButtonDown();
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      self->OnLeftButtonUp();
      break;
    case vtkCommand::MiddleButtonPressEvent:
      self->OnMiddleButtonDown();
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      self->OnMiddleButtonUp();
      break;
    case vtkCommand::RightButtonPressEvent:
      self->OnRightButtonDown();
      break;
    case vtkCommand::RightButtonReleaseEvent:
      self->OnRightButtonUp();
      break;
    case vtkCommand::MouseMoveEvent:
      self->OnMouseMove();
      break;
    }
}

void vtkLineWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->HandleProperty )
    {
    os << indent << "Handle Property: " << this->HandleProperty << "\n";
    }
  else
    {
    os << indent << "Handle Property: (none)\n";
    }
  if ( this->SelectedHandleProperty )
    {
    os << indent << "Selected Handle Property: " 
       << this->SelectedHandleProperty << "\n";
    }
  else
    {
    os << indent << "Selected Handle Property: (none)\n";
    }

  if ( this->LineProperty )
    {
    os << indent << "Line Property: " << this->LineProperty << "\n";
    }
  else
    {
    os << indent << "Line Property: (none)\n";
    }
  if ( this->SelectedLineProperty )
    {
    os << indent << "Selected Line Property: " 
       << this->SelectedLineProperty << "\n";
    }
  else
    {
    os << indent << "Selected Line Property: (none)\n";
    }

  os << indent << "Align with: ";
  switch ( this->Align ) 
    {
    case XAxis:
      os << "X Axis";
      break;
    case YAxis:
      os << "Y Axis";
      break;
    case ZAxis:
      os << "Z Axis";
      break;
    default:
      os << "None";
    }
  int res = this->LineSource->GetResolution();
  float *pt1 = this->LineSource->GetPoint1();
  float *pt2 = this->LineSource->GetPoint2();

  os << indent << "Resolution: " << res << "\n";
  os << indent << "Point 1: (" << pt1[0] << ", "
                               << pt1[1] << ", "
                               << pt1[2] << ")\n";
  os << indent << "Point 2: (" << pt2[0] << ", "
                               << pt2[1] << ", "
                               << pt2[2] << ")\n";
}

void vtkLineWidget::PositionHandles()
{
  //int res = this->LineSource->GetResolution();
  float *pt1 = this->LineSource->GetPoint1();
  float *pt2 = this->LineSource->GetPoint2();

  this->HandleGeometry[0]->SetCenter(pt1);
  this->HandleGeometry[1]->SetCenter(pt2);
}

int vtkLineWidget::HighlightHandle(vtkProp *prop)
{
  // first unhighlight anything picked
  if ( this->CurrentHandle )
    {
    this->CurrentHandle->SetProperty(this->HandleProperty);
    }

  // set the current handle
  this->CurrentHandle = (vtkActor *)prop;

  // find the current handle
  if ( this->CurrentHandle )
    {
    this->HandlePicker->GetPickPosition(this->LastPickPosition);
    this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
    return (this->CurrentHandle == this->Handle[0] ? 0 : 1);
    }
  else
    {
    return -1;
    }
}

int vtkLineWidget::ForwardEvent(unsigned long event)
{
  if ( ! this->CurrentPointWidget )
    {
    return 0;
    }
  
  this->CurrentPointWidget->ProcessEvents(this,event,
                                          this->CurrentPointWidget,NULL);
  
  return 1;
}

// assumed current handle is set
void vtkLineWidget::EnablePointWidget()
{
  // Set up the point widgets
  float x[3];
  if ( this->CurrentHandle ) //picking the handles
    {
    if ( this->CurrentHandle == this->Handle[0] )
      {
      this->CurrentPointWidget = this->PointWidget1;
      this->LineSource->GetPoint1(x);
      }
    else
      {
      this->CurrentPointWidget = this->PointWidget2;
      this->LineSource->GetPoint2(x);
      }
    }
  else //picking the line
    {
    this->CurrentPointWidget = this->PointWidget;
    this->LinePicker->GetPickPosition(x);
    this->PWCallback->LastPosition[0] = x[0];
    this->PWCallback->LastPosition[1] = x[1];
    this->PWCallback->LastPosition[2] = x[2];
    }
  
  float bounds[6];
  bounds[0] = x[0] - 0.1*this->InitialLength;
  bounds[1] = x[0] + 0.1*this->InitialLength;
  bounds[2] = x[1] - 0.1*this->InitialLength;
  bounds[3] = x[1] + 0.1*this->InitialLength;
  bounds[4] = x[2] - 0.1*this->InitialLength;
  bounds[5] = x[2] + 0.1*this->InitialLength;

  // Note: translation mode is disabled and enabled to control
  // the proper positioning of the bounding box.
  this->CurrentPointWidget->SetInteractor(this->Interactor);
  this->CurrentPointWidget->TranslationModeOff();
  this->CurrentPointWidget->SetPlaceFactor(1.0);
  this->CurrentPointWidget->PlaceWidget(bounds);
  this->CurrentPointWidget->TranslationModeOn();
  this->CurrentPointWidget->SetPosition(x);
  this->CurrentPointWidget->On();
}

// assumed current handle is set
void vtkLineWidget::DisablePointWidget()
{
  this->CurrentPointWidget->Off();
  this->CurrentPointWidget = NULL;
}

void vtkLineWidget::HighlightHandles(int highlight)
{
  if ( highlight )
    {
    this->HandlePicker->GetPickPosition(this->LastPickPosition);
    this->Handle[0]->SetProperty(this->SelectedHandleProperty);
    this->Handle[1]->SetProperty(this->SelectedHandleProperty);
    }
  else
    {
    this->Handle[0]->SetProperty(this->HandleProperty);
    this->Handle[1]->SetProperty(this->HandleProperty);
    }
}

void vtkLineWidget::HighlightLine(int highlight)
{
  if ( highlight )
    {
    this->LinePicker->GetPickPosition(this->LastPickPosition);
    this->LineActor->SetProperty(this->SelectedLineProperty);
    }
  else
    {
    this->LineActor->SetProperty(this->LineProperty);
    }
}

void vtkLineWidget::OnLeftButtonDown()
{
  int forward=0;

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  this->Interactor->FindPokedRenderer(X,Y);
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->State = vtkLineWidget::MovingHandle;
    this->HighlightHandle(path->GetFirstNode()->GetProp());
    this->EnablePointWidget();
    forward = this->ForwardEvent(vtkCommand::LeftButtonPressEvent);
    }
  else
    {
    this->LinePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->LinePicker->GetPath();
    if ( path != NULL )
      {
      this->State = vtkLineWidget::MovingLine;
      this->HighlightLine(1);
      this->EnablePointWidget();
      forward = this->ForwardEvent(vtkCommand::LeftButtonPressEvent);
      }
    else
      {
      this->State = vtkLineWidget::Outside;
      this->HighlightHandle(NULL);
      return;
      }
    }
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  if ( ! forward )
    {
    this->Interactor->Render();
    }
}

void vtkLineWidget::OnLeftButtonUp()
{
  if ( this->State == vtkLineWidget::Outside ||
       this->State == vtkLineWidget::Start )
    {
    return;
    }

  this->State = vtkLineWidget::Start;
  this->HighlightHandle(NULL);
  this->HighlightLine(0);

  int forward = this->ForwardEvent(vtkCommand::LeftButtonReleaseEvent);
  this->DisablePointWidget();
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  if ( ! forward )
    {
    this->Interactor->Render();
    }
}

void vtkLineWidget::OnMiddleButtonDown()
{
  int forward=0;

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkAssemblyPath *path;
  this->Interactor->FindPokedRenderer(X,Y);
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->State = vtkLineWidget::MovingLine;
    this->HighlightHandles(1);
    this->HighlightLine(1);
    this->EnablePointWidget();
    this->ForwardEvent(vtkCommand::LeftButtonPressEvent);
    }
  else
    {
    this->LinePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->LinePicker->GetPath();
    if ( path != NULL )
      {
      //note that the haighlight methods set the LastPickPosition, so they are ordered
      this->HighlightHandles(1);
      this->HighlightLine(1);
      this->State = vtkLineWidget::MovingLine;
      this->EnablePointWidget();
      this->ForwardEvent(vtkCommand::LeftButtonPressEvent);
      }
    else
      {
      this->State = vtkLineWidget::Outside;
      return;
      }
    }
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  if ( ! forward )
    {
    this->Interactor->Render();
    }
}

void vtkLineWidget::OnMiddleButtonUp()
{
  if ( this->State == vtkLineWidget::Outside ||
       this->State == vtkLineWidget::Start )
    {
    return;
    }

  this->State = vtkLineWidget::Start;
  this->HighlightLine(0);
  this->HighlightHandles(0);
  
  int forward = this->ForwardEvent(vtkCommand::LeftButtonReleaseEvent);
  this->DisablePointWidget();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  if ( ! forward )
    {
    this->Interactor->Render();
    }
}

void vtkLineWidget::OnRightButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkAssemblyPath *path;
  this->Interactor->FindPokedRenderer(X,Y);
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->HighlightLine(1);
    this->HighlightHandles(1);
    this->State = vtkLineWidget::Scaling;
    }
  else
    {
    this->LinePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->LinePicker->GetPath();
    if ( path != NULL )
      {
      this->HighlightHandles(1);
      this->HighlightLine(1);
      this->State = vtkLineWidget::Scaling;
      }
    else
      {
      this->State = vtkLineWidget::Outside;
      this->HighlightLine(0);
      return;
      }
    }
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkLineWidget::OnRightButtonUp()
{
  if ( this->State == vtkLineWidget::Outside ||
       this->State == vtkLineWidget::Start )
    {
    return;
    }

  this->State = vtkLineWidget::Start;
  this->HighlightLine(0);
  this->HighlightHandles(0);
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkLineWidget::OnMouseMove()
{
  // See whether we're active
  if ( this->State == vtkLineWidget::Outside || 
       this->State == vtkLineWidget::Start )
    {
    return;
    }
  
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z;

  vtkRenderer *renderer = this->Interactor->FindPokedRenderer(X,Y);
  vtkCamera *camera = renderer->GetActiveCamera();
  if ( !camera )
    {
    return;
    }

  // Compute the two points defining the motion vector
  this->ComputeWorldToDisplay(this->LastPickPosition[0], this->LastPickPosition[1],
                              this->LastPickPosition[2], focalPoint);
  z = focalPoint[2];
  this->ComputeDisplayToWorld(double(this->Interactor->GetLastEventPosition()[0]),
                              double(this->Interactor->GetLastEventPosition()[1]),
                              z, prevPickPoint);
  this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);

  // Process the motion
  int forward=0;
  if ( this->State == vtkLineWidget::MovingHandle )
    {
    forward = this->ForwardEvent(vtkCommand::MouseMoveEvent);
    }
  else if ( this->State == vtkLineWidget::MovingLine )
    {
    forward = this->ForwardEvent(vtkCommand::MouseMoveEvent);
    }
  else if ( this->State == vtkLineWidget::Scaling )
    {
    this->Scale(prevPickPoint, pickPoint, X, Y);
    }

  // Interact, if desired
  this->EventCallbackCommand->SetAbortFlag(1);
  this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  if ( ! forward )
    {
    this->Interactor->Render();
    }
}

void vtkLineWidget::Scale(double *p1, double *p2, int vtkNotUsed(X), int Y)
{
  //Get the motion vector
  double v[3];
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  //int res = this->LineSource->GetResolution();
  float *pt1 = this->LineSource->GetPoint1();
  float *pt2 = this->LineSource->GetPoint2();

  float center[3];
  center[0] = (pt1[0]+pt2[0]) / 2.0;
  center[1] = (pt1[1]+pt2[1]) / 2.0;
  center[2] = (pt1[2]+pt2[2]) / 2.0;

  // Compute the scale factor
  float sf = vtkMath::Norm(v) / sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));
  if ( Y > this->Interactor->GetLastEventPosition()[1] )
    {
    sf = 1.0 + sf;
    }
  else
    {
    sf = 1.0 - sf;
    }
  
  // Move the end points
  float point1[3], point2[3];
  for (int i=0; i<3; i++)
    {
    point1[i] = sf * (pt1[i] - center[i]) + center[i];
    point2[i] = sf * (pt2[i] - center[i]) + center[i];
    }

  this->LineSource->SetPoint1(point1);
  this->LineSource->SetPoint2(point2);
  this->LineSource->Update();

  this->PositionHandles();
}

void vtkLineWidget::CreateDefaultProperties()
{
  // Handle properties
  this->HandleProperty = vtkProperty::New();
  this->HandleProperty->SetColor(1,1,1);

  this->SelectedHandleProperty = vtkProperty::New();
  this->SelectedHandleProperty->SetColor(1,0,0);
  
  // Line properties
  this->LineProperty = vtkProperty::New();
  this->LineProperty->SetRepresentationToWireframe();
  this->LineProperty->SetAmbient(1.0);
  this->LineProperty->SetAmbientColor(1.0,1.0,1.0);
  this->LineProperty->SetLineWidth(2.0);

  this->SelectedLineProperty = vtkProperty::New();
  this->SelectedLineProperty->SetRepresentationToWireframe();
  this->SelectedLineProperty->SetAmbient(1.0);
  this->SelectedLineProperty->SetAmbientColor(0.0,1.0,0.0);
  this->SelectedLineProperty->SetLineWidth(2.0);
}

void vtkLineWidget::PlaceWidget(float bds[6])
{
  int i;
  float bounds[6], center[3];

  this->AdjustBounds(bds, bounds, center);
  
  if ( this->Align == vtkLineWidget::YAxis )
    {
    this->LineSource->SetPoint1(center[0],bounds[2],center[2]);
    this->LineSource->SetPoint2(center[0],bounds[3],center[2]);
    }
  else if ( this->Align == vtkLineWidget::ZAxis )
    {
    this->LineSource->SetPoint1(center[0],center[1],bounds[4]);
    this->LineSource->SetPoint2(center[0],center[1],bounds[5]);
    }
  else if ( this->Align == vtkLineWidget::XAxis )//default or x-aligned
    {
    this->LineSource->SetPoint1(bounds[0],center[1],center[2]);
    this->LineSource->SetPoint2(bounds[1],center[1],center[2]);
    }
  this->LineSource->Update();

  // Position the handles at the end of the lines
  this->PositionHandles();

  for (i=0; i<6; i++)
    {
    this->InitialBounds[i] = bounds[i];
    }
  this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                             (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                             (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));
  for(i=0; i<2; i++)
    {
    this->HandleGeometry[i]->SetRadius(0.025*this->InitialLength);
    }
}

void vtkLineWidget::SetPoint1(float x, float y, float z) 
{
  this->LineSource->SetPoint1(x,y,z); 
  this->PositionHandles();
}

void vtkLineWidget::SetPoint2(float x, float y, float z) 
{
  this->LineSource->SetPoint2(x,y,z); 
  this->PositionHandles();
}

