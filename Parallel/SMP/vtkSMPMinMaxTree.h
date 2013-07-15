/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPMinMaxTree.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPMinMaxTree - A multi-core accelerated vtkSimpleScalarTree
// .SECTION Description
// !!!!

#ifndef VTKSMPMINMAXTREE_H
#define VTKSMPMINMAXTREE_H

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkParallelTree.h" //for inheritance
#include "vtkSimpleScalarTree.h"

class vtkGenericCell;
class vtkFunctor;
class InitializeFunctor;

//TODO: single inheritance
class VTKPARALLELSMP_EXPORT vtkSMPMinMaxTree : public vtkSimpleScalarTree, public vtkParallelTree
{
public:
  vtkTypeMacro(vtkSMPMinMaxTree, vtkSimpleScalarTree);
  static vtkSMPMinMaxTree* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void BuildTree();
  void InitTraversal(double scalarValue);

  virtual int TraverseNode( vtkIdType id, int lvl, vtkFunctor* function ) const;
  virtual void GetTreeSize ( int& max_level, vtkIdType& branching_factor ) const;

  friend class InitializeFunctor;

protected:
  vtkSMPMinMaxTree();
  ~vtkSMPMinMaxTree();

private:
  vtkSMPMinMaxTree(const vtkSMPMinMaxTree&); // Not implemented
   void operator=(const vtkSMPMinMaxTree&); // Not implemented
};

#endif // VTKSMPMINMAXTREE_H

// VTK-HeaderTest-Exclude: vtkSMPMinMaxTree.h
