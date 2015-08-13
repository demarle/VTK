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

#include "vtkFloatArray.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkViewNodeCollection.h"
#include "vtkViewNodeFactory.h"
#include "vtkWindowViewNode.h"

vtkViewNode *maker()
{
  vtkWindowViewNode *wvn = vtkWindowViewNode::New();
  cerr << "makers' gotta make " << wvn << endl;
  return wvn;
}

int UnitTests( int argc, char *argv[] )
{
  vtkWindowViewNode *wvn = vtkWindowViewNode::New();
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
  cerr << "factory makes" << endl;
  cerr << vn << endl;
  vn->Delete();

  vnf->RegisterOverride("vtkFloatArray", maker);
  cerr << "CREATE post override from string" << endl;
  vn = vnf->CreateNode("vtkFloatArray");
  cerr << "factory makes" << endl;
  cerr << vn << endl;
  vn->Delete();

  vtkFloatArray *fa = vtkFloatArray::New();
  cerr << "CREATE post override from object" << endl;
  cerr << "factory makes" << endl;
  vn = vnf->CreateNode(fa);
  cerr << vn << endl;

  cerr << "TRAVERSE [" << endl;
  vn->Traverse();
  cerr << "]" << endl;
  vn->Delete();
  fa->Delete();


  vtkRenderWindow *rwin = vtkRenderWindow::New();
  vnf->RegisterOverride(rwin->GetClassName(), maker);
  cerr << "CREATE node for renderwindow" << endl;
  vn = vnf->CreateNode(rwin);
  cerr << "factory makes" << endl;
  cerr << vn << endl;
  cerr << "TRAVERSE [" << endl;
  vn->Traverse();
  cerr << "]" << endl;


  cerr << "add renderer" << endl;
  vtkRenderer *ren = vtkRenderer::New();
  rwin->AddRenderer(ren);
  cerr << "TRAVERSE [" << endl;
  vn->Traverse();
  cerr << "]" << endl;

  vn->Delete();
  ren->Delete();
  rwin->Delete();

  vnf->Delete();
  return 0;
}
