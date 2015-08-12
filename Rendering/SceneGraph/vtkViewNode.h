/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkViewNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkViewNode -
// .SECTION Description
//

#ifndef vtkViewNode_h
#define vtkViewNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkObject.h"


class VTKRENDERINGSCENEGRAPH_EXPORT vtkViewNode :
  public vtkObject
{
public:
  static vtkViewNode* New();
  vtkTypeMacro(vtkViewNode, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkViewNode();
  ~vtkViewNode();

private:
  vtkViewNode(const vtkViewNode&); // Not implemented.
  void operator=(const vtkViewNode&); // Not implemented.

  class vtkInternals;
  vtkInternals *Internals;
};

#endif
