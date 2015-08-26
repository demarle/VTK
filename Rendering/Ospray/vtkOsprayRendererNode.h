/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayRendererNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOsprayRendererNode -
// .SECTION Description
//

#ifndef vtkOsprayRendererNode_h
#define vtkOsprayRendererNode_h

#include "vtkRenderingOsprayModule.h" // For export macro
#include "vtkRendererNode.h"

class VTKRENDERINGOSPRAY_EXPORT vtkOsprayRendererNode :
  public vtkRendererNode
{
public:
  static vtkOsprayRendererNode* New();
  vtkTypeMacro(vtkOsprayRendererNode, vtkRendererNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();

protected:
  vtkOsprayRendererNode();
  ~vtkOsprayRendererNode();

private:
  vtkOsprayRendererNode(const vtkOsprayRendererNode&); // Not implemented.
  void operator=(const vtkOsprayRendererNode&); // Not implemented.
};

#endif
