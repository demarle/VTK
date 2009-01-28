/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQtView.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkQtView.h"

#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkQtView, "1.2");
vtkStandardNewMacro(vtkQtView);


//----------------------------------------------------------------------------
vtkQtView::vtkQtView()
{
  this->Widget = 0;
}

//----------------------------------------------------------------------------
vtkQtView::~vtkQtView()
{
}

//----------------------------------------------------------------------------
void vtkQtView::SetWidget(QWidget *w)
{
  this->Widget = w;
}

//----------------------------------------------------------------------------
QWidget* vtkQtView::GetWidget()
{
  return this->Widget;
}

//----------------------------------------------------------------------------
void vtkQtView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

