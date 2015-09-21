/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestOsprayDynamicScene.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This test verifies that dynamic scene contents work acceptably
//
// The command line arguments are:
// -I        => run in interactive mode; unless this is used, the program will
//              not allow interaction and exit

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkOsprayPass.h"
#include "vtkOsprayViewNodeFactory.h"
#include "vtkOsprayWindowNode.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"

#include <map>

int TestOsprayDynamicScene(int argc, char* argv[])
{
  int retVal = 1;

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  iren->SetRenderWindow(renWin);
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renWin->AddRenderer(renderer);
  renderer->SetBackground(0.1,0.1,1.0);
  renWin->SetSize(400,400);
  renWin->Render();

  vtkSmartPointer<vtkOsprayViewNodeFactory> vnf = vtkSmartPointer<vtkOsprayViewNodeFactory>::New();
  vtkViewNode *vn = vnf->CreateNode(renWin);
  vn->Build();

  vtkSmartPointer<vtkOsprayPass> ospray=vtkSmartPointer<vtkOsprayPass>::New();
  ospray->SetSceneGraph(vtkOsprayWindowNode::SafeDownCast(vn));
  //TODO: doesn't work right if I use ospray
  renderer->SetPass(ospray);

  //TODO: segfault if render before any geometry
  //renWin->Render();

  #define MAXFRAME 2
  vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
  camera->SetPosition(MAXFRAME*3,MAXFRAME*3,MAXFRAME*4);
  renderer->SetActiveCamera(camera);

  std::map<int, vtkActor*> actors;
  for (int i = 0; i < MAXFRAME; i++)
    {
    for (int j = 0; j < MAXFRAME; j++)
      {
      for (int k = 0; k < MAXFRAME; k++)
        {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(i,j,k);
        sphere->SetPhiResolution(10);
        sphere->SetThetaResolution(10);
        vtkSmartPointer<vtkPolyDataMapper> mapper=vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(sphere->GetOutputPort());
        vtkActor *actor= vtkActor::New();
        renderer->AddActor(actor);
        actor->SetMapper(mapper);
        actors[i*MAXFRAME*MAXFRAME+j*MAXFRAME+k] = actor;
        renWin->Render();
        }
      }
    }

  for (int i = 0; i < MAXFRAME; i++)
    {
    for (int j = 0; j < MAXFRAME; j++)
      {
      for (int k = 0; k < MAXFRAME; k++)
        {
        vtkActor *actor = actors[i*MAXFRAME*MAXFRAME+j*MAXFRAME+k];
        actor->VisibilityOff();
        renWin->Render();
        }
      }
    }

  for (int i = 0; i < MAXFRAME; i++)
    {
    for (int j = 0; j < MAXFRAME; j++)
      {
      for (int k = 0; k < MAXFRAME; k++)
        {
        vtkActor *actor = actors[i*MAXFRAME*MAXFRAME+j*MAXFRAME+k];
        actor->VisibilityOn();
        renWin->Render();
        }
      }
    }

  for (int i = 0; i < MAXFRAME; i++)
    {
    for (int j = 0; j < MAXFRAME; j++)
      {
      for (int k = 0; k < MAXFRAME; k++)
        {
        vtkActor *actor = actors[i*MAXFRAME*MAXFRAME+j*MAXFRAME+k];
        renderer->RemoveActor(actor);
        actor->Delete();
        renWin->Render();
        }
      }
    }


  vn->Delete();

  //iren->Start();


  return !retVal;
}
