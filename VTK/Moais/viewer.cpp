#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"

#include "vtkPointData.h"
#include "vtkCellData.h"

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

  /* === Printing mesh informations === */
  if ( !usgReader )
    {
    polyReader->Update();
    polyReader->GetOutput()->Print( cout );
    polyReader->Delete();
    }
  else
    {
    usgReader->Update();
    usgReader->GetOutput()->Print( cout );
    usgReader->GetOutput()->GetPointData()->GetScalars()->Print( cout );
    usgReader->Delete();
    }
}
