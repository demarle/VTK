#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkSMPApplication.h"
#include "vtkSMPWarpVector.h"

int core( int argc, char** argv )
{
  vtkConeSource* cone = vtkConeSource::New();
  cone->SetHeight( 3.0 );
  cone->SetRadius( 1.0 );
  cone->SetResolution( 10 );

  vtkSMPWarpVector* filter = vtkSMPWarpVector::New();
  filter->SetScaleFactor( 2.0 );
  filter->SetInputConnection( cone->GetOutputPort() );
  
  vtkPolyDataMapper* coneMapper = vtkPolyDataMapper::New();
  coneMapper->SetInputConnection( filter->GetOutputPort() );

  vtkActor* coneActor = vtkActor::New();
  coneActor->SetMapper( coneMapper );

  vtkRenderer* ren1= vtkRenderer::New();
  ren1->AddActor( coneActor );
  ren1->SetBackground( 0.1, 0.2, 0.4 );

  vtkRenderWindow* renWin = vtkRenderWindow::New();
  renWin->AddRenderer( ren1 );
  renWin->SetSize( 300, 300 );

  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);

  iren->Initialize();
  iren->Start();
  
  cone->Delete();
  coneMapper->Delete();
  coneActor->Delete();
  ren1->Delete();
  renWin->Delete();
  iren->Delete();
  style->Delete();

  return 0;
}

int main( int argc, char** argv )
{
  vtkSMPApplication* app = vtkSMPApplication::New();
  app->SetCoreFunction( &core );
  int run = app->Execute( argc, argv );
  app->Delete();
  return run;
}
