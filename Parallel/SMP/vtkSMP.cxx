#include "vtkSMP.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDummyMergeFunctor.h"
#include "vtkLockPointMerger.h"
#include "vtkOffsetManager.h"
#include "vtkParallelCellMerger.h"
#include "vtkParallelPointMerger.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSMPMergePoints.h"
#include "vtkThreadLocal.h"

typedef vtkIdType *vtkIdTypePtr;

vtkIdType** TreatedTable = 0;

//------------------------------------------------------------------------------
void vtkSMPMergePointsOp(vtkSMPMergePoints *outPoints,
                         vtkThreadLocal<vtkSMPMergePoints> *inPoints,
                         vtkPointData *outPtsData,
                         vtkThreadLocal<vtkPointData> *inPtsData,
                         vtkCellArray *outVerts,
                         vtkThreadLocal<vtkCellArray> *inVerts,
                         vtkCellArray *outLines,
                         vtkThreadLocal<vtkCellArray> *inLines,
                         vtkCellArray *outPolys,
                         vtkThreadLocal<vtkCellArray> *inPolys,
                         vtkCellArray *outStrips,
                         vtkThreadLocal<vtkCellArray> *inStrips,
                         vtkCellData *outCellsData,
                         vtkThreadLocal<vtkCellData> *inCellsData,
                         int SkipThreads)
{
  TreatedTable = new vtkIdTypePtr[outPoints->GetNumberOfBuckets()];
  memset( TreatedTable, 0, outPoints->GetNumberOfBuckets() * sizeof(vtkIdTypePtr) );

  vtkDummyMergeFunctor* DummyFunctor = vtkDummyMergeFunctor::New();
  DummyFunctor->InitializeNeeds( inPoints, 0, outPoints,
                                 inVerts, outVerts,
                                 inLines, outLines,
                                 inPolys, outPolys,
                                 inStrips, outStrips,
                                 inPtsData, outPtsData,
                                 inCellsData, outCellsData );

  vtkParallelPointMerger* TheMerge = vtkParallelPointMerger::New();
  TheMerge->self = DummyFunctor;
  vtkSMPParallelOp<vtkSMPMergePoints>( TheMerge, DummyFunctor->Locators->Begin(), SkipThreads );
  TheMerge->Delete();

  vtkParallelCellMerger* TheCellMerge = vtkParallelCellMerger::New();
  TheCellMerge->self = DummyFunctor;
  vtkSMPParallelOp<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>
    (
     TheCellMerge,
     DummyFunctor->Maps->Begin(),
     DummyFunctor->InCd->Begin(),
     DummyFunctor->InVerts->Begin(),
     DummyFunctor->InLines->Begin(),
     DummyFunctor->InPolys->Begin(),
     DummyFunctor->InStrips->Begin(),
     DummyFunctor->vertOffset->GetCellsOffset(),
     DummyFunctor->vertOffset->GetTuplesOffset(),
     DummyFunctor->lineOffset->GetCellsOffset(),
     DummyFunctor->lineOffset->GetTuplesOffset(),
     DummyFunctor->polyOffset->GetCellsOffset(),
     DummyFunctor->polyOffset->GetTuplesOffset(),
     DummyFunctor->stripOffset->GetCellsOffset(),
     DummyFunctor->stripOffset->GetTuplesOffset(),
     SkipThreads );
  TheCellMerge->Delete();

  // Correcting size of arrays
  outPoints->FixSizeOfPointArray();
  outPtsData->SetNumberOfTuples( outPoints->GetPoints()->GetNumberOfPoints() );
  outCellsData->SetNumberOfTuples( DummyFunctor->GetNumberOfCells() );
  if (outVerts) outVerts->SetNumberOfCells( DummyFunctor->vertOffset->GetNumberOfCells() );
  if (outLines) outLines->SetNumberOfCells( DummyFunctor->lineOffset->GetNumberOfCells() );
  if (outPolys) outPolys->SetNumberOfCells( DummyFunctor->polyOffset->GetNumberOfCells() );
  if (outStrips) outStrips->SetNumberOfCells( DummyFunctor->stripOffset->GetNumberOfCells() );

  delete [] TreatedTable;
  TreatedTable = 0;
  DummyFunctor->Delete();
}

