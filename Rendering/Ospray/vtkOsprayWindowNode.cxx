/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayWindowNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayWindowNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayWindowNode);

//----------------------------------------------------------------------------
vtkOsprayWindowNode::vtkOsprayWindowNode()
{
  cerr << "HELLO OSPRAY" << endl;
  int ac = 2;
  const char* av[] = {"pvOSPRay\0","--osp:verbose\0"};
  ospInit(&ac, av);
  cerr << "YEAH" << endl;
}

//----------------------------------------------------------------------------
vtkOsprayWindowNode::~vtkOsprayWindowNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayWindowNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
