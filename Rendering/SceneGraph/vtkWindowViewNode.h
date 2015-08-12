/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXXX.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkWindowViewNode -
// .SECTION Description
//

#ifndef vtkWindowViewNode_h
#define vtkWindowViewNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkViewNode.h"


class VTKRENDERINGSCENEGRAPH_EXPORT vtkWindowViewNode :
  public vtkViewNode
{
public:
  static vtkWindowViewNode* New();
  vtkTypeMacro(vtkWindowViewNode, vtkViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkWindowViewNode();
  ~vtkWindowViewNode();

private:
  vtkWindowViewNode(const vtkWindowViewNode&); // Not implemented.
  void operator=(const vtkWindowViewNode&); // Not implemented.

  class vtkInternals;
  vtkInternals *Internals;
};

#endif
