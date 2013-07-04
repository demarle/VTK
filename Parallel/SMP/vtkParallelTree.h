/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelTree.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParallelTree - !!!!
// .SECTION Description
// !!!!

#ifndef __vtkParallelTree_h__
#define __vtkParallelTree_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"

class vtkFunctor;

//======================================================================================
class VTKPARALLELSMP_EXPORT vtkParallelTree
{
public:
  virtual int TraverseNode( vtkIdType id, int lvl, vtkFunctor* function ) const = 0;
  virtual void GetTreeSize ( int& max_level, vtkIdType& branching_factor ) const = 0;
};

#endif
// VTK-HeaderTest-Exclude: vtkParallelTree.h
