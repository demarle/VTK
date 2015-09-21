/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestOsprayRenderMesh.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This test verifies that we can do simple mesh rendering with ospray
// and VTK's standard rendering modes work as expected.
//
// The command line arguments are:
// -I        => run in interactive mode; unless this is used, the program will
//              not allow interaction and exit

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkImageData.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkOsprayPass.h"
#include "vtkOsprayViewNodeFactory.h"
#include "vtkOsprayWindowNode.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkTextureMapToSphere.h"
#include "vtkTransformTextureCoords.h"
#include "vtkUnsignedCharArray.h"

class renderable
{
public:
  vtkSphereSource *s;
  vtkPolyDataMapper *m;
  vtkActor *a;
  ~renderable()
  {
    s->Delete();
    m->Delete();
    a->Delete();
  }
};

renderable *MakeSphereAt(double x, double y, double z, int res=10)
{
  renderable *ret = new renderable;
  ret->s = vtkSphereSource::New();
  ret->s->SetEndTheta(180); //make half spheres to better show variation and too show f and back
  ret->s->SetStartPhi(30);
  ret->s->SetEndPhi(150);
  ret->s->SetPhiResolution(res);
  ret->s->SetThetaResolution(res);
  ret->s->SetCenter(x,y,z);
  //make texture coordinate
  vtkSmartPointer<vtkTextureMapToSphere> tc =
    vtkSmartPointer<vtkTextureMapToSphere>::New();
  tc->SetCenter(x,y,z);
  tc->PreventSeamOn();
  tc->AutomaticSphereGenerationOff();
  tc->SetInputConnection(ret->s->GetOutputPort());
  vtkSmartPointer<vtkTransformTextureCoords> tt =
    vtkSmartPointer<vtkTransformTextureCoords>::New();
  tt->SetInputConnection(tc->GetOutputPort());
  //tt->SetScale(1,0.5,1);
  //make normals
  vtkSmartPointer<vtkPolyDataNormals> nl =
    vtkSmartPointer<vtkPolyDataNormals>::New();
  nl->SetInputConnection(tt->GetOutputPort());
  nl->Update();
  //make more attribute arrays
  vtkPolyData *pd = nl->GetOutput();
  //point aligned
  vtkSmartPointer<vtkDoubleArray> da = vtkSmartPointer<vtkDoubleArray>::New();
  da->SetName("testarray1");
  da->SetNumberOfComponents(1);
  pd->GetPointData()->AddArray(da);
  int np = pd->GetNumberOfPoints();
  int nc = pd->GetNumberOfCells();
  for (int i = 0; i < np; i++)
    {
    da->InsertNextValue((double)i/(double)np);
    }
  da = vtkSmartPointer<vtkDoubleArray>::New();
  da->SetName("testarray2");
  da->SetNumberOfComponents(3);
  pd->GetPointData()->AddArray(da);
  for (int i = 0; i < np; i++)
    {
    double vals[3] = {(double)i/(double)np, (double)(i*4)/(double)np-2.0, 42.0};
    da->InsertNextTuple3(vals[0],vals[1],vals[2]);
    }
  vtkSmartPointer<vtkUnsignedCharArray> ca = vtkSmartPointer<vtkUnsignedCharArray>::New();
  ca->SetName("testarray3");
  ca->SetNumberOfComponents(3);
  pd->GetPointData()->AddArray(ca);
  for (int i = 0; i < np; i++)
    {
    unsigned char vals[3] = {(double)i/(double)np*255, (double)(1-i)/(double)np, 42};
    ca->InsertNextTuple3(vals[0],vals[1],vals[2]);
    }
  //cell aligned
  da = vtkSmartPointer<vtkDoubleArray>::New();
  da->SetName("testarray4");
  da->SetNumberOfComponents(1);
  pd->GetCellData()->AddArray(da);
  for (int i = 0; i < pd->GetNumberOfCells(); i++)
    {
    da->InsertNextValue((double)i/(double)pd->GetNumberOfCells());
    }
  da = vtkSmartPointer<vtkDoubleArray>::New();
  da->SetName("testarray5");
  da->SetNumberOfComponents(3);
  pd->GetCellData()->AddArray(da);
  for (int i = 0; i < nc; i++)
    {
    double vals[3] = {(double)i/(double)nc, (double)(i*2)/(double)nc, 42.0};
    da->InsertNextTuple3(vals[0],vals[1],vals[2]);
    }
  ca = vtkSmartPointer<vtkUnsignedCharArray>::New();
  ca->SetName("testarray6");
  ca->SetNumberOfComponents(3);
  pd->GetCellData()->AddArray(ca);
  for (int i = 0; i < nc; i++)
    {
    unsigned char vals[3] = {(double)i/(double)np*255, (double)(1-i)/(double)np, 42};
    ca->InsertNextTuple3(vals[0],vals[1],vals[2]);
    }
  ret->m = vtkPolyDataMapper::New();
  ret->m->SetInputData(pd);
  ret->a=vtkActor::New();
  ret->a->SetMapper(ret->m);
  return ret;
}

