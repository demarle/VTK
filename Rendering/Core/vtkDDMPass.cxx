/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDDMPass.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkDDMPass.h"
#include "vtkObjectFactory.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"

vtkStandardNewMacro(vtkDDMPass);

// ----------------------------------------------------------------------------
vtkDDMPass::vtkDDMPass()
{
  cerr << "HELLO" << endl;
}

// ----------------------------------------------------------------------------
vtkDDMPass::~vtkDDMPass()
{
  cerr << "GOODBYE" << endl;
}

// ----------------------------------------------------------------------------
void vtkDDMPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// ----------------------------------------------------------------------------
void vtkDDMPass::Render(const vtkRenderState *s)
{
  cerr << "WORLD" << endl;
  (void)s;
  this->NumberOfRenderedProps=0;
}
