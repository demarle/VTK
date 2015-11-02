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
#include "vtkOsprayRendererNode.h"
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
void vtkOsprayWindowNode::Render()
{
  vtkViewNodeCollection *renderers = this->GetChildren();
  vtkCollectionIterator *it = renderers->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkOsprayRendererNode *child =
      vtkOsprayRendererNode::SafeDownCast(it->GetCurrentObject());
    child->Render();
    it->GoToNextItem();
    }
  it->Delete();
}

//----------------------------------------------------------------------------
void vtkOsprayWindowNode::RenderSelf()
{
}

//----------------------------------------------------------------------------
void vtkOsprayWindowNode::PostRender()
{
  //composite all renderers framebuffers together
  unsigned char *rgba = new unsigned char[this->Size[0]*this->Size[1]*4];

  vtkViewNodeCollection *renderers = this->GetChildren();
  vtkCollectionIterator *it = renderers->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkOsprayRendererNode *child =
      vtkOsprayRendererNode::SafeDownCast(it->GetCurrentObject());
    child->WriteLayer(rgba, this->Size[0], this->Size[1]);
    it->GoToNextItem();
    }
  it->Delete();

  //show the result
  vtkRenderWindow *rwin = vtkRenderWindow::SafeDownCast(this->Renderable);
  rwin->SetRGBACharPixelData( 0,  0, this->Size[0]-1, this->Size[1]-1,
                              rgba, 0, 0 );
  rwin->Frame();//TODO: why twice?
  rwin->Frame();
  delete[] rgba;
}
