/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkActorNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkActorNode.h"

#include "vtkActor.h"
#include "vtkObjectFactory.h"

//============================================================================
vtkStandardNewMacro(vtkActorNode);

//----------------------------------------------------------------------------
vtkActorNode::vtkActorNode()
{
}

//----------------------------------------------------------------------------
vtkActorNode::~vtkActorNode()
{
}

//----------------------------------------------------------------------------
void vtkActorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkActorNode::SynchronizeSelf()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkActor *mine = vtkActor::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //TODO: get state from our renderable
}
