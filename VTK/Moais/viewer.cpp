#include "vtkPolyDataReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGridVolumeRayCastMapper.h"
#include "vtkActor.h"
#include "vtkVolume.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include <cstdlib>

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

  /* === Building viewport === */
  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );

  if ( !usgReader )
    {
    vtkPolyDataMapper* map = vtkPolyDataMapper::New();
    map->SetInputConnection( polyReader->GetOutputPort() );
    polyReader->Delete();

    vtkActor* object = vtkActor::New();
    object->SetMapper( map );
    map->Delete();

    viewport->AddActor( object );
    object->Delete();
    }
  else
    {
    vtkUnstructuredGridVolumeRayCastMapper* map = vtkUnstructuredGridVolumeRayCastMapper::New();
    map->SetInputConnection( usgReader->GetOutputPort() );
    map->SetNumberOfThreads( 8 );
    usgReader->Delete();

    vtkVolume* object = vtkVolume::New();
    object->SetMapper( map );
    map->Delete();

    viewport->AddActor( object );
    object->Delete();
    }


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

  return 0;
}
