/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCameraNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCameraNode - vtkViewNode specialized for vtkCameras
// .SECTION Description
//

#ifndef vtkCameraNode_h
#define vtkCameraNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkViewNode.h"

class VTKRENDERINGSCENEGRAPH_EXPORT vtkCameraNode :
  public vtkViewNode
{
public:
  static vtkCameraNode* New();
  vtkTypeMacro(vtkCameraNode, vtkViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  virtual void BuildSelf() {};

  //Description:
  //Get state from our renderable.
  virtual void SynchronizeSelf();

  //Description:
  //...
  virtual void RenderSelf() {};

protected:
  vtkCameraNode();
  ~vtkCameraNode();

  double Position[3];
  double FocalPoint[3];
  double ViewUp[3];
  double ViewAngle;

private:
  vtkCameraNode(const vtkCameraNode&); // Not implemented.
  void operator=(const vtkCameraNode&); // Not implemented.
};

#endif
