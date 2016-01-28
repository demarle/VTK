/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestOsprayPass.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This test verifies that we can hot swap ospray and GL backends.
//
// The command line arguments are:
// -I        => run in interactive mode; unless this is used, the program will
//              not allow interaction and exit

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOsprayPass.h"
#include "vtkOsprayViewNodeFactory.h"
#include "vtkOsprayWindowNode.h"
#include "vtkPolyDataMapper.h"
#include "vtkPLYReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera
{
  private:
  vtkOpenGLRenderer *GLRenderer;
  vtkRenderPass *O;
  vtkRenderPass *G;

  public:

    static KeyPressInteractorStyle* New();
    vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);

    KeyPressInteractorStyle()
    {
      this->SetPipelineControlPoints(NULL,NULL,NULL);
    }

    void SetPipelineControlPoints(vtkOpenGLRenderer *g,
                                  vtkRenderPass *_O,
                                  vtkRenderPass *_G)
    {
      this->GLRenderer = g;
      this->O = _O;
      this->G = _G;
    }

    virtual void OnKeyPress()
    {
      if (this->GLRenderer == NULL)
        {
        return;
        }

      // Get the keypress
      vtkRenderWindowInteractor *rwi = this->Interactor;
      std::string key = rwi->GetKeySym();

      if(key == "c")
        {
        vtkRenderPass * current = this->GLRenderer->GetPass();
        if (current == this->G)
          {
          cerr << "OSPRAY rendering" << endl;
          this->GLRenderer->SetPass(this->O);
          this->GLRenderer->GetRenderWindow()->Render();
          }
        else if (current == this->O)
          {
          cerr << "GL rendering" << endl;
          this->GLRenderer->SetPass(this->G);
          this->GLRenderer->GetRenderWindow()->Render();
          }
        }

      // Forward events
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

};
vtkStandardNewMacro(KeyPressInteractorStyle);

int TestOsprayPass(int argc, char* argv[])
{
  int retVal = 1;

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  iren->SetRenderWindow(renWin);
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renWin->AddRenderer(renderer);
#if 1
  vtkSmartPointer<vtkPLYReader> polysource = vtkSmartPointer<vtkPLYReader>::New();
  polysource->SetFileName("/Data/VTKData/Data/bunny.ply");
  polysource->SetFileName("/Data/Stanford/dragon_vrip.ply");
  //polysource->SetFileName("/Data/Stanford/xyzrgb_dragon.ply");
  polysource->SetFileName("/Data/Stanford/lucy.ply");
#else
  vtkSmartPointer<vtkSphereSource> polysource = vtkSmartPointer<vtkSphereSource>::New();
  polysource->SetPhiResolution(100);
#endif
  vtkSmartPointer<vtkPolyDataMapper> mapper=vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(polysource->GetOutputPort());
  vtkSmartPointer<vtkActor> actor=vtkSmartPointer<vtkActor>::New();
  renderer->AddActor(actor);
  actor->SetMapper(mapper);
  renderer->SetBackground(0.1,0.1,1.0);
  renWin->SetSize(400,400);
  renWin->Render();

  vtkSmartPointer<vtkOsprayViewNodeFactory> vnf = vtkSmartPointer<vtkOsprayViewNodeFactory>::New();
  vtkViewNode *vn = vnf->CreateNode(renWin);
  vn->Build();

  vtkSmartPointer<vtkOsprayPass> ospray=vtkSmartPointer<vtkOsprayPass>::New();
  ospray->SetSceneGraph(vtkOsprayWindowNode::SafeDownCast(vn));

  renderer->SetPass(ospray);
  renWin->Render();

  vtkLight *light = vtkLight::SafeDownCast(renderer->GetLights()->GetItemAsObject(0));
  double lColor[3];
  light->GetDiffuseColor(lColor);

  vtkCamera *camera = renderer->GetActiveCamera();
  double position[3];
  camera->GetPosition(position);

  vtkSmartPointer<KeyPressInteractorStyle> style =
    vtkSmartPointer<KeyPressInteractorStyle>::New();
  style->SetPipelineControlPoints((vtkOpenGLRenderer*)renderer.Get(), ospray, NULL);
  iren->SetInteractorStyle(style);
  style->SetCurrentRenderer(renderer); 


  iren->Start();
  vn->Delete();

  //iren->Start();

  return !retVal;
}
