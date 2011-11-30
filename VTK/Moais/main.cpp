#include "vtkTransform.h"
#include "vtkTransformFilter.h"
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

int main( int argc, char** argv )
{
  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName("../../VTKData/Data/lucy.vtk");

  vtkTransformFilter* filter_seq = vtkTransformFilter::New();
  vtkTransform* t = vtkTransform::New();
  t->Scale( -1., -1., -1. );
  filter_seq->SetTransform( t );
  filter_seq->SetInputConnection( polyReader->GetOutputPort() );
  t->Delete();

  vtkSMPTransformFilter* filter_1 = vtkSMPTransformFilter::New();
  t = vtkTransform::New();
  t->Scale( -1., -1., -1. );
  filter_1->SetTransform( t );
  filter_1->SetInputConnection( polyReader->GetOutputPort() );
  t->Delete();

  vtkTransformFilter* filter_2 = vtkTransformFilter::New();
  vtkSMPTransform* tt = vtkSMPTransform::New();
  tt->Scale( -1., -1., -1. );
  filter_2->SetTransform( tt );
  filter_2->SetInputConnection( polyReader->GetOutputPort() );
  tt->Delete();

/* */
  // Simulate a call to vtkRenderWindow::Render()
  if ( argc != 1 )
  {
    cout << endl << "Sequential" << endl;
    filter_seq->Update();
  }
  filter_seq->Delete();
  cout << endl << "Kaapi1" << endl;
  filter_1->Update();
  cout << endl << "Kaapi2" << endl;
  filter_2->Update();
  filter_2->Delete();
/*/
  vtkPolyDataMapper* map = vtkPolyDataMapper::New();
  map->SetInputConnection( filter_1->GetOutputPort() );

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
  filter_1->Delete();
  polyReader->Delete();

#if defined(NO_FILTER) || defined(NO_TRANSFORM)
  vtkObjectFactory::UnRegisterFactory( of );
  of->Delete();
#endif
  return 0;
}
