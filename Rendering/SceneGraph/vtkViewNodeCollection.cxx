/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkViewNodeCollection.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkViewNodeCollection.h"

#include "vtkViewNode.h"
#include "vtkObjectFactory.h"

//============================================================================
vtkStandardNewMacro(vtkViewNodeCollection);

//----------------------------------------------------------------------------
void vtkViewNodeCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkViewNodeCollection::AddItem(vtkViewNode *a)
{
  this->vtkCollection::AddItem(a);
}

//----------------------------------------------------------------------------
vtkViewNode *vtkViewNodeCollection::GetNextItem()
{
  return static_cast<vtkViewNode *>(this->GetNextItemAsObject());
}

//----------------------------------------------------------------------------
vtkViewNode *vtkViewNodeCollection::GetNextViewNode(
  vtkCollectionSimpleIterator &cookie)
{
  return static_cast<vtkViewNode *>(this->GetNextItemAsObject(cookie));
}
