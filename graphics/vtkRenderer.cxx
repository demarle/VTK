/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRenderer.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include <stdlib.h>
#include <string.h>

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkMath.h"
#include "vtkVolume.h"
#include "vtkRayCaster.h"

// Description:
// Create a vtkRenderer with a black background, a white ambient light, 
// two-sided lighting turned on, a viewport of (0,0,1,1), and backface culling
// turned off.
vtkRenderer::vtkRenderer()
{
  this->ActiveCamera = NULL;

  this->Ambient[0] = 1;
  this->Ambient[1] = 1;
  this->Ambient[2] = 1;

  this->RayCaster = vtkRayCaster::New();
  this->RayCaster->SetRenderer( this );

  this->AllocatedRenderTime = 0;
  
  this->CreatedLight = NULL;
  
  this->TwoSidedLighting = 1;
  this->BackingStore = 0;
  this->BackingImage = NULL;
}

vtkRenderer::~vtkRenderer()
{
  if (this->ActiveCamera)
    {
    this->ActiveCamera->UnRegister(this);
    this->ActiveCamera = NULL;
    }

  if (this->CreatedLight)
    {
    this->CreatedLight->UnRegister(this);
    this->CreatedLight = NULL;
    }

  this->RayCaster->Delete();
  if (this->BackingImage) delete [] this->BackingImage;
}

#ifdef VTK_USE_GLR
#include "vtkGLRenderer.h"
#endif
#ifdef VTK_USE_OGLR
#include "vtkOpenGLRenderer.h"
#endif
#ifdef VTK_USE_SBR
#include "vtkStarbaseRenderer.h"
#endif
#ifdef VTK_USE_XGLR
#include "vtkXGLRenderer.h"
#endif
#ifdef _WIN32
#include "vtkOpenGLRenderer.h"
#endif
// return the correct type of Renderer 
vtkRenderer *vtkRenderer::New()
{
  char *temp = vtkRenderWindow::GetRenderLibrary();
  
#ifdef VTK_USE_SBR
  if (!strcmp("Starbase",temp)) return vtkStarbaseRenderer::New();
#endif
#ifdef VTK_USE_GLR
  if (!strcmp("GL",temp)) return vtkGLRenderer::New();
#endif
#ifdef VTK_USE_OGLR
  if (!strcmp("OpenGL",temp)) return vtkOpenGLRenderer::New();
#endif
#ifdef _WIN32
  if (!strcmp("Win32OpenGL",temp)) return vtkOpenGLRenderer::New();
#endif
#ifdef VTK_USE_XGLR
  if (!strcmp("XGL",temp)) return vtkXGLRenderer::New();
#endif
  
  return new vtkRenderer;
}

