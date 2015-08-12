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
#include "vtkViewNodeCollection.h"

//============================================================================
vtkStandardNewMacro(vtkWindowViewNode);

//----------------------------------------------------------------------------
vtkWindowViewNode::vtkWindowViewNode()
{
}

//----------------------------------------------------------------------------
vtkWindowViewNode::~vtkWindowViewNode()
{
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::Traverse(vtkRenderWindow *win)
{
  this->UpdateChildren();
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::UpdateChildren()
{
  /*
    this->RendWin->GetRenderers()
    for each Renderer make sure there is a node for it, otherwise create one with CreateViewNode
  */

  vtkCollectionIterator *it = this->Children->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    //vtkViewNode *child = vtkViewNode::SafeDownCast(it->GetCurrentObject());
    //child->   ? ();
    it->GoToNextItem();
    }
}
