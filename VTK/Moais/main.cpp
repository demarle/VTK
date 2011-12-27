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
#include "vtkSMPContourFilter.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"

#include "vtkCellArray.h"

int main( int argc, char** argv )
{
  bool parallel = argc == 1 || std::string(argv[1]) != "1";

  /* === Reading 3D model === */

  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName("../../VTKData/Data/bunny.vtk");

  /* === Testing transform filter === */

  vtkTransformFilter* pre_transform = vtkTransformFilter::New();
  pre_transform->SetInputConnection( polyReader->GetOutputPort() );

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( pre_transform->GetOutputPort() );

  pre_transform->Delete();

  if ( parallel )
  {
    vtkSMPTransform* t = vtkSMPTransform::New();
    t->Scale( -1., -1., -1. );
    pre_transform->SetTransform( t );
    transform->SetTransform( t );
    t->Delete();
  }
  else
  {
    vtkTransform* t = vtkTransform::New();
    t->Scale( -1., -1., -1. );
    pre_transform->SetTransform( t );
    transform->SetTransform( t );
    t->Delete();
  }

  /* === Testing contour filter === */
  transform->Update();
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(transform->GetOutput()->GetNumberOfPoints());
  s->SetName("scalars");
  for (vtkIdType i = 0; i < transform->GetOutput()->GetNumberOfPoints(); ++i)
  {
    double* h = transform->GetOutput()->GetPoint(i);
    s->SetTuple(i,&h[2]);
  }
  transform->GetOutput()->GetPointData()->SetScalars( s );
  s->Delete();

  vtkContourFilter* isosurface = parallel ? vtkSMPContourFilter::New() : vtkContourFilter::New();
  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->GenerateValues(2, 0.0, 1.0);
  isosurface->UseScalarTreeOff();
  transform->Delete();

/* *
  // Simulate a call to vtkRenderWindow::Render()
  polyReader->Delete();
  isosurface->Update();
  isosurface->Delete();
/*/
  vtkPolyDataMapper* map = vtkPolyDataMapper::New();
  map->SetInputConnection( isosurface->GetOutputPort() );
  isosurface->Delete();

  vtkActor* object = vtkActor::New();
  object->SetMapper( map );
  map->Delete();

  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection( polyReader->GetOutputPort() );
  polyReader->Delete();

  vtkActor* reference = vtkActor::New();
  reference->SetMapper( mapper );
  reference->AddPosition( .2, 0., 0. );
  mapper->Delete();

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );
  viewport->AddActor( object );
  viewport->AddActor( reference );
  object->Delete();
  reference->Delete();

  vtkRenderWindow* window = vtkRenderWindow::New();
  window->AddRenderer( viewport );
  viewport->Delete();

  vtkRenderWindowInteractor* eventsCatcher = vtkRenderWindowInteractor::New();
  eventsCatcher->SetRenderWindow( window );
  window->Delete();

  eventsCatcher->Initialize();
  eventsCatcher->Start();

  eventsCatcher->Delete();
/* */
  cout << "should exit" << endl;
  return 0;
}
