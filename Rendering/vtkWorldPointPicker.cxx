/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWorldPointPicker.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkWorldPointPicker.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkWorldPointPicker, "1.18");
vtkStandardNewMacro(vtkWorldPointPicker);

vtkWorldPointPicker::vtkWorldPointPicker()
{
}

// Perform pick operation with selection point provided. The z location
// is recovered from the zBuffer. Always returns 0 since no actors are picked.
int vtkWorldPointPicker::Pick(float selectionX, float selectionY, 
                              float selectionZ, vtkRenderer *renderer)
{
  vtkCamera *camera;
  float cameraFP[4];
  float display[3], *world;
  float *displayCoord;
  float z;

  // Initialize the picking process
  this->Initialize();
  this->Renderer = renderer;
  this->SelectionPoint[0] = selectionX;
  this->SelectionPoint[1] = selectionY;
  this->SelectionPoint[2] = selectionZ;

  // Invoke start pick method if defined
  this->InvokeEvent(vtkCommand::StartPickEvent,NULL);

  // Since renderers are rendered backwards from numlayers->0, the
  // z-buffer data is wiped out for all but the last rendered layer (0).
  // for multiple-layer renderwindows, we need to perform a pick render
  // before accessing z-buffer info, or else we will get renderer[0]'s
  // z-buffer info.
  if (renderer->GetLayer() != 0)
    {
    vtkDebugMacro(<< " Renderer not on layer 0, Getting Z from PickProp() ");
    renderer->PickProp((int) selectionX, (int) selectionY);
    selectionZ = renderer->GetPickedZ();
    vtkDebugMacro(<< " z from transparent renderer: " << selectionZ);
    }
  else
    {
    z = renderer->GetZ ((int) selectionX, (int) selectionY);
    
    // if z is 1.0, we assume the user has picked a point on the
    // screen that has not been rendered into. Use the camera's focal
    // point for the z value. The test value .999999 has to be used
    // instead of 1.0 because for some reason our SGI Infinite Reality
    // engine won't return a 1.0 from the zbuffer
    if (z < 0.999999)
      {
        selectionZ = z;
        vtkDebugMacro(<< " z from zBuffer: " << selectionZ);
      }
    else
      {
        // Get camera focal point and position. Convert to display (screen) 
        // coordinates. We need a depth value for z-buffer.
        camera = renderer->GetActiveCamera();
        camera->GetFocalPoint((float *)cameraFP); cameraFP[3] = 1.0;
        
        renderer->SetWorldPoint(cameraFP);
        renderer->WorldToDisplay();
        displayCoord = renderer->GetDisplayPoint();
        selectionZ = displayCoord[2];
        vtkDebugMacro(<< "computed z from focal point: " << selectionZ);
      }
    }

  // now convert the display point to world coordinates
  display[0] = selectionX;
  display[1] = selectionY;
  display[2] = selectionZ;

  renderer->SetDisplayPoint (display);
  renderer->DisplayToWorld ();
  world = renderer->GetWorldPoint ();
  
  for (int i=0; i < 3; i++) 
    {
    this->PickPosition[i] = world[i] / world[3];
    }

  // Invoke end pick method if defined
  this->InvokeEvent(vtkCommand::EndPickEvent,NULL);

  return 0;
}

void vtkWorldPointPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
