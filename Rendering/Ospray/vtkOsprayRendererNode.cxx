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
#include "vtkOsprayLightNode.h"
#include "vtkRenderer.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

int vtkOsprayRendererNode::maxframes = 1;
int vtkOsprayRendererNode::rtype=0;
int vtkOsprayRendererNode::doshadows=0;
int vtkOsprayRendererNode::spp=1;
//============================================================================
vtkStandardNewMacro(vtkOsprayRendererNode);

//----------------------------------------------------------------------------
vtkOsprayRendererNode::vtkOsprayRendererNode()
{
  this->Buffer = NULL;
  this->ZBuffer = NULL;
  this->Model = NULL;
  this->ORend = NULL;
}

//----------------------------------------------------------------------------
vtkOsprayRendererNode::~vtkOsprayRendererNode()
{
  delete[] this->Buffer;
  delete[] this->ZBuffer;
  ospRelease((OSPModel)this->Model);
  ospRelease((OSPRenderer)this->ORend);
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::Render()
{
  OSPRenderer oRenderer = NULL;
  static int lRend = 0;
  if (lRend != rtype)
    {
    lRend = rtype;
    ospRelease((osp::Renderer*)this->ORend);
    this->ORend = NULL;
    }
  if (!this->ORend)
    {
    ospRelease((osp::Renderer*)this->ORend);
    if (rtype==0)
      {
      oRenderer = (osp::Renderer*)ospNewRenderer("ao16");
      }
    else
      {
      oRenderer = (osp::Renderer*)ospNewRenderer("obj");
      }
    //TODO: other options include {ao{1,2,4,8,16},obj,tachyon,pathtracer,raycast,volume...} - which to pick?
    //git grep OSP_REGISTER_RENDERER
    //ao - simple ambient occlusion (X semi-rand sample per hit) does not account for opacity
    //obj - implements much of OBJ standard, but not all
    //volume - for volumes with intermixed surfaces
    //tachyon - scene characteristics from John Stone's Tachyon
    //pathtracer - experimental code
    //raycast - 1 hit and done w color chosen from a few possibilities (normal, etc)
    //everything but raycast is a possibility, but none seem to be feature perfect right now

    this->ORend = oRenderer;
    }
  else
    {
    oRenderer = (osp::Renderer*)this->ORend;
    }

  if (doshadows)
    {
    ospSet1i(oRenderer,"shadowsEnabled",1);
    }
  else
    {
    ospSet1i(oRenderer,"shadowsEnabled",0);
    }
  ospSet1i(oRenderer,"spp",spp);

  ospSet3f(oRenderer,"bgColor",
           this->Background[0],
           this->Background[1],
           this->Background[2]);

  //camera
  //TODO: this repeated traversal to find things of particular types
  //is bad, find something smarter
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
      ospRelease(oCamera);
      break;
      }
    it->GoToNextItem();
    }

  //lights
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkOsprayLightNode *child =
      vtkOsprayLightNode::SafeDownCast(it->GetCurrentObject());
    if (child)
      {
      child->ORender(oRenderer);
      break;
      }
    it->GoToNextItem();
    }

  //actors
  OSPModel oModel=NULL;
  it->InitTraversal();
  unsigned int recent = 0;
  while (!it->IsDoneWithTraversal())
    {
    vtkOsprayActorNode *child =
      vtkOsprayActorNode::SafeDownCast(it->GetCurrentObject());
    if (child)
      {
      if (child->RenderTime > recent)
        {
        recent = child->RenderTime;
        }
      }
    it->GoToNextItem();
    }
  static int cnt = 0;
  if (recent > this->RenderTime || !this->Model)
    {
    cnt = 0;
    ospRelease((OSPModel)this->Model);

    oModel = ospNewModel();
    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      vtkOsprayActorNode *child =
        vtkOsprayActorNode::SafeDownCast(it->GetCurrentObject());
      if (child)
        {
        child->ORender(oRenderer, oModel);
        }
      it->GoToNextItem();
      }
    this->Model = oModel;
    this->RenderTime.Modified();
    ospSetObject(oRenderer,"model", oModel);
    ospCommit(oModel);
    }
  else
    {
    oModel = (OSPModel)this->Model;
    }
  it->Delete();
  ospCommit(oRenderer);

  OSPFrameBuffer osp_framebuffer = ospNewFrameBuffer
    (osp::vec2i(this->Size[0], this->Size[1]),
     OSP_RGBA_I8, OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);
  ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
  for (int i = 0; i < maxframes; i++)
    {
    ospRenderFrame(osp_framebuffer, oRenderer, OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
    }
  const void* rgba = ospMapFrameBuffer(osp_framebuffer, OSP_FB_COLOR);
  delete[] this->Buffer;
  this->Buffer = new unsigned char[this->Size[0]*this->Size[1]*4];
  memcpy((void*)this->Buffer, rgba, this->Size[0]*this->Size[1]*4);
  ospUnmapFrameBuffer(rgba, osp_framebuffer);

  const void* Z = ospMapFrameBuffer(osp_framebuffer, OSP_FB_DEPTH);
  delete[] this->ZBuffer;
  this->ZBuffer = new float[this->Size[0]*this->Size[1]*1];
  memcpy((void*)this->ZBuffer, Z, this->Size[0]*this->Size[1]*1*sizeof(float));
  ospUnmapFrameBuffer(Z, osp_framebuffer);

  ospRelease(osp_framebuffer);
}

//----------------------------------------------------------------------------
void vtkOsprayRendererNode::WriteLayer(unsigned char *buffer,
                                       int buffx, int buffy)
{
  //TODO: have to keep depth information around in VTK side for
  //parallel compositing to work too.
  unsigned char *iptr = this->Buffer;
  float *zptr = this->ZBuffer;
  unsigned char *optr = buffer;
  if (this->Layer == 0)
    {
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
    }
  else
    {
    for (int i = 0; i < buffx && i < this->Size[0]; i++)
      {
      for (int j = 0; j < buffy && i < this->Size[1]; j++)
        {
        if (!std::isinf(*zptr))
          {
          *optr++ = *iptr++;
          *optr++ = *iptr++;
          *optr++ = *iptr++;
          *optr++ = *iptr++;
          }
        else
          {
          optr+=4;
          iptr+=4;
          }
        zptr++;
        }
      }
    }
}
