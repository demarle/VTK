/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWindowNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkWindowNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkViewNodeCollection.h"

//============================================================================
vtkStandardNewMacro(vtkWindowNode);

//----------------------------------------------------------------------------
vtkWindowNode::vtkWindowNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Size[0] = 0;
  this->Size[1] = 0;
}

//----------------------------------------------------------------------------
vtkWindowNode::~vtkWindowNode()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
}

//----------------------------------------------------------------------------
void vtkWindowNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkWindowNode::BuildSelf()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderWindow *mine = vtkRenderWindow::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //create/delete children as required
  //to make sure my children are consistent with my renderables
  vtkViewNodeCollection *nodes = this->GetChildren();
  vtkRendererCollection *rens = mine->GetRenderers();

  //remove viewnodes if their renderables are no longer present
  vtkCollectionIterator *nit = nodes->NewIterator();
  nit->InitTraversal();
  while (!nit->IsDoneWithTraversal())
    {
    vtkViewNode *node = vtkViewNode::SafeDownCast(nit->GetCurrentObject());
    vtkObject *obj = node->GetRenderable();
    if (!rens->IsItemPresent(obj))
      {
      nodes->RemoveItem(node);
      }
    nit->GoToNextItem();
    }
  nit->Delete();

  //add viewnodes for renderables that are not yet present
  vtkCollectionIterator *rit = rens->NewIterator();
  rit->InitTraversal();
  while (!rit->IsDoneWithTraversal())
    {
    vtkRenderer *obj = vtkRenderer::SafeDownCast(rit->GetCurrentObject());
    if (!nodes->IsRenderablePresent(obj))
      {
      vtkViewNode *node = this->CreateViewNode(obj);
      nodes->AddItem(node);
      node->Delete();
      }
    rit->GoToNextItem();
    }
  rit->Delete();
}

//----------------------------------------------------------------------------
void vtkWindowNode::SynchronizeSelf()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkRenderWindow *mine = vtkRenderWindow::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //TODO: get state from our renderable
  /*
    GetAAFrames()   vtkRenderWindow virtual
    GetActualSize() vtkWindow
    GetAlphaBitPlanes()     vtkRenderWindow virtual
    GetDoubleBuffer()       vtkWindow       virtual
    GetDPI()        vtkWindow       virtual
    GetFDFrames()   vtkRenderWindow virtual
    GetFullScreen() vtkRenderWindow virtual
    GetLineSmoothing()      vtkRenderWindow virtual
    GetMapped()     vtkWindow       virtual
    GetMTime()      vtkObject       virtual
    GetMultiSamples()       vtkRenderWindow virtual
    GetNeverRendered()      vtkRenderWindow virtual
    GetNumberOfLayers()     vtkRenderWindow virtual
    GetOffScreenRendering() vtkWindow       virtual
    GetPointSmoothing()     vtkRenderWindow virtual
    GetPolygonSmoothing()   vtkRenderWindow virtual
    GetPosition()   vtkWindow       virtual
    GetScreenSize()=0       vtkWindow       pure virtual
    GetSize()       vtkWindow       virtual
    GetStereoType() vtkRenderWindow virtual
    GetSubFrames()  vtkRenderWindow virtual
    GetSwapBuffers()        vtkRenderWindow virtual
    GetTileScale()  vtkWindow       virtual
    GetTileViewport()       vtkWindow       virtual
    GetUseConstantFDOffsets()       vtkRenderWindow virtual
  */
  int * sz = mine->GetSize();
  this->Size[0] = sz[0];
  this->Size[1] = sz[1];
}
