/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWindowNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkWindowNode - vtkViewNode specialized for vtkRenderWindows
// .SECTION Description
//

#ifndef vtkWindowNode_h
#define vtkWindowNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkViewNode.h"

class vtkRenderWindow;

class VTKRENDERINGSCENEGRAPH_EXPORT vtkWindowNode :
  public vtkViewNode
{
public:
  static vtkWindowNode* New();
  vtkTypeMacro(vtkWindowNode, vtkViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Build containers for our child nodes.
  virtual void BuildSelf();

  //Description:
  //Get state of my renderable.
  virtual void SynchronizeSelf();

  //Description:
  //...
  virtual void RenderSelf() {};


protected:
  vtkWindowNode();
  ~vtkWindowNode();

private:
  vtkWindowNode(const vtkWindowNode&); // Not implemented.
  void operator=(const vtkWindowNode&); // Not implemented.
};

#endif
