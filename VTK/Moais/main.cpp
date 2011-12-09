#include "vtkTransform.h"
#include "vtkSMPTransform.h"
#include "vtkSMPTransformFilter.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyData.h"

#include <numa.h>

int main( int argc, char** argv )
{
  int cpu = numa_num_thread_cpus();
  
  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName("../../VTKData/Data/lucy.vtk");

  vtkTransformFilter* pre_filter = vtkTransformFilter::New();
  pre_filter->SetInputConnection( polyReader->GetOutputPort() );
  
  vtkTransformFilter* filter = vtkTransformFilter::New();
  filter->SetInputConnection( pre_filter->GetOutputPort() );
  
  pre_filter->Delete();
  
  if ( cpu == 1 )
  {
    vtkTransform* t = vtkTransform::New();
    t->Scale( -1., -1., -1. );
    pre_filter->SetTransform( t );
    filter->SetTransform( t );
    t->Delete();
  }
  else
  {
    vtkSMPTransform* t = vtkSMPTransform::New();
    t->Scale( -1., -1., -1. );
    pre_filter->SetTransform( t );
    filter->SetTransform( t );
    t->Delete();
  }

/* */
  // Simulate a call to vtkRenderWindow::Render()
  cout << cpu << "cores" << endl;
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

#if defined(NO_FILTER) || defined(NO_TRANSFORM)
  vtkObjectFactory::UnRegisterFactory( of );
  of->Delete();
#endif
  return 0;
}
