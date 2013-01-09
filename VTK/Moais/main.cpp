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
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPTransform.h"
  #include "vtkSMPContourFilter.h"
  #include "vtkSMPMergePoints.h"
  #include "vtkSMPMinMaxTree.h"
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
  vtkSMPTransform* t = vtkSMPTransform::New();
#else
  vtkTransform* t = vtkTransform::New();
#endif
  t->Identity();
  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetTransform( t );
  t->Delete();
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
  eventsCatcher->Start();
#else
  isosurface->Update();
#endif

  /* === New computation with search tree === */
  isosurface->UseScalarTreeOn();
  isosurface->Modified();

#ifdef HIDE_VTK_WINDOW
  isosurface->Update();
  isosurface->Delete();
#else
  eventsCatcher->Render();
  eventsCatcher->Start();
  eventsCatcher->Delete();
#endif
  return 0;
  }
