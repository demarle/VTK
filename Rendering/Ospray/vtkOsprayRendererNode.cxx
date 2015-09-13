/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayRendererNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayRendererNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkOsprayActorNode.h"
#include "vtkOsprayCameraNode.h"
#include "vtkRenderer.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayRendererNode);

//----------------------------------------------------------------------------
vtkOsprayRendererNode::vtkOsprayRendererNode()
{
  this->Buffer = NULL;
}

//----------------------------------------------------------------------------
vtkOsprayRendererNode::~vtkOsprayRendererNode()
{
  delete[] this->Buffer;
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::Render()
{
  cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  OSPRenderer oRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  ospSet3f(oRenderer,"bgColor",
           this->Background[0],
           this->Background[1],
           this->Background[2]);

  //camera
  vtkViewNodeCollection *nodes = this->GetChildren();
  vtkCollectionIterator *it = nodes->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkOsprayCameraNode *child =
      vtkOsprayCameraNode::SafeDownCast(it->GetCurrentObject());
    if (child)
      {
      OSPCamera oCamera = ospNewCamera("perspective");
      ospSetObject(oRenderer,"camera", oCamera);
      child->ORender(this->TiledSize, oCamera);
      ospCommit(oCamera);
      break;
      }
    it->GoToNextItem();
    }

  //actors
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkOsprayActorNode *child =
      vtkOsprayActorNode::SafeDownCast(it->GetCurrentObject());
    if (child)
      {
      OSPModel oModel = ospNewModel();
      child->ORender(oModel);
      ospSetObject(oRenderer,"model", oModel);
      ospCommit(oModel);
      }
    it->GoToNextItem();
    }
  it->Delete();

  ospCommit(oRenderer);

  OSPFrameBuffer osp_framebuffer = ospNewFrameBuffer
    (osp::vec2i(this->Size[0], this->Size[1]),
     OSP_RGBA_I8, OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);
  ospFrameBufferClear(osp_framebuffer, OSP_FB_ACCUM);

  ospRenderFrame(osp_framebuffer, oRenderer, OSP_FB_COLOR|OSP_FB_ACCUM);

  const void* rgba = ospMapFrameBuffer(osp_framebuffer);
  delete[] this->Buffer;
  this->Buffer = new unsigned char[this->Size[0]*this->Size[1]*4];
  memcpy((void*)this->Buffer, rgba, this->Size[0]*this->Size[1]*4);
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::WriteLayer(unsigned char *buffer,
                                       int buffx, int buffy)
{
  unsigned char *iptr = this->Buffer;
  unsigned char *optr = buffer;
  for (int i = 0; i < buffx && i < this->Size[0]; i++)
    {
    for (int j = 0; j < buffy && i < this->Size[1]; j++)
      {
      *optr++ = *iptr++;
      *optr++ = *iptr++;
      *optr++ = *iptr++;
      *optr++ = *iptr++;
      }
    }
  cerr << "DONE" << endl;
}
