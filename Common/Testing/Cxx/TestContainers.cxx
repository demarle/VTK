/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestContainers.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkObject.h"
#include "vtkAbstractList.h"

int main(int argc, char** argv)
{
  // Let us for now just create pointers so that we see that
  // it can parse through the header file.
  vtkContainer *cnt = 0;
  vtkAbstractList<int> *alist = 0;

  // This is here so that it does not complain about 
  // pointers not being used
  return 
    reinterpret_cast<int>( cnt ) + 
    reinterpret_cast<int>( alist );
}
