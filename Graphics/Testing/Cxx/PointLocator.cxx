#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPointLocator.h"

#include "vtkRegressionTestImage.h"
#include "vtkDebugLeaks.h"

int main( int argc, char *argv[] )
{
  vtkDebugLeaks::PromptUserOff();

  vtkRenderer *renderer = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer(renderer);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);

  vtkSphereSource *sphere = vtkSphereSource::New();
    sphere->SetThetaResolution(8); sphere->SetPhiResolution(8);
    sphere->SetRadius(1.0);
    sphere->Update();
  vtkPolyDataMapper *sphereMapper = vtkPolyDataMapper::New();
    sphereMapper->SetInput(sphere->GetOutput());
  vtkActor *sphereActor = vtkActor::New();
    sphereActor->SetMapper(sphereMapper);
    
  vtkSphereSource *spot = vtkSphereSource::New();
    spot->SetPhiResolution(6);
    spot->SetThetaResolution(6);
    spot->SetRadius(0.1);

  vtkPolyDataMapper *spotMapper = vtkPolyDataMapper::New();
    spotMapper->SetInput(spot->GetOutput());

  // Build a locator 
  vtkPointLocator *pointLocator = vtkPointLocator::New();
  pointLocator->SetDataSet(sphere->GetOutput());
  pointLocator->BuildLocator();

  // 
  float p1[] = {2.0, 1.0, 3.0};
  float p2[] = {0.0, 0.0, 0.0};

  // Find closest point
  vtkIdType ptId;
  float dist;
  p1[0] = 0.1; p1[1] = -0.2; p1[2] = 0.2;
  ptId = pointLocator->FindClosestPoint(p1);
  vtkActor *closestPointActor = vtkActor::New();
    closestPointActor->SetMapper(spotMapper);
    closestPointActor->SetPosition(sphere->GetOutput()->GetPoints()->GetPoint(ptId));
    closestPointActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

  // Find closest point within radius
  float radius = 5.0;
  p1[0] = .2; p1[1] = 1.0; p1[2] = 1.0;
  ptId = pointLocator->FindClosestPointWithinRadius(radius, p1, dist);
  vtkActor *closestPointActor2 = vtkActor::New();
    closestPointActor2->SetMapper(spotMapper);
    closestPointActor2->SetPosition(sphere->GetOutput()->GetPoints()->GetPoint(ptId));
    closestPointActor2->GetProperty()->SetColor(0.0, 1.0, 0.0);
  
  renderer->AddActor(sphereActor);
  renderer->AddActor(closestPointActor);
  renderer->AddActor(closestPointActor2);
  renderer->SetBackground(1,1,1);
  renWin->SetSize(300,300);

  // interact with data
  renWin->Render();

  int retVal = vtkRegressionTestImage( renWin );

  // Clean up
  renderer->Delete();
  renWin->Delete();
  iren->Delete();
  sphere->Delete();
  sphereMapper->Delete();
  sphereActor->Delete();
  spot->Delete();
  spotMapper->Delete();
  closestPointActor->Delete();
  closestPointActor2->Delete();
  pointLocator->FreeSearchStructure();
  pointLocator->Delete();

  return !retVal;
}
