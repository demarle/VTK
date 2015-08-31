/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayWindowNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayWindowNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayWindowNode);

//----------------------------------------------------------------------------
vtkOsprayWindowNode::vtkOsprayWindowNode()
{
  int ac = 2;
  const char* av[] = {"pvOSPRay\0","--osp:verbose\0"};
  ospInit(&ac, av);
}

//----------------------------------------------------------------------------
vtkOsprayWindowNode::~vtkOsprayWindowNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayWindowNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayWindowNode::Update()
{
  cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  cerr << this->Renderable->GetClassName() << endl;

  OSPRenderer oRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  ospSet3f(oRenderer,"bgColor",1,0,1);

  OSPCamera oCamera = ospNewCamera("perspective");
  OSPModel oModel = ospNewModel();

  //ospSetObject(vRenderer,"world",vModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);

  ospCommit(oModel);
  ospCommit(oCamera);
  ospCommit(oRenderer);

  OSPFrameBuffer osp_framebuffer = ospNewFrameBuffer
    (osp::vec2i(400, 400),
     OSP_RGBA_I8, OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);
  ospFrameBufferClear(osp_framebuffer, OSP_FB_ACCUM);

  ospRenderFrame(osp_framebuffer, oRenderer, OSP_FB_COLOR|OSP_FB_ACCUM);

  const void* rgba = ospMapFrameBuffer(osp_framebuffer);

  vtkRenderWindow *rwin = vtkRenderWindow::SafeDownCast(this->Renderable);
  rwin->SetRGBACharPixelData( 0,  0, 399, 399,
                              (unsigned char*)rgba, 0, 0 );
  ospUnmapFrameBuffer(rgba, osp_framebuffer);

  rwin->Frame();
}
