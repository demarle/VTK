#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkUnstructuredGrid.h"

#include "vtkPointSet.h"
#include "vtkDoubleArray.h"
#include "vtkCellArray.h"
#include "vtkGenericCell.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include "vtkDataSetTriangleFilter.h"
#include "vtkSubdivideTetra.h"
#include "vtkVectorNorm.h"
#include "vtkCastToConcrete.h"
#include <string>

void SetScalarsAtTheBeginning( vtkPointSet* mesh, int size )
  {
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(mesh->GetNumberOfPoints());
  s->SetName("scalars");

  vtkIdType num = mesh->GetNumberOfCells() / size + 1, n;
  vtkGenericCell* cell = vtkGenericCell::New();

  for ( vtkIdType i = 0; i < mesh->GetNumberOfCells(); ++i )
    {
    mesh->GetCell( i, cell );
    n = cell->GetNumberOfPoints();
    int v = i < num;
    while ( n-- )
      {
      s->SetTuple1( cell->GetPointId( n ), v );
      }
    }
  cell->Delete();

  mesh->GetPointData()->SetScalars( s );
  s->Delete();
  }

void SetScalarsEverywhere( vtkPointSet* mesh )
  {
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(mesh->GetNumberOfPoints());
  s->SetName("scalars");

  double v = 0;
  for ( vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i )
    {
    s->SetTuple1( i, (v = 1 - v) );
    }
  mesh->GetPointData()->SetScalars( s );
  s->Delete();
  }

void SetScalarsPeriodicaly( vtkPointSet* mesh, int frequency )
  {
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(mesh->GetNumberOfPoints());
  s->SetName("scalars");

  double v = 0;
  const vtkIdType length = mesh->GetNumberOfPoints() / frequency;
  for ( vtkIdType i = 0, hz = length; i < mesh->GetNumberOfPoints(); ++i, --hz )
    {
    s->SetTuple1( i, v );
    if ( !hz )
      {
      v = 1 - v;
      hz = length;
      }
    }
  mesh->GetPointData()->SetScalars( s );
  s->Delete();
  }

void ColorByPointNumber( vtkPointSet* mesh )
  {
  vtkIdType number = mesh->GetNumberOfPoints();

  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(number);
  s->SetName("scalars");

  for ( vtkIdType i = 0; i < number; ++i )
    {
    s->SetTuple1( i, double(i) / number );
    }
  mesh->GetPointData()->SetScalars( s );
  s->Delete();
  }

void ColorByCellNumber( vtkPointSet* mesh )
  {
  vtkIdType number = mesh->GetNumberOfCells();

  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(number);
  s->SetName("scalars");

  for ( vtkIdType i = 0; i < number; ++i )
    {
    s->SetTuple1( i, double(i) / number );
    }
  mesh->GetCellData()->SetScalars( s );
  s->Delete();
  }

void DoNothing( vtkPointSet* data ) {}

void PolyDataWrapper( vtkPolyData* data, std::string filename, void (*fill)( vtkPointSet* ) )
  {
  vtkPolyData* mesh = vtkPolyData::New();
  mesh->DeepCopy( data );

  fill( mesh );

  vtkPolyDataWriter* polyWriter = vtkPolyDataWriter::New();
  polyWriter->SetFileTypeToBinary();
  polyWriter->SetInput( mesh );
  polyWriter->SetFileName( filename.c_str() );
  polyWriter->Write();

  polyWriter->Delete();
  mesh->Delete();

  cout << filename << " created." << endl;
  }

void PolyDataWrapperWithValue( vtkPolyData* data, int value, std::string filename, void (*fill)( vtkPointSet*, int ) )
  {
  vtkPolyData* mesh = vtkPolyData::New();
  mesh->DeepCopy( data );

  fill( mesh, value );

  vtkPolyDataWriter* polyWriter = vtkPolyDataWriter::New();
  polyWriter->SetFileTypeToBinary();
  polyWriter->SetInput( mesh );
  polyWriter->SetFileName( filename.c_str() );
  polyWriter->Write();

  polyWriter->Delete();
  mesh->Delete();

  cout << filename << " created." << endl;
  }

void UnstructuredGridWrapper( vtkUnstructuredGrid* data, std::string filename, void (*fill)( vtkPointSet* ) )
  {
  vtkUnstructuredGrid* mesh = vtkUnstructuredGrid::New();
  mesh->DeepCopy( data );

  fill( mesh );

  vtkUnstructuredGridWriter* polyWriter = vtkUnstructuredGridWriter::New();
  polyWriter->SetFileTypeToBinary();
  polyWriter->SetInput( mesh );
  polyWriter->SetFileName( filename.c_str() );
  polyWriter->Write();

  polyWriter->Delete();
  mesh->Delete();

  cout << filename << " created." << endl;
  }

