/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGenericPointIterator.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkGenericPointIterator.h"

vtkCxxRevisionMacro(vtkGenericPointIterator, "1.2");

//----------------------------------------------------------------------------
vtkGenericPointIterator::vtkGenericPointIterator()
{
}

//----------------------------------------------------------------------------
vtkGenericPointIterator::~vtkGenericPointIterator()
{
}

//----------------------------------------------------------------------------
void vtkGenericPointIterator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
}

