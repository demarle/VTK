/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWindowNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkWindowNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkViewNodeCollection.h"

//============================================================================
vtkStandardNewMacro(vtkWindowNode);

//----------------------------------------------------------------------------
vtkWindowNode::vtkWindowNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
}

//----------------------------------------------------------------------------
vtkWindowNode::~vtkWindowNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
}

//----------------------------------------------------------------------------
void vtkWindowNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkWindowNode::Traverse()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Update();
  this->Superclass::Traverse();
}

//----------------------------------------------------------------------------
void vtkWindowNode::UpdateChildren()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderWindow *mine = vtkRenderWindow::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //create/delete children as required
  //to make sure my children are consistent with my renderables
  vtkViewNodeCollection *nodes = this->GetChildren();
  vtkRendererCollection *rens = mine->GetRenderers();

  //remove viewnodes if their renderables are no longer present
  vtkCollectionIterator *nit = nodes->NewIterator();
  nit->InitTraversal();
  while (!nit->IsDoneWithTraversal())
    {
    vtkViewNode *node = vtkViewNode::SafeDownCast(nit->GetCurrentObject());
    vtkObject *obj = node->GetRenderable();
    if (!rens->IsItemPresent(obj))
      {
      nodes->RemoveItem(node);
      }
    nit->GoToNextItem();
    }
  nit->Delete();

  //add viewnodes for renderables that are not yet present
  vtkCollectionIterator *rit = rens->NewIterator();
  rit->InitTraversal();
  while (!rit->IsDoneWithTraversal())
    {
    vtkRenderer *obj = vtkRenderer::SafeDownCast(rit->GetCurrentObject());
    if (!nodes->IsRenderablePresent(obj))
      {
      vtkViewNode *node = this->CreateViewNode(obj);
      nodes->AddItem(node);
      node->Delete();
      }
    rit->GoToNextItem();
    }
  rit->Delete();

  //call parent class to recurse
  this->Superclass::UpdateChildren();
}

//----------------------------------------------------------------------------
void vtkWindowNode::Update()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderWindow *mine = vtkRenderWindow::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //TODO: get state from our renderable
}
