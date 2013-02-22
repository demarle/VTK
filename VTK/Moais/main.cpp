#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAxesActor.h"

#include "vtkTransformFilter.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPTransform.h"
  #include "vtkSMPContourFilter.h"
  #include "vtkSMPMergePoints.h"
  #include "vtkSMPMinMaxTree.h"
  #include "vtkSMPZCurve.h"
#else
  #include "vtkTransform.h"
  #include "vtkContourFilter.h"
  #include "vtkSimpleScalarTree.h"
#endif

int main( int argc, char** argv )
  {
  if ( argc < 2 )
    {
    cout << "You must provide a file name" << endl;
    return 1;
    }

  /* === Reading 3D model === */
  vtkUnstructuredGridReader* usgReader = 0;
  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName(argv[1]);
  if ( !polyReader->IsFilePolyData() )
    {
    usgReader = vtkUnstructuredGridReader::New();
    usgReader->SetFileName(argv[1]);
    polyReader->CloseVTKFile();
    polyReader->Delete();
    polyReader = 0;
    if ( !usgReader->IsFileUnstructuredGrid() )
      {
      usgReader->Delete();
      cout << argv[1] << " is not a suitable file" << endl;
      return 1;
      }
    }
  cout << "Using file " << argv[1] << endl;

  /* === Distributing data === */
#ifdef VTK_CAN_USE_SMP
  vtkSMPZCurve* transform = vtkSMPZCurve::New();
#else
  vtkTransform* t = vtkTransform::New();
  t->Identity();
  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetTransform( t );
  t->Delete();
#endif
  transform->SetInputConnection( usgReader ? usgReader->GetOutputPort() : polyReader->GetOutputPort() );
  if (usgReader)
    usgReader->Delete();
  else
    polyReader->Delete();

  /* === Testing contour filter === */
#ifdef VTK_CAN_USE_SMP
  vtkSMPContourFilter* isosurface = vtkSMPContourFilter::New();
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
  transform->Delete();
  isosurface->GenerateValues( 11, 0.0, 1.0 );
  isosurface->UseScalarTreeOff();

  /* === Pipeline pull === */
#ifndef HIDE_VTK_WINDOW
  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection( transform->GetOutputPort() );
  vtkActor* reference = vtkActor::New();
  reference->SetMapper( mapper );
  mapper->Delete();
  double bounds[6];
  mapper->GetBounds(bounds);
  reference->SetPosition( bounds[1], bounds[2], bounds[4] );

  vtkIdType n;
  vtkPointSet* tmp;
  vtkDataArray* scalars;

  vtkPolyDataMapper* map = vtkPolyDataMapper::New();
  map->SetInputConnection( isosurface->GetOutputPort() );
  isosurface->Delete();

  vtkActor* object = vtkActor::New();
  object->SetMapper( map );
  map->Delete();
  object->SetPosition( bounds[0], bounds[2], bounds[4]);

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );
  viewport->AddActor( object );
  object->Delete();
  viewport->AddActor( reference );
  reference->Delete();
  /* === Visualisation of points mapping === */
  transform->Update();
  tmp = transform->GetOutput()->NewInstance();
  tmp->DeepCopy( transform->GetOutput() );
  n = tmp->GetNumberOfPoints();
  scalars = tmp->GetPointData()->GetScalars();
  for ( vtkIdType i = 0; i < n; ++i )
    {
    scalars->SetTuple1( i, double(i)/n);
    }
  vtkPolyDataMapper* ptmapper = vtkPolyDataMapper::New();
  ptmapper->SetInput( vtkPolyData::SafeDownCast(tmp) );
  vtkActor* ptactor = vtkActor::New();
  ptactor->SetMapper( ptmapper );
  ptmapper->Delete();
  tmp->Delete();
  ptactor->SetPosition( bounds[1]*2 - bounds[0], bounds[2], bounds[4] );
  viewport->AddActor( ptactor );
  ptactor->Delete();
  /* */
  /* === Visualisation of cells mapping === */
  transform->Update();
  tmp = transform->GetOutput()->NewInstance();
  tmp->DeepCopy( transform->GetOutput() );
  n = tmp->GetNumberOfCells();
  scalars = vtkDoubleArray::New();
  scalars->SetNumberOfComponents( 1 );
  scalars->Allocate( n, n );
  scalars->SetNumberOfTuples( n );
  for ( vtkIdType i = 0; i < n; ++i )
    {
    scalars->SetTuple1( i, double(i)/n);
    }
  tmp->GetCellData()->SetScalars( scalars );
  scalars->Delete();
  tmp->GetPointData()->SetScalars(NULL);
  vtkPolyDataMapper* cellmapper = vtkPolyDataMapper::New();
  cellmapper->SetInput( vtkPolyData::SafeDownCast(tmp) );
  vtkActor* cellactor = vtkActor::New();
  cellactor->SetMapper( cellmapper );
  cellmapper->Delete();
  tmp->Delete();
  cellactor->SetPosition( bounds[1]*3 - bounds[0]*2, bounds[2], bounds[4] );
  viewport->AddActor( cellactor );
  cellactor->Delete();
  /* */

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
#else
  isosurface->Update();
  isosurface->Delete();
#endif

  return 0;
  }
