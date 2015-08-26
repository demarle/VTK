/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayActorNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayActorNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkActor.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayActorNode);

//----------------------------------------------------------------------------
vtkOsprayActorNode::vtkOsprayActorNode()
{
}

//----------------------------------------------------------------------------
vtkOsprayActorNode::~vtkOsprayActorNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayActorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayActorNode::Update()
{
  cerr << "Hello from " << this << " " << this->GetClassName() << endl;
}
