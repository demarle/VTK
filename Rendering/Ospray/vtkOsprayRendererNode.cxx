/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayRendererNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayRendererNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayRendererNode);

//----------------------------------------------------------------------------
vtkOsprayRendererNode::vtkOsprayRendererNode()
{
}

//----------------------------------------------------------------------------
vtkOsprayRendererNode::~vtkOsprayRendererNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::RenderSelf()
{
  cerr << "Hello from " << this << " " << this->GetClassName() << endl;
}