void UnstructuredGridWrapperWithValue( vtkUnstructuredGrid* data, int value, std::string filename, void (*fill)( vtkPointSet*, int ) )
  {
  vtkUnstructuredGrid* mesh = vtkUnstructuredGrid::New();
  mesh->DeepCopy( data );

  fill( mesh, value );

  vtkUnstructuredGridWriter* polyWriter = vtkUnstructuredGridWriter::New();
  polyWriter->SetFileTypeToBinary();
  polyWriter->SetInput( mesh );
  polyWriter->SetFileName( filename.c_str() );
  polyWriter->Write();

  polyWriter->Delete();
  mesh->Delete();

  cout << filename << " created." << endl;
  }

int main( int argc, char** argv )
{
  if ( argc < 2 )
    {
    cout << "You must provide a file name and a folder to store meshes" << endl;
    return 1;
    }
  if ( argc < 3 )
    {
    cout << "You must provide a folder to store meshes" << endl;
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
  const std::string filepath(argv[1]);
  const std::string filename = filepath.substr( filepath.find_last_of('/') + 1 );
  const std::string file = filename.substr( 0, filename.find_last_of('.') );
  cout << "Using mesh " << file << endl;

  std::string storage_folder( argv[2] );
  cout << "Storage into «" << storage_folder << "»" << endl;
  if ( storage_folder[storage_folder.length() - 1] != '/' )
    {
    storage_folder += "/";
    }

  /* === Computing scalars === */
  if ( !usgReader )
    {
    polyReader->Update();
    vtkPolyData* data = polyReader->GetOutput();
    PolyDataWrapper( data, storage_folder + file + ".vtk", DoNothing );

    PolyDataWrapperWithValue( data, 8, storage_folder + file + "-first8.vtk", SetScalarsAtTheBeginning );
    PolyDataWrapperWithValue( data, 48, storage_folder + file + "-first48.vtk", SetScalarsAtTheBeginning );
    PolyDataWrapper( data, storage_folder + file + "-full.vtk", SetScalarsEverywhere );
    PolyDataWrapperWithValue( data, 10, storage_folder + file + "-10pieces.vtk", SetScalarsPeriodicaly );
    PolyDataWrapperWithValue( data, 50, storage_folder + file + "-50pieces.vtk", SetScalarsPeriodicaly );
    PolyDataWrapperWithValue( data, 100, storage_folder + file + "-100pieces.vtk", SetScalarsPeriodicaly );

    PolyDataWrapper( data, storage_folder + file + "-points_mapping.vtk", ColorByPointNumber );
    PolyDataWrapper( data, storage_folder + file + "-cells_mapping.vtk", ColorByCellNumber );

    polyReader->Delete();
    }
  else
    {
    usgReader->Update();
    vtkUnstructuredGrid* data = vtkUnstructuredGrid::New();
    data->DeepCopy( usgReader->GetOutput() );
    usgReader->Delete();

    if (data->GetPointData()->GetScalars() == 0 )
      {
      vtkCastToConcrete* ctc = vtkCastToConcrete::New();
      vtkVectorNorm* normaliser = vtkVectorNorm::New();
//      data->GetCellData()->SetScalars(0);
      normaliser->SetInput( data );
      normaliser->SetNormalize(1);
      ctc->SetInputConnection( normaliser->GetOutputPort() );
      normaliser->Delete();
      ctc->Update();
      UnstructuredGridWrapper( vtkUnstructuredGrid::SafeDownCast(ctc->GetOutput()), storage_folder + file + ".vtk", DoNothing );
      ctc->Delete();
      }
    else
      {
      char ext[4];
      sprintf(ext, "");
      vtkSubdivideTetra* tetra;
      int size = 1;
      while ( size < 65 )
        {
        UnstructuredGridWrapper( data, storage_folder + file + ext + ".vtk", DoNothing );
        UnstructuredGridWrapper( data, storage_folder + file + ext + "-points_mapping.vtk", ColorByPointNumber );
        UnstructuredGridWrapper( data, storage_folder + file + ext + "-cells_mapping.vtk", ColorByCellNumber );
        tetra = vtkSubdivideTetra::New();
        tetra->SetInput( data );
        tetra->Update();
        data->DeepCopy( tetra->GetOutput() );
        tetra->Delete();
        size *= 4;
        sprintf(ext, "%dx", size);
        }
      }
      data->Delete();
    }

  return 0;
}
