/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRendererNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkRendererNode.h"

#include "vtkActor.h"
#include "vtkActorNode.h"
#include "vtkCamera.h"
#include "vtkCameraNode.h"
#include "vtkCollectionIterator.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkLightNode.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRendererNode.h"
#include "vtkRenderWindow.h"
#include "vtkViewNodeCollection.h"


//============================================================================
vtkStandardNewMacro(vtkRendererNode);

//----------------------------------------------------------------------------
vtkRendererNode::vtkRendererNode()
{
  this->Ambient[0] = this->Ambient[1] = this->Ambient[2] = 0.0;
  this->Background[0] = this->Background[1] = this->Background[2] = 0.0;
  this->Background2[0] = this->Background2[1] = this->Background2[2] = 0.0;
  this->GradientBackground = false;
  this->Layer = 0;
  this->Origin[0] = this->Origin[1] = 0;
  this->Size[0] = this->Size[1] = 0;
}

//----------------------------------------------------------------------------
vtkRendererNode::~vtkRendererNode()
{
}

//----------------------------------------------------------------------------
void vtkRendererNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkRendererNode::BuildSelf()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderer *mine = vtkRenderer::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //create/delete children as required to make sure my children are
  //consistent with my renderables
  vtkViewNodeCollection *nodes = this->GetChildren();

  vtkActorCollection *actors = mine->GetActors();
  vtkLightCollection *lights = mine->GetLights();
  //vtkVolumeCollection *volumes = mine->GetVolumes();
  vtkCamera *cam = mine->GetActiveCamera();

  //remove viewnodes if their renderables are no longer present
  vtkCollectionIterator *nit = nodes->NewIterator();
  nit->InitTraversal();
  while (!nit->IsDoneWithTraversal())
    {
    vtkViewNode *node = vtkViewNode::SafeDownCast(nit->GetCurrentObject());
    vtkObject *obj = node->GetRenderable();
    if (!lights->IsItemPresent(obj) &&
//        !volumes->IsItemPresent(obj) &&
        !actors->IsItemPresent(obj) &&
        obj != cam)
      {
      nodes->RemoveItem(node);
      }
    nit->GoToNextItem();
    }
  nit->Delete();

  //add viewnodes for renderables that are not yet present
  //lights
  vtkCollectionIterator *rit = lights->NewIterator();
  rit->InitTraversal();
  while (!rit->IsDoneWithTraversal())
    {
    vtkLight *obj = vtkLight::SafeDownCast(rit->GetCurrentObject());
    if (!nodes->IsRenderablePresent(obj))
      {
      vtkViewNode *node = this->CreateViewNode(obj);
      nodes->AddItem(node);
      node->Delete();
      }
    rit->GoToNextItem();
    }
  rit->Delete();

  /*
  //volumes
  rit = volumes->NewIterator();
  rit->InitTraversal();
  while (!rit->IsDoneWithTraversal())
    {
    vtkVolume *obj = vtkVolume::SafeDownCast(rit->GetCurrentObject());
    if (!nodes->IsRenderablePresent(obj))
      {
      vtkViewNode *node = this->CreateViewNode(obj);
      nodes->AddItem(node);
      node->Delete();
      }
    rit->GoToNextItem();
    }
  rit->Delete();
  */

  //actors
  rit = actors->NewIterator();
  rit->InitTraversal();
  while (!rit->IsDoneWithTraversal())
    {
    vtkActor *obj = vtkActor::SafeDownCast(rit->GetCurrentObject());
    if (!nodes->IsRenderablePresent(obj))
      {
      vtkViewNode *node = this->CreateViewNode(obj);
      nodes->AddItem(node);
      node->Delete();
      }
    rit->GoToNextItem();
    }
  rit->Delete();

  //camera
  vtkCamera *obj = mine->GetActiveCamera();
  if (!nodes->IsRenderablePresent(obj))
    {
    vtkViewNode *node = this->CreateViewNode(obj);
    nodes->AddItem(node);
    node->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkRendererNode::SynchronizeSelf()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderer *mine = vtkRenderer::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //TODO: get state from our renderable
/*
  GetAmbient(double data[3])    vtkRenderer     virtual
  GetBackground(double &, double &, double &)   vtkViewport     virtual
  GetBackground2(double &, double &, double &)  vtkViewport     virtual
  GetBackgroundTexture()        vtkRenderer     virtual
  GetCullers()  vtkRenderer     inline
  GetDraw()     vtkRenderer     virtual
  GetErase()    vtkRenderer     virtual
  GetGradientBackground()       vtkViewport     virtual
  GetLayer()    vtkRenderer     virtual
  GetLightFollowCamera()        vtkRenderer     virtual
  GetMTime()    vtkRenderer     virtual
  GetNearClippingPlaneTolerance()       vtkRenderer     virtual
  GetOrigin()     vtkViewport     virtual
  GetPreserveColorBuffer()      vtkRenderer     virtual
  GetPreserveDepthBuffer()      vtkRenderer     virtual
  GetSize()     vtkViewport     virtual
  GetTexturedBackground()       vtkRenderer     virtual
  GetTiledSizeAndOrigin(int *width, int *height, int *lowerLeftX, int *lowerLeftY)
  GetTwoSidedLighting() vtkRenderer     virtual
  GetUseDepthPeeling()  vtkRenderer     virtual
  GetUseShadows()       vtkRenderer     virtual
  GetViewport(double data[4])   vtkViewport     virtual
*/
  mine->GetAmbient(this->Ambient);
  mine->GetBackground(this->Background[0],
                      this->Background[1],
                      this->Background[2]);
  mine->GetBackground2(this->Background2[0],
                       this->Background2[1],
                       this->Background2[2]);
  this->GradientBackground = mine->GetGradientBackground();
  this->Layer = mine->GetLayer();
  int *result;
  result = mine->GetOrigin();
  this->Origin[0] = result[0];
  this->Origin[1] = result[1];
  result = mine->GetSize();
  this->Size[0] = result[0];
  this->Size[1] = result[1];
  //this->TexturedBackground = mine->GetTexturedBackground();
}
