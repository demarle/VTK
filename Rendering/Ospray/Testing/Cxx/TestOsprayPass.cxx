/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestValuePasses.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This test covers

// The command line arguments are:
// -I        => run in interactive mode; unless this is used, the program will
//              not allow interaction and exit

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkOsprayPass.h"
#include "vtkOsprayViewNodeFactory.h"
#include "vtkOsprayWindowNode.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"

int TestOsprayPass(int argc, char* argv[])
{
  int retVal = 1;

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  iren->SetRenderWindow(renWin);
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renWin->AddRenderer(renderer);
  vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetPhiResolution(100);
  vtkSmartPointer<vtkPolyDataMapper> mapper=vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(sphere->GetOutputPort());
  vtkSmartPointer<vtkActor> actor=vtkSmartPointer<vtkActor>::New();
  renderer->AddActor(actor);
  actor->SetMapper(mapper);
  renderer->SetBackground(0.1,0.1,1.0);
  renWin->SetSize(400,400);
  renWin->Render();

  vtkSmartPointer<vtkOsprayViewNodeFactory> vnf = vtkSmartPointer<vtkOsprayViewNodeFactory>::New();
  vtkViewNode *vn = vnf->CreateNode(renWin);
  vn->Build();

  vtkSmartPointer<vtkOsprayPass> ddm=vtkSmartPointer<vtkOsprayPass>::New();
  ddm->SetSceneGraph(vtkOsprayWindowNode::SafeDownCast(vn));

  renderer->SetPass(ddm);
  renWin->Render();

  vtkLight *light = vtkLight::SafeDownCast(renderer->GetLights()->GetItemAsObject(0));
  double lColor[3];
  light->GetDiffuseColor(lColor);

  vtkCamera *camera = renderer->GetActiveCamera();
  double position[3];
  camera->GetPosition(position);

  for (int i = 0; i < 10; i++)
    {
    renWin->SetSize(400+i,400-i);
    double I = (double)i/10.0;
    sphere->SetThetaResolution(10+i);
    lColor[0] += I;
    lColor[1] -= I;
    cerr << lColor[0] << "," << lColor[1] << "," <<  lColor[2] << endl;
    light->SetDiffuseColor(lColor[0],lColor[1],lColor[2]); //TODO: not working, nor is lposition
    position[2] += I;
    camera->SetPosition(position);
    renderer->SetBackground(0.0,I,1-I);
    renWin->Render();
    }

  vn->Delete();

  //iren->Start();

  return !retVal;
}
