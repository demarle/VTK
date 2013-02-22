#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkUnstructuredGrid.h"

#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkSMPZCurve.h"

#include <cstdlib>

int main( int argc, char** argv )
{
  if ( argc < 2 )
  {
    cout << "You must provide a file name" << endl;
    return 1;
  }
  const std::string filename(argv[1]);
  const std::string file = filename.substr( 0, filename.find_last_of('.') ) + "-zcurve.vtk";

  /* === Reading 3D model === */
  vtkUnstructuredGridReader* usgReader = 0;
  vtkPolyDataReader* polyReader = vtkPolyDataReader::New();
  polyReader->SetFileName(argv[1]);
  if ( !polyReader->IsFilePolyData() )
    {
    usgReader = vtkUnstructuredGridReader::New();
    usgReader->SetFileName( filename.c_str() );
    polyReader->CloseVTKFile();
    polyReader->Delete();
    polyReader = 0;
    if ( !usgReader->IsFileUnstructuredGrid() )
      {
      usgReader->Delete();
      cout << filename << " is not a suitable file" << endl;
      return 1;
      }
    }
  cout << "Using file " << filename << endl;

  /* === Applying ZCurve === */
  vtkSMPZCurve* zcurve = vtkSMPZCurve::New();
  zcurve->SetInputConnection( usgReader ? usgReader->GetOutputPort() : polyReader->GetOutputPort() );

  /* === Printing mesh informations === */
  if ( !usgReader )
    {
    polyReader->Delete();
    vtkPolyDataWriter* polyWriter = vtkPolyDataWriter::New();
    polyWriter->SetInputConnection( zcurve->GetOutputPort() );
    polyWriter->SetFileTypeToBinary();
    polyWriter->SetFileName( file.c_str() );
    polyWriter->Update();
    polyWriter->Write();
    polyWriter->Delete();
    }
  else
    {
    usgReader->Delete();
    vtkUnstructuredGridWriter* usgWriter = vtkUnstructuredGridWriter::New();
    usgWriter->SetInputConnection( zcurve->GetOutputPort() );
    usgWriter->SetFileTypeToBinary();
    usgWriter->SetFileName( file.c_str() );
    usgWriter->Update();
    usgWriter->Write();
    usgWriter->Delete();
    }

  zcurve->GetOutput()->Print( cout );
  zcurve->Delete();
}
