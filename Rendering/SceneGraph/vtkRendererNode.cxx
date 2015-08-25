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
void vtkRendererNode::Traverse()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Update();
  this->Superclass::Traverse();
}

//----------------------------------------------------------------------------
void vtkRendererNode::UpdateChildren()
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

  //call parent class to recurse
  this->Superclass::UpdateChildren();
}

//----------------------------------------------------------------------------
void vtkRendererNode::Update()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderer *mine = vtkRenderer::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //TODO: get state from our renderable
}
