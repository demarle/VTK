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
#include "vtkContourFilter.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"

int main( int argc, char** argv )
{
  int parallel = argc == 1 ? 48 : atoi(argv[1]);

  /* === Reading 3D model === */

  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName("../../VTKData/Data/bunny.vtk");

  /* === Testing transform filter === */

  vtkTransformFilter* pre_transform = vtkTransformFilter::New();
  pre_transform->SetInputConnection( polyReader->GetOutputPort() );

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( pre_transform->GetOutputPort() );

  pre_transform->Delete();

  vtkTransform* t = vtkTransform::New();
  t->Scale( -1., -1., -1. );
  pre_transform->SetTransform( t );
  transform->SetTransform( t );
  t->Delete();

  transform->Update();
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(transform->GetOutput()->GetNumberOfPoints());
  s->SetName("scalars");
  vtkPointSet* data = transform->GetOutput();
  vtkIdType num = data->GetNumberOfCells() / parallel, n;
  ++num;
  vtkGenericCell* cell = vtkGenericCell::New();
  vtkIdList* cellPts;
  for ( vtkIdType i = 0; i < num; ++i )
  {
    data->GetCell( i, cell );
    cellPts = cell->GetPointIds();
    n = 0;
    while ( n != cellPts->GetNumberOfIds() )
    {
      s->SetTuple1( cellPts->GetId( n ), 1.);
      ++n;
    }
  }
  for ( vtkIdType i = num; i < data->GetNumberOfCells(); ++i )
  {
    data->GetCell( i, cell );
    cellPts = cell->GetPointIds();
    n = 0;
    while ( n != cellPts->GetNumberOfIds() )
    {
      s->SetTuple1( cellPts->GetId( n ), -1.);
      ++n;
    }
  }
  cell->Delete();
  data->GetPointData()->SetScalars( s );
  s->Delete();

  vtkContourFilter* isosurface = vtkContourFilter::New();
  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->GenerateValues( 1, 0.0, 1.0 );
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
  mapper->SetInputConnection( transform->GetOutputPort() );
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
  cout << "should exit (" << parallel << ")" << endl;
  return 0;
}
