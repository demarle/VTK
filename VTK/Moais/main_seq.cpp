#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyData.h"

int main( int argc, char** argv )
{
  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName("../../VTKData/Data/lucy.vtk");

  vtkTransformFilter* filter = vtkTransformFilter::New();
  vtkTransform* t = vtkTransform::New();
  t->Scale( -1., -1., -1. );
  filter->SetTransform( t );
  filter->SetInputConnection( polyReader->GetOutputPort() );
  t->Delete();

/* */
  // Simulate a call to vtkRenderWindow::Render()
  filter->Update();
/*/
  vtkPolyDataMapper* map = vtkPolyDataMapper::New();
  map->SetInputConnection( filter->GetOutputPort() );

  vtkActor* object = vtkActor::New();
  object->SetMapper( map );

  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection( polyReader->GetOutputPort() );

  vtkActor* reference = vtkActor::New();
  reference->SetMapper( mapper );
  reference->AddPosition( .2, 0., 0. );

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );
  viewport->AddActor( object );
  viewport->AddActor( reference );

  vtkRenderWindow* window = vtkRenderWindow::New();
  window->AddRenderer( viewport );

  vtkRenderWindowInteractor* eventsCatcher = vtkRenderWindowInteractor::New();
  eventsCatcher->SetRenderWindow( window );

  eventsCatcher->Initialize();
  eventsCatcher->Start();

  eventsCatcher->Delete();
  window->Delete();
  viewport->Delete();
  reference->Delete();
  object->Delete();
  mapper->Delete();
  map->Delete();
/* */
  filter->Delete();
  polyReader->Delete();

  return 0;
}