// Description:
// Concrete render method.
void vtkRenderer::Render(void)
{
  if (this->StartRenderMethod) 
    {
    (*this->StartRenderMethod)(this->StartRenderMethodArg);
    }

  // if backing store is on and we have a stored image
  if (this->BackingStore && this->BackingImage &&
      this->MTime < this->RenderTime &&
      this->ActiveCamera->GetMTime() < this->RenderTime &&
      this->RenderWindow->GetMTime() < this->RenderTime)
    {
    int mods = 0;
    vtkLight *light;
    vtkActor *anActor;
    
    // now we just need to check the lights and actors
    for(this->Lights.InitTraversal(); 
	(light = this->Lights.GetNextItem()); )
      {
      if (light->GetSwitch() && 
	  light->GetMTime() > this->RenderTime) mods = 1;
      }
    for (this->Actors.InitTraversal(); 
	 (anActor = this->Actors.GetNextItem()); )
      {
      // if it's invisible, we can skip the rest 
      if (anActor->GetVisibility())
	{
	if (anActor->GetMTime() > this->RenderTime) mods = 1;
	if (anActor->GetProperty()->GetMTime() > this->RenderTime) mods = 1;
	if (anActor->GetTexture() && 
	    anActor->GetTexture()->GetMTime() > this->RenderTime) mods = 1;
	if (anActor->GetMapper()->GetMTime() > this->RenderTime) mods = 1;
	anActor->GetMapper()->GetInput()->Update();
	if (anActor->GetMapper()->GetInput()->GetMTime() > this->RenderTime) mods = 1;
	}
      }
    
    if (!mods)
      {
      int x1, y1, x2, y2;
      
      // backing store should be OK, lets use it
      // calc the pixel range for the renderer
      x1 = (int)(this->Viewport[0]*(this->RenderWindow->GetSize()[0] - 1));
      y1 = (int)(this->Viewport[1]*(this->RenderWindow->GetSize()[1] - 1));
      x2 = (int)(this->Viewport[2]*(this->RenderWindow->GetSize()[0] - 1));
      y2 = (int)(this->Viewport[3]*(this->RenderWindow->GetSize()[1] - 1));
      this->RenderWindow->SetPixelData(x1,y1,x2,y2,this->BackingImage,0);
      return;
      }
    }
  
  // do the render library specific stuff
  this->DeviceRender();

  if (this->BackingStore)
    {
    if (this->BackingImage) delete [] this->BackingImage;
    
    int x1, y1, x2, y2;
    
    // backing store should be OK, lets use it
    // calc the pixel range for the renderer
    x1 = (int)(this->Viewport[0]*(this->RenderWindow->GetSize()[0] - 1));
    y1 = (int)(this->Viewport[1]*(this->RenderWindow->GetSize()[1] - 1));
    x2 = (int)(this->Viewport[2]*(this->RenderWindow->GetSize()[0] - 1));
    y2 = (int)(this->Viewport[3]*(this->RenderWindow->GetSize()[1] - 1));
    this->BackingImage = this->RenderWindow->GetPixelData(x1,y1,x2,y2,0);
    }
    
  if (this->EndRenderMethod) 
    {
    (*this->EndRenderMethod)(this->EndRenderMethodArg);
    }
  this->RenderTime.Modified();
}

void vtkRenderer::Render2D()
{
  this->Actors2D->Render(this);
}

vtkWindow *vtkRenderer::GetVTKWindow()
{
  return this->RenderWindow;
}

// Description:
// Specify the camera to use for this renderer.
void vtkRenderer::SetActiveCamera(vtkCamera *cam)
{
  if (this->ActiveCamera == cam)
    {
    return;
    }

  if (this->ActiveCamera)
    {
    this->ActiveCamera->UnRegister(this);
    this->ActiveCamera = NULL;
    }
  if (cam)
    {
    cam->Register(this);
    }

  this->ActiveCamera = cam;
  this->Modified();
}

// Description:
// Get the current camera.
vtkCamera *vtkRenderer::GetActiveCamera()
{
  if ( this->ActiveCamera == NULL )
    {
    this->ActiveCamera = vtkCamera::New();
    this->ResetCamera();
    }

  return this->ActiveCamera;
}

// Description:
// Add a light to the list of lights.
void vtkRenderer::AddLight(vtkLight *light)
{
  this->Lights.AddItem(light);
}

// Description:
// Add an actor to the list of actors.
void vtkRenderer::AddActor(vtkActor *actor)
{
  this->Actors.AddItem(actor);
}

// Description:
// Add a volume to the list of volumes.
void vtkRenderer::AddVolume(vtkVolume *volume)
{
  this->Volumes.AddItem(volume);
}

// Description:
// Remove a light from the list of lights.
void vtkRenderer::RemoveLight(vtkLight *light)
{
  this->Lights.RemoveItem(light);
}

// Description:
// Remove an actor from the list of actors.
void vtkRenderer::RemoveActor(vtkActor *actor)
{
  this->Actors.RemoveItem(actor);
}

// Description:
// Remove a volume from the list of volumes.
void vtkRenderer::RemoveVolume(vtkVolume *volume)
{
  this->Volumes.RemoveItem(volume);
}

void vtkRenderer::CreateLight(void)
{
  if (this->CreatedLight)
    {
    this->CreatedLight->UnRegister(this);
    this->CreatedLight = NULL;
    }

  this->CreatedLight = vtkLight::New();
  this->AddLight(this->CreatedLight);
  this->CreatedLight->SetPosition(this->ActiveCamera->GetPosition());
  this->CreatedLight->SetFocalPoint(this->ActiveCamera->GetFocalPoint());
}

