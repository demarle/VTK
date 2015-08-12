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
#include "vtkObjectFactory.h"


class vtkViewNode::vtkInternals
{
public:

  vtkInternals()
    {
    }

  ~vtkInternals()
    {
    }
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkViewNode);

//----------------------------------------------------------------------------
vtkViewNode::vtkViewNode()
{
  this->Internals = new vtkInternals;
}

//----------------------------------------------------------------------------
vtkViewNode::~vtkViewNode()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
