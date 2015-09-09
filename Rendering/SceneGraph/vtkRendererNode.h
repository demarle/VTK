/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRendererNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRendererNode - vtkViewNode specialized for vtkRenderers
// .SECTION Description
//

#ifndef vtkRendererNode_h
#define vtkRendererNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkViewNode.h"

class VTKRENDERINGSCENEGRAPH_EXPORT vtkRendererNode :
  public vtkViewNode
{
public:
  static vtkRendererNode* New();
  vtkTypeMacro(vtkRendererNode, vtkViewNode);
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
  vtkRendererNode();
  ~vtkRendererNode();

private:
  vtkRendererNode(const vtkRendererNode&); // Not implemented.
  void operator=(const vtkRendererNode&); // Not implemented.
};

#endif
