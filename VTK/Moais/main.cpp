#include "vtkTransform.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPTransform.h"
  #include "vtkSMPTransformFilter.h"
  #include "vtkSMPContourFilter.h"
  #include "vtkSMPMergePoints.h"
  #include "vtkSMPMinMaxTree.h"
#else
  #include "vtkTransformFilter.h"
  #include "vtkContourFilter.h"
#endif

#include "vtkCellArray.h"
#include "vtkGenericCell.h"
#include <cstdlib>

int main( int argc, char** argv )
{
  if ( argc < 2 )
  {
    cout << "You must provide a file name" << endl;
    return 1;
  }
  if ( ifstream(argv[1]) )
  {
    cout << "Using file " << argv[1] << endl;
  }
  else
  {
    cout << argv[1] << " is not a regular file" << endl;
    return 1;
  }
  int parallel = argc == 2 ? 48 : atoi(argv[2]);

  /* === Reading 3D model === */

  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName(argv[1]);

  /* === Testing transform filter === */

  vtkTransformFilter* pre_transform = vtkTransformFilter::New();
  pre_transform->SetInputConnection( polyReader->GetOutputPort() );

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( pre_transform->GetOutputPort() );

  pre_transform->Delete();

#ifdef VTK_CAN_USE_SMP
  if ( parallel != 1 )
  {
    vtkSMPTransform* t = vtkSMPTransform::New();
    t->Scale( -1., -1., -1. );
    pre_transform->SetTransform( t );
    transform->SetTransform( t );
    t->Delete();
  }
  else
  {
#endif
    vtkTransform* t = vtkTransform::New();
    t->Scale( -1., -1., -1. );
    pre_transform->SetTransform( t );
    transform->SetTransform( t );
    t->Delete();
#ifdef VTK_CAN_USE_SMP
  }
#endif

  /* === Testing contour filter === */
  transform->Update();
  polyReader->Update();
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(transform->GetOutput()->GetNumberOfPoints());
  s->SetName("scalars");
  vtkPointSet* data = transform->GetOutput();
  vtkIdType num = data->GetNumberOfPoints();

  double coord[3];
  double v = 0;
  for ( vtkIdType i = 0; i < num; ++i )
  {
    data->GetPoint( i, coord );
    s->SetTuple1( i, (v = - ( v - 1 )) );
  }
  data->GetPointData()->SetScalars( s );
  s->Delete();

#ifdef VTK_CAN_USE_SMP
  vtkContourFilter* isosurface = parallel != 1 ? vtkSMPContourFilter::New() : vtkContourFilter::New();
  vtkSMPMergePoints* locator = vtkSMPMergePoints::New();
  isosurface->SetLocator( locator );
  locator->Delete();
  vtkSMPMinMaxTree* tree = vtkSMPMinMaxTree::New();
  isosurface->SetScalarTree(tree);
  tree->Delete();
#else
  vtkContourFilter* isosurface = vtkContourFilter::New();
#endif
  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->GenerateValues( 11, 0.0, 1.0 );
  isosurface->UseScalarTreeOn();
  transform->Delete();

#ifdef HIDE_VTK_WINDOW
  // Simulate a call to vtkRenderWindow::Render()
  polyReader->Delete();
  isosurface->Update();
  isosurface->Delete();
#else
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
  window->SetWindowName(VTK_WINDOW_NAME);
  viewport->Delete();

  vtkRenderWindowInteractor* eventsCatcher = vtkRenderWindowInteractor::New();
  eventsCatcher->SetRenderWindow( window );
  window->Delete();

  eventsCatcher->Initialize();
  eventsCatcher->Start();

  eventsCatcher->Delete();
#endif
  cout << "should exit (" << parallel << ")" << endl;
  return 0;
}