// Description:
// Automatically set up the camera based on the visible actors.
// The camera will reposition itself to view the center point of the actors,
// and move along its initial view plane normal (i.e., vector defined from 
// camera position to focal point) so that all of the actors can be seen.
void vtkRenderer::ResetCamera()
{
  vtkActor   *anActor;
  vtkVolume  *aVolume;
  float      *bounds;
  float      allBounds[6];
  int        nothingVisible=1;

  allBounds[0] = allBounds[2] = allBounds[4] = VTK_LARGE_FLOAT;
  allBounds[1] = allBounds[3] = allBounds[5] = -VTK_LARGE_FLOAT;
  
  // loop through actors (and their parts)
  for ( this->Actors.InitTraversal(); (anActor = this->Actors.GetNextItem()); )
    {
    // if it's invisible, or has no geometry, we can skip the rest 
    if ( anActor->GetVisibility() )
      {
      bounds = anActor->GetBounds();
      // make sure we haven't got bogus bounds
      if ( bounds[0] > -VTK_LARGE_FLOAT && bounds[1] < VTK_LARGE_FLOAT &&
           bounds[2] > -VTK_LARGE_FLOAT && bounds[3] < VTK_LARGE_FLOAT &&
           bounds[4] > -VTK_LARGE_FLOAT && bounds[5] < VTK_LARGE_FLOAT )
	{
        nothingVisible = 0;

	if (bounds[0] < allBounds[0]) allBounds[0] = bounds[0]; 
	if (bounds[1] > allBounds[1]) allBounds[1] = bounds[1]; 
	if (bounds[2] < allBounds[2]) allBounds[2] = bounds[2]; 
	if (bounds[3] > allBounds[3]) allBounds[3] = bounds[3]; 
	if (bounds[4] < allBounds[4]) allBounds[4] = bounds[4]; 
	if (bounds[5] > allBounds[5]) allBounds[5] = bounds[5]; 
	}
      }
    }

  // loop through volumes
  for ( this->Volumes.InitTraversal(); 
	(aVolume = this->Volumes.GetNextItem()); )
    {
    // if it's invisible we can skip the rest 
    if ( aVolume->GetVisibility() )
      {
      nothingVisible = 0;
      bounds = aVolume->GetBounds();

      if (bounds[0] < allBounds[0]) allBounds[0] = bounds[0]; 
      if (bounds[1] > allBounds[1]) allBounds[1] = bounds[1]; 
      if (bounds[2] < allBounds[2]) allBounds[2] = bounds[2]; 
      if (bounds[3] > allBounds[3]) allBounds[3] = bounds[3]; 
      if (bounds[4] < allBounds[4]) allBounds[4] = bounds[4]; 
      if (bounds[5] > allBounds[5]) allBounds[5] = bounds[5]; 
      }
    }


  if ( nothingVisible )
    {
    vtkErrorMacro(<< "Can't reset camera if no actors or volumes are visible");
    return;
    }

  this->ResetCamera(allBounds);
}

// Description:
// Automatically set up the camera based on a specified bounding box
// (xmin,xmax, ymin,ymax, zmin,zmax). Camera will reposition itself so
// that its focal point is the center of the bounding box, and adjust its
// distance and position to preserve its initial view plane normal 
// (i.e., vector defined from camera position to focal point). Note: is 
// the view plane is parallel to the view up axis, the view up axis will
// be reset to one of the three coordinate axes.
void vtkRenderer::ResetCamera(float bounds[6])
{
  float center[3];
  float distance;
  float width;
  float vn[3], *vup;;

  if ( this->ActiveCamera != NULL )
    {
    this->ActiveCamera->GetViewPlaneNormal(vn);
    }
  else
    {
    vtkErrorMacro(<< "Trying to reset non-existant camera");
    return;
    }

  center[0] = (bounds[0] + bounds[1])/2.0;
  center[1] = (bounds[2] + bounds[3])/2.0;
  center[2] = (bounds[4] + bounds[5])/2.0;

  width = bounds[3] - bounds[2];
  if (width < (bounds[1] - bounds[0]))
    {
    width = bounds[1] - bounds[0];
    }
  distance = 
    0.8*width/tan(this->ActiveCamera->GetViewAngle()*vtkMath::Pi()/360.0);
  distance = distance + (bounds[5] - bounds[4])/2.0;

  // check view-up vector against view plane normal
  vup = this->ActiveCamera->GetViewUp();
  if ( fabs(vtkMath::Dot(vup,vn)) > 0.999 )
    {
    vtkWarningMacro(<<"Resetting view-up since view plane normal is parallel");
    this->ActiveCamera->SetViewUp(-vup[2], vup[0], vup[1]);
    }

  // update the camera
  this->ActiveCamera->SetFocalPoint(center);
  this->ActiveCamera->SetPosition(center[0]+distance*vn[0],
				  center[1]+distance*vn[1],
				  center[2]+distance*vn[2]);
  this->ActiveCamera->SetClippingRange(distance/10.0,distance*5.0);
}
  
