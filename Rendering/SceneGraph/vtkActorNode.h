/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkActorNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkActorNode - vtkViewNode specialized for vtkActors
// .SECTION Description
//

#ifndef vtkActorNode_h
#define vtkActorNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkViewNode.h"

class VTKRENDERINGSCENEGRAPH_EXPORT vtkActorNode :
  public vtkViewNode
{
public:
  static vtkActorNode* New();
  vtkTypeMacro(vtkActorNode, vtkViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //...
  virtual void BuildSelf() {};

  //Description:
  //Get state from our renderable.
  virtual void SynchronizeSelf();

  //Description:
  //...
  virtual void RenderSelf() {};

protected:
  vtkActorNode();
  ~vtkActorNode();

private:
  vtkActorNode(const vtkActorNode&); // Not implemented.
  void operator=(const vtkActorNode&); // Not implemented.
};

#endif
