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

  //Description:
  //Traverse graph in ospray's prefered order and render
  virtual void Render();

  //Description:
  //Make ospray calls to render me.
  virtual void RenderSelf() {};

  //Description:
  //Put my results into the correct place in the provided pixel buffer.
  virtual void WriteLayer(unsigned char *buffer, int buffx, int buffy);

protected:
  vtkOsprayRendererNode();
  ~vtkOsprayRendererNode();

  //internal structures
  unsigned char *Buffer;
  float *ZBuffer;

private:
  vtkOsprayRendererNode(const vtkOsprayRendererNode&); // Not implemented.
  void operator=(const vtkOsprayRendererNode&); // Not implemented.
};

#endif
