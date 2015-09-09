/*=========================================================================

  Program:   Visualization Toolkit
  Module:    Mace.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkActor.h"
#include "vtkActorNode.h"
#include "vtkCamera.h"
#include "vtkCameraNode.h"
#include "vtkLight.h"
#include "vtkLightNode.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRendererNode.h"
#include "vtkRenderWindow.h"
#include "vtkSphereSource.h"
#include "vtkViewNodeCollection.h"
#include "vtkViewNodeFactory.h"
#include "vtkWindowNode.h"

//------------------------------------------------------------------------------
//make subclasses so test can control precisely what they do

#include "vtkObjectFactory.h"
class vtkMyActorNode : public vtkActorNode
{
public:
  static vtkMyActorNode* New();
  vtkTypeMacro(vtkMyActorNode, vtkActorNode);
  virtual void Update() {
    cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  }
  vtkMyActorNode() {};
  ~vtkMyActorNode() {};
};
vtkStandardNewMacro(vtkMyActorNode);

class vtkMyCameraNode : public vtkCameraNode
{
public:
  static vtkMyCameraNode* New();
  vtkTypeMacro(vtkMyCameraNode, vtkCameraNode);
  virtual void Update() {
    cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  }
  vtkMyCameraNode() {};
  ~vtkMyCameraNode() {};
};
vtkStandardNewMacro(vtkMyCameraNode);

class vtkMyLightNode : public vtkLightNode
{
public:
  static vtkMyLightNode* New();
  vtkTypeMacro(vtkMyLightNode, vtkLightNode);
  virtual void Update() {
    cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  }
  vtkMyLightNode() {};
  ~vtkMyLightNode() {};
};
vtkStandardNewMacro(vtkMyLightNode);

class vtkMyRendererNode : public vtkRendererNode
{
public:
  static vtkMyRendererNode* New();
  vtkTypeMacro(vtkMyRendererNode, vtkRendererNode);
  virtual void Update() {
    cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  }
  vtkMyRendererNode() {};
  ~vtkMyRendererNode() {};
};
vtkStandardNewMacro(vtkMyRendererNode);

class vtkMyWindowNode : public vtkWindowNode
{
public:
  static vtkMyWindowNode* New();
  vtkTypeMacro(vtkMyWindowNode, vtkWindowNode);
  virtual void Update() {
    cerr << "Hello from " << this << " " << this->GetClassName() << endl;
  }
  vtkMyWindowNode() {};
  ~vtkMyWindowNode() {};
};
vtkStandardNewMacro(vtkMyWindowNode);
//------------------------------------------------------------------------------


//builders that make our specialized versions
vtkViewNode *act_maker()
{
  vtkMyActorNode *vn = vtkMyActorNode::New();
  cerr << "make actor node " << vn << endl;
  return vn;
}

vtkViewNode *cam_maker()
{
  vtkMyCameraNode *vn = vtkMyCameraNode::New();
  cerr << "make camera node " << vn << endl;
  return vn;
}

vtkViewNode *light_maker()
{
  vtkMyLightNode *vn = vtkMyLightNode::New();
  cerr << "make light node " << vn << endl;
  return vn;
}

vtkViewNode *ren_maker()
{
  vtkMyRendererNode *vn = vtkMyRendererNode::New();
  cerr << "make renderer node " << vn << endl;
  return vn;
}

vtkViewNode *win_maker()
{
  vtkMyWindowNode *vn = vtkMyWindowNode::New();
  cerr << "make window node " << vn << endl;
  return vn;
}

//the test, which exercises the classes in meaningful ways
int UnitTests( int argc, char *argv[] )
{
  vtkWindowNode *wvn = vtkWindowNode::New();
  cerr << "made " << wvn << endl;

  vtkViewNodeCollection *vnc = vtkViewNodeCollection::New();
  cerr << "made " << vnc << endl;
  vnc->AddItem(wvn);
  vnc->PrintSelf(cerr, vtkIndent(0));
  wvn->Delete();
  vnc->Delete();

  vtkViewNode *vn = NULL;
  vtkViewNodeFactory *vnf = vtkViewNodeFactory::New();
  cerr << "CREATE pre override" << endl;
  vn = vnf->CreateNode(vnc);
  if (vn)
    {
    cerr << "Shouldn't have made anything" << endl;
    return 1;
    }
  cerr << "factor made nothing as it should have" << endl;

  vtkRenderWindow *rwin = vtkRenderWindow::New();
  vnf->RegisterOverride(rwin->GetClassName(), win_maker);
  cerr << "CREATE node for renderwindow" << endl;
  vn = vnf->CreateNode(rwin);

  cerr << "factory makes" << endl;
  cerr << vn << endl;
  cerr << "BUILD [" << endl;
  vn->Build();
  cerr << "]" << endl;

  cerr << "add renderer" << endl;
  vtkRenderer *ren = vtkRenderer::New();
  vnf->RegisterOverride(ren->GetClassName(), ren_maker);
  rwin->AddRenderer(ren);

  vtkLight *light = vtkLight::New();
  vnf->RegisterOverride(light->GetClassName(), light_maker);
  ren->AddLight(light);
  light->Delete();

  vtkCamera *cam = vtkCamera::New();
  vnf->RegisterOverride(cam->GetClassName(), cam_maker);
  cam->Delete();

  vtkActor *actor = vtkActor::New();
  vnf->RegisterOverride(actor->GetClassName(), act_maker);
  ren->AddActor(actor);
  actor->Delete();

  vtkSphereSource *sphere = vtkSphereSource::New();
  vtkPolyDataMapper *pmap = vtkPolyDataMapper::New();
  pmap->SetInputConnection(sphere->GetOutputPort());
  actor->SetMapper(pmap);
  rwin->Render();
  sphere->Delete();
  pmap->Delete();

  cerr << "BUILD [" << endl;
  vn->Build();
  cerr << "]" << endl;
  cerr << "SYNCHRONIZE [" << endl;
  vn->Synchronize();
  cerr << "]" << endl;
  cerr << "RENDER [" << endl;
  vn->Render();
  cerr << "]" << endl;

  vn->Delete();
  ren->Delete();
  rwin->Delete();

  vnf->Delete();
  return 0;
}
