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
#include "vtkDataSetReader.h"
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"

#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOsprayPass.h"
#include "vtkOsprayViewNodeFactory.h"
#include "vtkOsprayWindowNode.h"
#include "vtkOsprayRendererNode.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkPLYReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkVertexGlyphFilter.h"


// Define interaction style
class KeyPressInteractorStyle1 : public vtkInteractorStyleTrackballCamera
{
  private:
  vtkOpenGLRenderer *GLRenderer;
  vtkRenderPass *O;
  vtkRenderPass *G;

  public:

    static KeyPressInteractorStyle1* New();
    vtkTypeMacro(KeyPressInteractorStyle1, vtkInteractorStyleTrackballCamera);

    KeyPressInteractorStyle1()
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

      if(key == "P")
        {
        vtkOsprayRendererNode::maxframes=
          vtkOsprayRendererNode::maxframes+=4;
        if (vtkOsprayRendererNode::maxframes>64)
          {
          vtkOsprayRendererNode::maxframes=64;
          }
        cerr << "MF" << vtkOsprayRendererNode::maxframes << endl;
        this->GLRenderer->GetRenderWindow()->Render();
        }
      if(key == "p")
        {
        if (vtkOsprayRendererNode::maxframes>1)
          {
          vtkOsprayRendererNode::maxframes=
            vtkOsprayRendererNode::maxframes/2;
          }
        cerr << "MF" << vtkOsprayRendererNode::maxframes << endl;
        this->GLRenderer->GetRenderWindow()->Render();
        }

      if(key == "s")
        {
        cerr << "change shadows" << endl;
        if (vtkOsprayRendererNode::doshadows)
          {
          vtkOsprayRendererNode::doshadows=0;
          }
        else
          {
          vtkOsprayRendererNode::doshadows=1;
          }
        this->GLRenderer->GetRenderWindow()->Render();
        }

      if(key == "T")
        {
        cerr << "change TYPE" << endl;
        if (vtkOsprayRendererNode::rtype==1)
          {
          vtkOsprayRendererNode::rtype=0;
          }
        else
          {
          vtkOsprayRendererNode::rtype=1;
          }
        this->GLRenderer->GetRenderWindow()->Render();
        }

      
      if(key == "2")
        {
        cerr << "change SPP" << endl;
        vtkOsprayRendererNode::spp++;
        this->GLRenderer->GetRenderWindow()->Render();          
        }
      if(key == "1")
        {
        cerr << "change SPP" << endl;
        vtkOsprayRendererNode::spp=0;
        this->GLRenderer->GetRenderWindow()->Render();          
        }

// Forward events
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

};
vtkStandardNewMacro(KeyPressInteractorStyle1);

int TestOsprayDemo(int argc, char* argv[])
{
  int retVal = 1;

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  iren->SetRenderWindow(renWin);
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renWin->AddRenderer(renderer);
#if 0
  vtkSmartPointer<vtkPLYReader> polysource = vtkSmartPointer<vtkPLYReader>::New();
  polysource->SetFileName("/Data/VTKData/Data/bunny.ply");
  polysource->SetFileName("/Data/Stanford/dragon_vrip.ply");
  //polysource->SetFileName("/Data/Stanford/xyzrgb_dragon.ply");
  polysource->SetFileName("/Data/Stanford/lucy.ply");
#else
  //vtkSmartPointer<vtkSphereSource> polysource = vtkSmartPointer<vtkSphereSource>::New();
  //polysource->SetPhiResolution(100);
  vtkSmartPointer<vtkDataSetReader> source = vtkSmartPointer<vtkDataSetReader>::New();
  source->SetFileName("/Data/i2x4.vtk");
  vtkSmartPointer<vtkGlyph3D> polysource = vtkSmartPointer<vtkGlyph3D>::New();
  polysource->SetInputConnection(0, source->GetOutputPort());
  vtkSmartPointer<vtkGlyphSource2D> glyphSource = vtkSmartPointer<vtkGlyphSource2D>::New();
  glyphSource->SetGlyphTypeToVertex();
  polysource->SetInputConnection(1, glyphSource->GetOutputPort());
#endif
  vtkSmartPointer<vtkPolyDataMapper> mapper=vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(polysource->GetOutputPort());
  vtkSmartPointer<vtkActor> actor=vtkSmartPointer<vtkActor>::New();
  actor->GetProperty()->SetPointSize(10);
  actor->GetProperty()->SetRepresentationToPoints();
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

  vtkSmartPointer<KeyPressInteractorStyle1> style =
    vtkSmartPointer<KeyPressInteractorStyle1>::New();
  style->SetPipelineControlPoints((vtkOpenGLRenderer*)renderer.Get(), ospray, NULL);
  iren->SetInteractorStyle(style);
  style->SetCurrentRenderer(renderer); 


  iren->Start();
  vn->Delete();

  //iren->Start();

  return !retVal;
}
