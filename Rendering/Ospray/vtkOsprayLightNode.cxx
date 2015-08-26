/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayLightNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayLightNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkLight.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayLightNode);

//----------------------------------------------------------------------------
vtkOsprayLightNode::vtkOsprayLightNode()
{
}

//----------------------------------------------------------------------------
vtkOsprayLightNode::~vtkOsprayLightNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayLightNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayLightNode::Update()
{
  cerr << "Hello from " << this << " " << this->GetClassName() << endl;
}
