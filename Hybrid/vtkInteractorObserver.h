/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInteractorObserver.h
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
// .NAME vtkInteractorObserver - an abstract superclass for classes observing events invoked by vtkRenderWindowInteractor
// .SECTION Description
// vtkInteractorObserver is an abstract superclass for classes that observe events
// invoked by vtkRenderWindowInteractor.
//
// vtkInteractorObserver defines the method SetInteractor() and enables and
// disables the processing of events by the vtkInteractorObserver. Use the
// methods EnabledOn() or SetEnabled(1) to turn on the interactor observer,
// and the methods EnabledOff() or SetEnabled(0) to turn off the interactor.
//
// To support interactive manipulation of objects, this class (and
// subclasses) invoke the events StartInteractionEvent, InteractionEvent, and
// EndInteractionEvent.  These events are invoked when the vtkInteractorObserver enters
// a state where rapid response is desired: mouse motion, etc. The events can
// be used, for example, to set the desired update frame rate
// (StartInteractionEvent), operate on data or update a pipeline (InteractionEvent), 
// and set the desired frame rate back to normal values (EndInteractionEvent). Two 
// other events, EnableEvent and DisableEvent, are invoked when the interactor 
// observer is enabled or disabled.

// .SECTION See Also
// vtk3DWidget vtkBoxWidget vtkLineWidget

#ifndef __vtkInteractorObserver_h
#define __vtkInteractorObserver_h

#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"

class vtkCallbackCommand;

class VTK_EXPORT vtkInteractorObserver : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkInteractorObserver,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Methods for turning the interactor observer on and off, and determining
  // its state. All subclasses must provide the SetEnabled() method.
  // Enabling a vtkInteractorObserver has the side effect of adding observers;
  // disabling it removes the observers. Prior to enabling the vtkInteractorObserver
  // you must set the render window interactor (via SetInteractor()).
  virtual void SetEnabled(int) = 0;
  int GetEnabled() {return this->Enabled;}
  void EnabledOn() {this->SetEnabled(1);}
  void EnabledOff() {this->SetEnabled(0);}
  void On() {this->SetEnabled(1);}
  void Off() {this->SetEnabled(0);}

  // Description:
  // This method is used to associate the widget with the render window
  // interactor.  Observers of the appropriate events invoked in the render
  // window interactor are set up as a result of this method invocation.
  // The SetInteractor() method must be invoked prior to enabling the
  // vtkInteractorObserver.
  virtual void SetInteractor(vtkRenderWindowInteractor* iren);
  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);

  // Description:
  // Set/Get the priority at which events are processed. This is used when
  // multiple interactor observers are used simultaneously. The default value is 1.0
  // (highest priority.) Note that when multiple interactor observer have
  // the same priority, then the last observer added will process the event
  // first.
  vtkSetClampMacro(Priority,float,0.0,1.0);
  vtkGetMacro(Priority,float);

  // Description:
  // Enable/Disable of the use of a keypress to turn on and off the
  // interactor observer. (By default, the keypress is 'i' for "interactor observer".)
  // Set the KeyPressActivationValue to change which key activates the widget.)
  vtkSetMacro(KeyPressActivation,int);
  vtkGetMacro(KeyPressActivation,int);
  vtkBooleanMacro(KeyPressActivation,int);
  
  // Description:
  // Specify which key press value to use to activate the interactor observer 
  // (if key press activation is enabled). By default, the key press activation 
  // value is 'i'. Note: once the SetInteractor() method is invoked, changing the key press
  // activation value will not affect the key press until SetInteractor is 
  // called again.
  vtkSetMacro(KeyPressActivationValue,char);
  vtkGetMacro(KeyPressActivationValue,char);

protected:
  vtkInteractorObserver();
  ~vtkInteractorObserver();

  //handles the char widget activation event
  static void ProcessEvents(vtkObject* object, unsigned long event,
                            void* clientdata, void* calldata);

  // Sets up the keypress-W event. 
  // Should be invoked by subclass' ProcessEvents()
  void OnChar(int ctrl, int shift, char keycode, int repeatcount);
  
  // helper method for subclasses
  void ComputeDisplayToWorld(double x, double y, double z, double *worldPt);
  void ComputeWorldToDisplay(double x, double y, double z, double *displayPt);
    
  // The state of the widget, whether on or off (observing events or not)
  int Enabled;
  
  // Used to process events
  vtkCallbackCommand* EventCallbackCommand;
  vtkCallbackCommand* KeypressCallbackCommand; //listens to key activation

  // Priority at which events are processed
  float Priority;

  // Keypress activation controls
  int KeyPressActivation;
  char KeyPressActivationValue;

  // Used to associate observers with the interactor
  vtkRenderWindowInteractor *Interactor;
  
  // Internal ivars for processing events
  vtkRenderer *CurrentRenderer;
  vtkCamera *CurrentCamera;
  float OldX;
  float OldY;

private:
  vtkInteractorObserver(const vtkInteractorObserver&);  // Not implemented.
  void operator=(const vtkInteractorObserver&);  // Not implemented.
  
};

#endif
