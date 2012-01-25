/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestExtractSelection.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkNew.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkRegressionTestImage.h"
#include "vtkTestUtilities.h"
#include "vtkGlyph3D.h"
#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include <vtkPointData.h>
#include <vtkConeSource.h>
#include <vtkCamera.h>

int TestGlyph3D(int argc, char* argv[])
{
  vtkSmartPointer<vtkDoubleArray> vectors = 
    vtkSmartPointer<vtkDoubleArray>::New();
  vectors->SetName("Normals"); 
  vectors->SetNumberOfComponents(2);
  vectors->InsertNextTuple2(1,1);
  vectors->InsertNextTuple2(1,0);
  vectors->InsertNextTuple2(0,1); 

  vtkSmartPointer<vtkPoints> points = 
    vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(0,0,0);
  points->InsertNextPoint(1,1,1);
  points->InsertNextPoint(2,2,2);
  vtkSmartPointer<vtkPolyData> polydata = 
    vtkSmartPointer<vtkPolyData>::New(); 
  polydata->SetPoints(points);
  
 
  polydata->GetPointData()->AddArray(vectors);
  polydata->Update();

  vtkSmartPointer<vtkPolyData> glyph = 
    vtkSmartPointer<vtkPolyData>::New();
 
  vtkSmartPointer<vtkConeSource> glyphSource = 
      vtkSmartPointer<vtkConeSource>::New();
 
  vtkSmartPointer<vtkGlyph3D> glyph3D = 
    vtkSmartPointer<vtkGlyph3D>::New(); 
  glyph3D->SetSource(glyphSource->GetOutput());
  glyph3D->SetInput(polydata);
  glyph3D->SetInputArrayToProcess(1,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,"Normals");
  glyph3D->SetVectorModeToUseVector();
  glyph3D->Update(); 

  // Visualize

  vtkSmartPointer<vtkPolyDataMapper> mapper = 
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(glyph3D->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper.GetPointer());

  vtkNew<vtkRenderer> ren;
  ren->SetBackground(0,0,0);
  ren->AddActor(actor.GetPointer());
  ren->ResetCamera();
  ren->GetActiveCamera()->Zoom(1.5);

  vtkNew<vtkRenderWindow> renWin;

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  renWin->AddRenderer(ren.GetPointer());
  renWin->SetSize(300,300);
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin.GetPointer());
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    }
  return !retVal;
}
