#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkTransformFilter.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPTransform.h"
  #include "vtkSMPContourFilter.h"
  #include "vtkSMPMergePoints.h"
  #include "vtkSMPMinMaxTree.h"
#else
  #include "vtkContourFilter.h"
  #include "vtkSimpleScalarTree.h"
  #include "vtkTransform.h"
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
  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( usgReader ? usgReader->GetOutputPort() : polyReader->GetOutputPort() );
#ifdef VTK_CAN_USE_SMP
  vtkSMPTransform* t = vtkSMPTransform::New();
#else
  vtkTransform* t = vtkTransform::New();
#endif
  t->Identity();
  transform->SetTransform( t );
  t->Delete();

  /* === Testing contour filter === */
#ifdef VTK_CAN_USE_SMP
  vtkContourFilter* isosurface = vtkSMPContourFilter::New();
  vtkSMPMergePoints* locator = vtkSMPMergePoints::New();
  isosurface->SetLocator( locator );
  locator->Delete();
  vtkSMPMinMaxTree* tree = vtkSMPMinMaxTree::New();
#else
  vtkContourFilter* isosurface = vtkContourFilter::New();
  vtkSimpleScalarTree* tree = vtkSimpleScalarTree::New();
#endif
  isosurface->SetScalarTree(tree);
  tree->Delete();
  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->GenerateValues( 11, 0.0, 1.0 );
  isosurface->UseScalarTreeOn();
  transform->Delete();

  /* === Pipeline pull === */
#ifndef HIDE_VTK_WINDOW
  vtkPolyDataMapper* map = vtkPolyDataMapper::New();
  map->SetInputConnection( isosurface->GetOutputPort() );
  isosurface->Delete();

  vtkActor* object = vtkActor::New();
  object->SetMapper( map );
  map->Delete();

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );
  viewport->AddActor( object );
  object->Delete();

  vtkRenderWindow* window = vtkRenderWindow::New();
  window->AddRenderer( viewport );
  window->SetWindowName(VTK_WINDOW_NAME);
  viewport->Delete();

  vtkRenderWindowInteractor* eventsCatcher = vtkRenderWindowInteractor::New();
  eventsCatcher->SetRenderWindow( window );
  window->Delete();

  eventsCatcher->Initialize();
#endif

  int bf = 2;
  while ( bf < 10 )
    {
    cout << endl << "Branching factor set to " << bf;
    tree->SetBranchingFactor( bf );
    isosurface->Modified();
#ifdef HIDE_VTK_WINDOW
    isosurface->Update();
#else
    eventsCatcher->Render();
    eventsCatcher->Start();
#endif
    cout << "Tree level was " << tree->GetLevel() << endl;
    if ( bf < 4 ) ++bf; else bf *= 2;
    }
#ifdef HIDE_VTK_WINDOW
  isosurface->Delete();
#else
  eventsCatcher->Delete();
#endif

  return 0;
  }
