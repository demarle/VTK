/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkViewNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkViewNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkViewNodeCollection.h"
#include "vtkViewNodeFactory.h"

//============================================================================
vtkStandardNewMacro(vtkViewNode);

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkViewNode,Parent,vtkViewNode);

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkViewNode,Children,vtkViewNodeCollection);

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkViewNode,MyFactory,vtkViewNodeFactory);

//----------------------------------------------------------------------------
vtkViewNode::vtkViewNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Renderable = NULL;
  this->Parent = NULL;
  this->Children = vtkViewNodeCollection::New();
  this->MyFactory = NULL;
}

//----------------------------------------------------------------------------
vtkViewNode::~vtkViewNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  if (this->Parent)
    {
    this->Parent->Delete();
    }
  if (this->Children)
    {
    this->Children->Delete();
    }
  if (this->MyFactory)
    {
    this->MyFactory->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkViewNode::Traverse()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<  endl;
  this->UpdateChildren();
  this->TraverseChildren();
}

//----------------------------------------------------------------------------
void vtkViewNode::Update()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  cerr << "update " << this->GetRenderable()->GetClassName()
       << "[" << this->GetRenderable() << "]" << endl;
}

//----------------------------------------------------------------------------
void vtkViewNode::UpdateChildren()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkCollectionIterator *it = this->Children->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkViewNode *child = vtkViewNode::SafeDownCast(it->GetCurrentObject());
    child->Update();
    it->GoToNextItem();
    }
  it->Delete();
}

//----------------------------------------------------------------------------
void vtkViewNode::TraverseChildren()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkCollectionIterator *it = this->Children->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkViewNode *child = vtkViewNode::SafeDownCast(it->GetCurrentObject());
    child->Traverse();
    it->GoToNextItem();
    }
  it->Delete();
}

//----------------------------------------------------------------------------
vtkViewNode *vtkViewNode::CreateViewNode(vtkObject *obj)
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkViewNode *ret = NULL;
  if (!this->MyFactory)
    {
    vtkWarningMacro("Can not create view nodes without my own factory");
    }
  else
    {
    ret = this->MyFactory->CreateNode(obj);
    ret->Renderable = obj;
    }
  return ret;
}

//----------------------------------------------------------------------------
void vtkViewNode::SetRenderable(vtkObject *obj)
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Renderable = obj;
}
