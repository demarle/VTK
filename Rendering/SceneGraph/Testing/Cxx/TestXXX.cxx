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

#include "vtkViewNodeCollection.h"
#include "vtkViewNodeFactory.h"
#include "vtkWindowViewNode.h"
#include "vtkFloatArray.h"

vtkViewNode *maker()
{
  cerr << "CALLING MAKER" << endl;
  return vtkWindowViewNode::New();
}

int TestXXX( int argc, char *argv[] )
{
  vtkWindowViewNode *wvn = vtkWindowViewNode::New();
  wvn->PrintSelf(cerr, vtkIndent(0));

  vtkViewNodeCollection *vnc = vtkViewNodeCollection::New();
  vnc->AddItem(wvn);
  vnc->PrintSelf(cerr, vtkIndent(0));
  wvn->Delete();
  vnc->Delete();

  vtkViewNode *vn = NULL;
  vtkViewNodeFactory *vnf = vtkViewNodeFactory::New();
  cerr << "CREATE pre override" << endl;
  vn = vnf->CreateNode(vnc);
  cerr << vn << endl;
  vn->Delete();

  vnf->RegisterOverride("vtkFloatArray", maker);
  cerr << "CREATE post override from string" << endl;
  vn = vnf->CreateNode("vtkFloatArray");
  cerr << vn << endl;
  vn->Delete();

  vtkFloatArray *fa = vtkFloatArray::New();
  cerr << "CREATE post override from object" << endl;
  vnf->CreateNode(fa);
  cerr << vn << endl;
  vn->Delete();
  fa->Delete();

  vnf->Delete();
  return 0;
}