// Description:
// Alternative version of ResetCamera(bounds[6]);
void vtkRenderer::ResetCamera(float xmin, float xmax, float ymin, float ymax, 
			      float zmin, float zmax)
{
  float bounds[6];

  bounds[0] = xmin;
  bounds[1] = xmax;
  bounds[2] = ymin;
  bounds[3] = ymax;
  bounds[4] = zmin;
  bounds[5] = zmax;

  this->ResetCamera(bounds);
}

// Description:
// Specify the rendering window in which to draw. This is automatically set
// when the renderer is created by MakeRenderer.  The user probably
// shouldn't ever need to call this method.
void vtkRenderer::SetRenderWindow(vtkRenderWindow *renwin)
{
  this->VTKWindow = renwin;
  this->RenderWindow = renwin;
}

// Description:
// Given a pixel location, return the Z value
float vtkRenderer::GetZ (int x, int y)
{
  float *zPtr;
  float z;

  zPtr = this->RenderWindow->GetZbufferData (x, y, x, y);
  if (zPtr)
    {
    z = *zPtr;
    delete zPtr;
    }
  else
    {
    z = 1.0;
    }
  return z;
}


// Description:
// Convert view point coordinates to world coordinates.
void vtkRenderer::ViewToWorld()
{
  vtkMatrix4x4 mat;
  float result[4];

  // get the perspective transformation from the active camera 
  mat = this->ActiveCamera->GetCompositePerspectiveTransform(1,0,1);
  
  // use the inverse matrix 
  mat.Invert();
 
  // Transform point to world coordinates 
  result[0] = this->ViewPoint[0];
  result[1] = this->ViewPoint[1];
  result[2] = this->ViewPoint[2];
  result[3] = 1.0;

  mat.Transpose();
  mat.PointMultiply(result,result);
  
  // Get the transformed vector & set WorldPoint 
  // while we are at it try to keep w at one
  if (result[3])
    {
    result[0] /= result[3];
    result[1] /= result[3];
    result[2] /= result[3];
    result[3] = 1;
    }
  
  this->SetWorldPoint(result);
}

void vtkRenderer::ViewToWorld(float &x, float &y, float &z)
{
  vtkMatrix4x4 mat;
  float result[4];

  // get the perspective transformation from the active camera 
  mat = this->ActiveCamera->GetCompositePerspectiveTransform(1,0,1);
  
  // use the inverse matrix 
  mat.Invert();
 
  // Transform point to world coordinates 
  result[0] = x;
  result[1] = y;
  result[2] = z;
  result[3] = 1.0;

  mat.Transpose();
  mat.PointMultiply(result,result);
  
  // Get the transformed vector & set WorldPoint 
  // while we are at it try to keep w at one
  if (result[3])
    {
    x = result[0] / result[3];
    y = result[1] / result[3];
    z = result[2] / result[3];
    }
}

