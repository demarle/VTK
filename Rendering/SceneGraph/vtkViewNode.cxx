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
  this->Parent = NULL;
  this->Children = vtkViewNodeCollection::New();
  this->MyFactory = NULL;
}

//----------------------------------------------------------------------------
vtkViewNode::~vtkViewNode()
{
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
  this->UpdateChildren();
  //?
}

//----------------------------------------------------------------------------
void vtkViewNode::UpdateChildren()
{
  vtkCollectionIterator *it = this->Children->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    //vtkViewNode *child = vtkViewNode::SafeDownCast(it->GetCurrentObject());
    //child->   ? ();
    it->GoToNextItem();
    }
}

//----------------------------------------------------------------------------
void vtkViewNode::TraverseChildren()
{
  vtkCollectionIterator *it = this->Children->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkViewNode *child = vtkViewNode::SafeDownCast(it->GetCurrentObject());
    child->Traverse();
    it->GoToNextItem();
    }
}

//----------------------------------------------------------------------------
vtkViewNode *vtkViewNode::CreateViewNode(vtkObject *obj)
{
  vtkViewNode *ret = NULL;
  if (!this->MyFactory)
    {
    vtkWarningMacro("Can not create view nodes without my own factory");
    return NULL;
    }
  else
    {
    ret = this->MyFactory->CreateNode(obj);
    }
  return ret;
}
