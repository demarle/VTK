/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayWindowViewNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOsprayWindowViewNode -
// .SECTION Description
//

#ifndef vtkOsprayWindowViewNode_h
#define vtkOsprayWindowViewNode_h

#include "vtkRenderingOsprayModule.h" // For export macro
#include "vtkWindowViewNode.h"

class vtkRenderWindow;

class VTKRENDERINGOSPRAY_EXPORT vtkOsprayWindowViewNode :
  public vtkWindowViewNode
{
public:
  static vtkOsprayWindowViewNode* New();
  vtkTypeMacro(vtkOsprayWindowViewNode, vtkWindowViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkOsprayWindowViewNode();
  ~vtkOsprayWindowViewNode();

private:
  vtkOsprayWindowViewNode(const vtkOsprayWindowViewNode&); // Not implemented.
  void operator=(const vtkOsprayWindowViewNode&); // Not implemented.
};

#endif