// Description:
// Convert world point coordinates to view coordinates.
void vtkRenderer::WorldToView()
{
  vtkMatrix4x4 matrix;
  float     view[4];
  float     *world;

  // get the perspective transformation from the active camera 
  matrix = this->ActiveCamera->GetCompositePerspectiveTransform(1,0,1);

  world = this->WorldPoint;
  view[0] = world[0]*matrix[0][0] + world[1]*matrix[0][1] +
    world[2]*matrix[0][2] + world[3]*matrix[0][3];
  view[1] = world[0]*matrix[1][0] + world[1]*matrix[1][1] +
    world[2]*matrix[1][2] + world[3]*matrix[1][3];
  view[2] = world[0]*matrix[2][0] + world[1]*matrix[2][1] +
    world[2]*matrix[2][2] + world[3]*matrix[2][3];
  view[3] = world[0]*matrix[3][0] + world[1]*matrix[3][1] +
    world[2]*matrix[3][2] + world[3]*matrix[3][3];

  if (view[3] != 0.0)
    {
    this->SetViewPoint(view[0]/view[3],
		       view[1]/view[3],
		       view[2]/view[3]);
    }
}

// Description:
// Convert world point coordinates to view coordinates.
void vtkRenderer::WorldToView(float &x, float &y, float &z)
{
  vtkMatrix4x4 matrix;
  float     view[4];

  // get the perspective transformation from the active camera 
  matrix = this->ActiveCamera->GetCompositePerspectiveTransform(1,0,1);

  view[0] = x*matrix[0][0] + y*matrix[0][1] +
    z*matrix[0][2] + matrix[0][3];
  view[1] = x*matrix[1][0] + y*matrix[1][1] +
    z*matrix[1][2] + matrix[1][3];
  view[2] = x*matrix[2][0] + y*matrix[2][1] +
    z*matrix[2][2] + matrix[2][3];
  view[3] = x*matrix[3][0] + y*matrix[3][1] +
    z*matrix[3][2] + matrix[3][3];

  if (view[3] != 0.0)
    {
    x = view[0]/view[3];
    y = view[1]/view[3];
    z = view[2]/view[3];
    }
}

void vtkRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkViewport::PrintSelf(os,indent);

  os << indent << "Actors:\n";
  this->Actors.PrintSelf(os,indent.GetNextIndent());
  os << indent << "Ambient: (" << this->Ambient[0] << ", " 
    << this->Ambient[1] << ", " << this->Ambient[2] << ")\n";

  os << indent << "BackingStore: " << (this->BackingStore ? "On\n":"Off\n");
  os << indent << "DisplayPoint: ("  << this->DisplayPoint[0] << ", " 
    << this->DisplayPoint[1] << ", " << this->DisplayPoint[2] << ")\n";
  os << indent << "Lights:\n";
  this->Lights.PrintSelf(os,indent.GetNextIndent());

  os << indent << "ViewPoint: (" << this->ViewPoint[0] << ", " 
    << this->ViewPoint[1] << ", " << this->ViewPoint[2] << ")\n";

  os << indent << "Two-sided Lighting: " 
     << (this->TwoSidedLighting ? "On\n" : "Off\n");

  if ( this->RayCaster )
    {
    os << indent << "Ray Caster: " << this->RayCaster << "\n";
    }
  else
    {
    os << indent << "Ray Caster: (none)\n";
    }

  os << indent << "Allocated Render Time: " << this->AllocatedRenderTime
     << "\n";
}

int vtkRenderer::VisibleActorCount()
{
  vtkActor *anActor;
  int count = 0;

  // loop through actors
  for (this->Actors.InitTraversal(); (anActor = this->Actors.GetNextItem()); )
    if (anActor->GetVisibility())
      count++;

  return count;
}

int vtkRenderer::VisibleVolumeCount()
{
  int        count = 0;
  vtkVolume  *aVolume;

  // loop through volumes
  for (this->Volumes.InitTraversal(); (aVolume = this->Volumes.GetNextItem()); )
    if (aVolume->GetVisibility())
      count++;

  return count;

}




unsigned long int vtkRenderer::GetMTime()
{
  unsigned long mTime=this-> vtkViewport::GetMTime();
  unsigned long time;

  if ( this-> RayCaster != NULL )
    {
    time = this->RayCaster ->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
if ( this->ActiveCamera != NULL )
    {
    time = this->ActiveCamera ->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
if ( this->CreatedLight != NULL )
    {
    time = this->CreatedLight ->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

