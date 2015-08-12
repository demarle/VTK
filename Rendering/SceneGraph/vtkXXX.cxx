/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXXX.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXXX.h"
#include "vtkObjectFactory.h"


class vtkXXX::vtkInternals
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
vtkStandardNewMacro(vtkXXX);

//----------------------------------------------------------------------------
vtkXXX::vtkXXX()
{
  this->Internals = new vtkInternals;
}

//----------------------------------------------------------------------------
vtkXXX::~vtkXXX()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkXXX::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
