/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayWindowNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOsprayWindowNode -
// .SECTION Description
//

#ifndef vtkOsprayWindowNode_h
#define vtkOsprayWindowNode_h

#include "vtkRenderingOsprayModule.h" // For export macro
#include "vtkWindowNode.h"

class vtkRenderWindow;

class VTKRENDERINGOSPRAY_EXPORT vtkOsprayWindowNode :
  public vtkWindowNode
{
public:
  static vtkOsprayWindowNode* New();
  vtkTypeMacro(vtkOsprayWindowNode, vtkWindowNode);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkOsprayWindowNode();
  ~vtkOsprayWindowNode();

private:
  vtkOsprayWindowNode(const vtkOsprayWindowNode&); // Not implemented.
  void operator=(const vtkOsprayWindowNode&); // Not implemented.
};

#endif
