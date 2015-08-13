/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWindowViewNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkWindowViewNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkViewNodeCollection.h"

//============================================================================
vtkStandardNewMacro(vtkWindowViewNode);

//----------------------------------------------------------------------------
vtkWindowViewNode::vtkWindowViewNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
}

//----------------------------------------------------------------------------
vtkWindowViewNode::~vtkWindowViewNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::Traverse()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Update();
  this->Superclass::Traverse();
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::UpdateChildren()
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

  //remove viewnodes if their renderables that are no longer present
  vtkCollectionIterator *nit = nodes->NewIterator();
  nit->InitTraversal();
  while (!nit->IsDoneWithTraversal())
    {
    vtkViewNode *node = vtkViewNode::SafeDownCast(nit->GetCurrentObject());
    vtkObject *obj = node->GetRenderable();
    if (!rens->IsItemPresent(obj))
      {
      cerr << "DELETED VN " << node << " for " << obj << endl;
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
    vtkRenderer *ren = vtkRenderer::SafeDownCast(rit->GetCurrentObject());
    vtkViewNode *node = this->CreateViewNode(ren);
    cerr << "CREATED VN " << node << " for " << ren << endl;
    nodes->AddItem(node);
    node->Delete();
    rit->GoToNextItem();
    }
  rit->Delete();

  //call parent class to recurse
  this->Superclass::UpdateChildren();
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::Update()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderWindow *mine = vtkRenderWindow::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    cerr << "I GOT NOTHING" << endl;
    return;
    }

  cerr << "update " << mine->GetClassName() << "[" << mine << "]" << endl;
  //mine->PrintSelf(cerr, vtkIndent(0));

  //get size, position, title, any other state that would be handy from
  //out renderable
}
