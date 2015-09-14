/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayViewNodeFactory.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayViewNodeFactory.h"
#include "vtkObjectFactory.h"

#include "vtkOsprayActorNode.h"
#include "vtkOsprayCameraNode.h"
#include "vtkOsprayLightNode.h"
#include "vtkOsprayRendererNode.h"
#include "vtkOsprayWindowNode.h"

vtkViewNode *win_maker()
{
  vtkOsprayWindowNode *vn = vtkOsprayWindowNode::New();
  return vn;
}

vtkViewNode *ren_maker()
{
  vtkOsprayRendererNode *vn = vtkOsprayRendererNode::New();
  return vn;
}

vtkViewNode *act_maker()
{
  vtkOsprayActorNode *vn = vtkOsprayActorNode::New();
  return vn;
}

vtkViewNode *cam_maker()
{
  vtkOsprayCameraNode *vn = vtkOsprayCameraNode::New();
  return vn;
}

vtkViewNode *light_maker()
{
  cerr << "MAKE NEW LIGHT" << endl;
  vtkOsprayLightNode *vn = vtkOsprayLightNode::New();
  return vn;
}

//============================================================================
vtkStandardNewMacro(vtkOsprayViewNodeFactory);

//----------------------------------------------------------------------------
vtkOsprayViewNodeFactory::vtkOsprayViewNodeFactory()
{
  this->RegisterOverride("vtkCocoaRenderWindow", win_maker); //TODO: sysname from OF
  //see vtkRenderWindow::GetRenderLibrary
  this->RegisterOverride("vtkOpenGLRenderer", ren_maker); //TODO: if not OpenGL?
  this->RegisterOverride("vtkOpenGLActor", act_maker);
  this->RegisterOverride("vtkOpenGLCamera", cam_maker);
  this->RegisterOverride("vtkOpenGLLight", light_maker);
}

//----------------------------------------------------------------------------
vtkOsprayViewNodeFactory::~vtkOsprayViewNodeFactory()
{
}

//----------------------------------------------------------------------------
void vtkOsprayViewNodeFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
