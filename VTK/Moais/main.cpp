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

#include "vtkEnSightWriter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"
#include "vtkAppendFilter.h"
#include "vtkUnsignedIntArray.h"
#include "vtkCellData.h"

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

  vtkAppendFilter* filter = vtkAppendFilter::New();
  filter->AddInputConnection(polyReader->GetOutputPort());
  polyReader->Delete();

  // Adding BlockID (BlockID = 1 for all cells)
  filter->Update();
  vtkUnstructuredGrid* usd = filter->GetOutput();
  vtkIdType number_cells = usd->GetNumberOfCells();
  vtkIdType blockSize = vtkIdType(number_cells / parallel);
  ++blockSize;

  vtkUnsignedIntArray* blockIDs = vtkUnsignedIntArray::New();
  blockIDs->SetNumberOfTuples(number_cells);
  blockIDs->SetNumberOfComponents(1);
  blockIDs->SetName("BlockId");
  vtkDataArray* s = vtkDoubleArray::New();
  s->SetNumberOfComponents(1);
  s->SetNumberOfTuples(usd->GetNumberOfPoints());
  s->SetName("scalars");
  vtkIdType n;
  vtkGenericCell* cell = vtkGenericCell::New();
  vtkIdList* cellPts;
  for ( vtkIdType i = 0; i < blockSize; ++i )
  {
    blockIDs->SetValue ( i, 1 );
    usd->GetCell( i, cell );
    cellPts = cell->GetPointIds();
    n = 0;
    while ( n != cellPts->GetNumberOfIds() )
    {
      s->SetTuple1( cellPts->GetId( n ), 1.);
      ++n;
    }
  }
  for ( vtkIdType i = blockSize; i < number_cells; ++i )
  {
    blockIDs->SetValue ( i, 1 + (i/blockSize) );
    usd->GetCell( i, cell );
    cellPts = cell->GetPointIds();
    n = 0;
    while ( n != cellPts->GetNumberOfIds() )
    {
      s->SetTuple1( cellPts->GetId( n ), -1.);
      ++n;
    }
  }
  cell->Delete();
  usd->GetPointData()->SetScalars( s );
  s->Delete();
  usd->GetCellData()->AddArray(blockIDs);
  blockIDs->Delete();

  vtkEnSightWriter* writer = vtkEnSightWriter::New();
  writer->SetFileName(argv[1]);
  writer->SetInputConnection(filter->GetOutputPort());
  writer->SetNumberOfBlocks(8);
  writer->Write();
  writer->WriteCaseFile(1);
}
