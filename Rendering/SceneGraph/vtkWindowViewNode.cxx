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
#include "vtkObjectFactory.h"


class vtkWindowViewNode::vtkInternals
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
vtkStandardNewMacro(vtkWindowViewNode);

//----------------------------------------------------------------------------
vtkWindowViewNode::vtkWindowViewNode()
{
  this->Internals = new vtkInternals;
}

//----------------------------------------------------------------------------
vtkWindowViewNode::~vtkWindowViewNode()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkWindowViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