int TestOsprayRenderMesh(int argc, char* argv[])
{
  int retVal = 1;

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  iren->SetRenderWindow(renWin);
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renWin->AddRenderer(renderer);
  renderer->SetBackground(0.0,0.0,0.0);
  renWin->SetSize(400,400);
  vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
  camera->SetPosition(2,15,-2);
  camera->SetFocalPoint(2,0,-2);
  camera->SetViewUp(0,0,1);
  renderer->SetActiveCamera(camera);
  //TODO: exercise exotic types of camera manipulation

  vtkSmartPointer<vtkOsprayViewNodeFactory> vnf = vtkSmartPointer<vtkOsprayViewNodeFactory>::New();
  //vtkViewNode *vn = vnf->CreateNode(renWin);
  //vn->Build();

  vtkSmartPointer<vtkOsprayPass> ospray = vtkSmartPointer<vtkOsprayPass>::New();
  //ospray->SetSceneGraph(vtkOsprayWindowNode::SafeDownCast(vn));
  //TODO: don't yet handle multiple actors correctly for some reason
  //renderer->SetPass(ospray);


  //Now, vary of most of the many parameters that rendering can vary by
  //representations points, wireframe, surface ////////////////////
  renderable *ren = MakeSphereAt(5,0,-5);
  ren->a->GetProperty()->SetRepresentationToPoints();
  renderer->AddActor(ren->a);
  delete(ren);

  ren = MakeSphereAt(5,0,-4);
  ren->a->GetProperty()->SetRepresentationToWireframe();
  renderer->AddActor(ren->a);
  delete(ren);

  ren = MakeSphereAt(5,0,-3);
  ren->a->GetProperty()->SetRepresentationToSurface();
  renderer->AddActor(ren->a);
  delete(ren);

  //actor color ////////////////////
  ren = MakeSphereAt(4,0,-5);
  ren->a->GetProperty()->SetColor(0,1,0);
  renderer->AddActor(ren->a);
  delete(ren);

  //ambient, diffuse, and specular components
  ren = MakeSphereAt(4,0,-4, 7);
  ren->a->GetProperty()->SetAmbient(0.5);
  ren->a->GetProperty()->SetAmbientColor(0.1,0.1,0.3);
  ren->a->GetProperty()->SetDiffuse(0.4);
  ren->a->GetProperty()->SetDiffuseColor(0.5,0.1,0.1);
  ren->a->GetProperty()->SetSpecular(0.2);
  ren->a->GetProperty()->SetSpecularColor(1,1,1);
  ren->a->GetProperty()->SetSpecularPower(100);
  ren->a->GetProperty()->SetInterpolationToPhong();
  renderer->AddActor(ren->a);
  delete(ren);

  //opacity
  ren = MakeSphereAt(4,0,-3);
  ren->a->GetProperty()->SetOpacity(0.2);
  renderer->AddActor(ren->a);
  delete(ren);


  //color map cell values ////////////////////
  ren = MakeSphereAt(3,0,-5);
  ren->m->SetScalarModeToUseCellFieldData();
  ren->m->SelectColorArray(0);
  renderer->AddActor(ren->a);
  delete(ren);

  //default color component
  ren = MakeSphereAt(3,0,-4);
  ren->m->SetScalarModeToUseCellFieldData();
  ren->m->SelectColorArray(1);
  renderer->AddActor(ren->a);
  delete(ren);

  //choose color component
  ren = MakeSphereAt(3,0,-3);
  ren->m->SetScalarModeToUseCellFieldData();
  ren->m->SelectColorArray(1);
  ren->m->ColorByArrayComponent(1,1); //todo, use lut since this is deprecated
  renderer->AddActor(ren->a);
  delete(ren);

  //RGB direct
  ren = MakeSphereAt(3,0,-2);
  ren->m->SetScalarModeToUseCellFieldData();
  ren->m->SelectColorArray(2);
  renderer->AddActor(ren->a);
  delete(ren);

  //RGB through LUT
  ren = MakeSphereAt(3,0,-1);
  ren->m->SetScalarModeToUseCellFieldData();
  ren->m->SelectColorArray(2);
  ren->m->SetColorModeToMapScalars();
  renderer->AddActor(ren->a);
  delete(ren);

  //color map point values ////////////////////
  ren = MakeSphereAt(2,0,-5,6);
  ren->m->SetScalarModeToUsePointFieldData();
  ren->m->SelectColorArray("testarray1");
  renderer->AddActor(ren->a);
  delete(ren);

  //interpolate scalars before mapping
  ren = MakeSphereAt(2,0,-4,6);
  ren->m->SetScalarModeToUsePointFieldData();
  ren->m->SelectColorArray("testarray1");
  ren->m->InterpolateScalarsBeforeMappingOn();
  renderer->AddActor(ren->a);
  delete(ren);

  //lighting off, flat, gouraud and phong lighting
  ren = MakeSphereAt(1,0,-5,7);
  ren->a->GetProperty()->LightingOff();
  renderer->AddActor(ren->a);
  delete(ren);

  ren = MakeSphereAt(1,0,-4,7);
  ren->a->GetProperty()->SetInterpolationToFlat();
  renderer->AddActor(ren->a);
  delete(ren);

  ren = MakeSphereAt(1,0,-3,7);
  ren->a->GetProperty()->SetInterpolationToGouraud();
  renderer->AddActor(ren->a);
  delete(ren);

  ren = MakeSphereAt(1,0,-2,7);
  ren->a->GetProperty()->SetInterpolationToPhong();
  renderer->AddActor(ren->a);
  delete(ren);

  //texture
  int maxi = 100;
  int maxj = 100;
  vtkSmartPointer<vtkImageData> texin = vtkSmartPointer<vtkImageData>::New();
  texin->SetExtent(0,maxi,0,maxj,0,0);
  texin->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
  vtkUnsignedCharArray *aa = vtkUnsignedCharArray::SafeDownCast(texin->GetPointData()->GetScalars());
  int idx = 0;
  for (int i = 0; i<=maxi; i++)
    {
    for (int j = 0; j<=maxj; j++)
      {
      bool ival = (i/10)%2==1;
      bool jval = (j/10)%2==1;
      unsigned char val = ival^jval?255:0;
      aa->SetTuple3(idx, val, val, val);
      if (j <= 3 || j >= maxj-3)
        {
        aa->SetTuple3(idx, 255, 255, 0);
        }
      if (i <= 20 || i >= maxi-20)
        {
        aa->SetTuple3(idx, 255, 0, 0);
        }
      idx = idx + 1;
      }
    }
  ren = MakeSphereAt(0,0,-5,20);
  renderer->AddActor(ren->a);
  vtkSmartPointer<vtkTexture> texture =
    vtkSmartPointer<vtkTexture>::New();
  texture->SetInputData(texin);
  ren->a->SetTexture(texture);
  delete(ren);

  //TODO: lut manipulation and range effects

  //TODO: NaN colors

  //TODO: imagespace positional transformations

  //TODO: mapper clipping planes

  renWin->Render();

  iren->Start();
  //vn->Delete();

  return !retVal;
}
