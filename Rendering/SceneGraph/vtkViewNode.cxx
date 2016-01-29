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

//----------------------------------------------------------------------------
const char *vtkViewNode::operation_type_strings[] =
{
  "noop",
  "build",
  "synchronize",
  "render",
  NULL
};

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
void vtkViewNode::Traverse(int operation)
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Apply(operation);

  vtkCollectionIterator *it = this->Children->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkViewNode *child = vtkViewNode::SafeDownCast(it->GetCurrentObject());
    child->Traverse(operation);
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
    if (ret)
      {
      ret->Renderable = obj;
      }
    }
  return ret;
}

//----------------------------------------------------------------------------
void vtkViewNode::SetRenderable(vtkObject *obj)
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Renderable = obj;
}

//----------------------------------------------------------------------------
void vtkViewNode::Build()
{
  //cerr << this->GetClassName() << "(" << this << ") Build()" << endl;
  this->Traverse(build);
}

//----------------------------------------------------------------------------
void vtkViewNode::Synchronize()
{
  //cerr << this->GetClassName() << "(" << this << ") Synchronize()" << endl;
  this->Traverse(synchronize);
}

//----------------------------------------------------------------------------
void vtkViewNode::Render()
{
  //cerr << this->GetClassName() << "(" << this << ") Render()" << endl;
  this->Traverse(render);
}

//----------------------------------------------------------------------------
void vtkViewNode::Apply(int operation)
{
  //cerr << this->GetClassName() << "(" << this << ") Apply("
  //     << vtkViewNode::operation_type_strings[operation] << ")" << endl;
  switch (operation)
    {
    case noop:
      break;
    case build:
      this->BuildSelf();
      break;
    case synchronize:
      this->SynchronizeSelf();
      break;
    case render:
      this->RenderSelf();
      break;
    default:
      cerr << "UNKNOWN OPERATION" << operation << endl;
    }
}
