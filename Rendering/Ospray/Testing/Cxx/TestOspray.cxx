/*=========================================================================

  Program:   Visualization Toolkit
  Module:    Mace.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOsprayViewNodeFactory.h"
#include "vtkOsprayWindowNode.h"

int TestOspray( int argc, char *argv[] )
{
  vtkOsprayWindowNode *owvn = vtkOsprayWindowNode::New();
  cerr << "owvn [" << owvn << "]" << endl;
  owvn->PrintSelf(cerr, vtkIndent(0));
  owvn->Delete();
  return 0;
}