//------------------------------------------------------------------------------
void vtkSMPMergePointsOp(vtkPoints *outPoints,
                         vtkThreadLocal<vtkPoints> *inPoints,
                         const double bounds[6],
                         vtkPointData *outPtsData,
                         vtkThreadLocal<vtkPointData> *inPtsData,
                         vtkCellArray *outVerts,
                         vtkThreadLocal<vtkCellArray> *inVerts,
                         vtkCellArray *outLines,
                         vtkThreadLocal<vtkCellArray> *inLines,
                         vtkCellArray *outPolys,
                         vtkThreadLocal<vtkCellArray> *inPolys,
                         vtkCellArray *outStrips,
                         vtkThreadLocal<vtkCellArray> *inStrips,
                         vtkCellData *outCellsData,
                         vtkThreadLocal<vtkCellData> *inCellsData,
                         int SkipThreads)
{
  vtkDummyMergeFunctor* Functor = vtkDummyMergeFunctor::New();
  vtkSMPMergePoints* outputLocator = vtkSMPMergePoints::New();
  vtkIdType NumberOfInPointsThread0 = (*(inPoints->Begin( 0 )))->GetNumberOfPoints();
  outputLocator->InitLockInsertion( outPoints, bounds, NumberOfInPointsThread0 );
  Functor->outputLocator = outputLocator;

  vtkIdType PointsAlreadyPresent = outPoints->GetNumberOfPoints();
  if ( PointsAlreadyPresent ) vtkSMPForEachOp( 0, PointsAlreadyPresent, Functor );

  Functor->InitializeNeeds( 0, inPoints, outputLocator,
                            inVerts, outVerts,
                            inLines, outLines,
                            inPolys, outPolys,
                            inStrips, outStrips,
                            inPtsData, outPtsData,
                            inCellsData, outCellsData );

  vtkIdType StartPoint = SkipThreads ? NumberOfInPointsThread0 : 0;
  vtkLockPointMerger* TheMerge = vtkLockPointMerger::New();
  TheMerge->Functor = Functor;
  TheMerge->NumberOfPointsFirstThread = NumberOfInPointsThread0;
  vtkSMPForEachOp( StartPoint, Functor->GetNumberOfPoints(), TheMerge );
  TheMerge->Delete();

  vtkParallelCellMerger* TheCellMerge = vtkParallelCellMerger::New();
  TheCellMerge->self = Functor;
  vtkSMPParallelOp<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>
    (
     TheCellMerge,
     Functor->Maps->Begin(),
     Functor->InCd->Begin(),
     Functor->InVerts->Begin(),
     Functor->InLines->Begin(),
     Functor->InPolys->Begin(),
     Functor->InStrips->Begin(),
     Functor->vertOffset->GetCellsOffset(),
     Functor->vertOffset->GetTuplesOffset(),
     Functor->lineOffset->GetCellsOffset(),
     Functor->lineOffset->GetTuplesOffset(),
     Functor->polyOffset->GetCellsOffset(),
     Functor->polyOffset->GetTuplesOffset(),
     Functor->stripOffset->GetCellsOffset(),
     Functor->stripOffset->GetTuplesOffset(),
     SkipThreads );
  TheCellMerge->Delete();

  // Correcting size of arrays
  Functor->outputLocator->FixSizeOfPointArray();
  outPtsData->SetNumberOfTuples( outPoints->GetNumberOfPoints() );
  outCellsData->SetNumberOfTuples( Functor->GetNumberOfCells() );
  if (outVerts) outVerts->SetNumberOfCells( Functor->vertOffset->GetNumberOfCells() );
  if (outLines) outLines->SetNumberOfCells( Functor->lineOffset->GetNumberOfCells() );
  if (outPolys) outPolys->SetNumberOfCells( Functor->polyOffset->GetNumberOfCells() );
  if (outStrips) outStrips->SetNumberOfCells( Functor->stripOffset->GetNumberOfCells() );

  outputLocator->Delete();
  Functor->Delete();
}
