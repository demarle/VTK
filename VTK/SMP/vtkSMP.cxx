#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkSMPMergePoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"

//--------------------------------------------------------------------------------
vtkFunctor::vtkFunctor() { }

vtkFunctor::~vtkFunctor() { }

void vtkFunctor::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  }

//--------------------------------------------------------------------------------
vtkFunctorInitialisable::vtkFunctorInitialisable() : vtkFunctor()
  {
  IsInitialized = false;
  }

vtkFunctorInitialisable::~vtkFunctorInitialisable() { }

bool vtkFunctorInitialisable::CheckAndSetInitialized() const
  {
  bool ret = IsInitialized;
  IsInitialized = true;
  return ret;
  }

void vtkFunctorInitialisable::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Is initialized: " << IsInitialized << endl;
  }

//--------------------------------------------------------------------------------
vtkSMPCommand::vtkSMPCommand() { }

vtkSMPCommand::~vtkSMPCommand() { }

void vtkSMPCommand::Execute(vtkObject *caller, unsigned long eventId, void *callData) { }

void vtkSMPCommand::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

//--------------------------------------------------------------------------------
class OffsetManager : public vtkObject
{
  OffsetManager( const OffsetManager& );
  void operator =( const OffsetManager& );
  vtkstd::vector<vtkIdType> cells;
  vtkstd::vector<vtkIdType> tuples;
  vtkIdType CellsOffset;
  vtkIdType TuplesOffset;

protected:
  OffsetManager() : vtkObject(), cells(vtkSMP::GetNumberOfThreads(), 0), tuples(vtkSMP::GetNumberOfThreads(), 0)
    {
    CellsOffset = 0;
    TuplesOffset = 0;
    }
  ~OffsetManager() { }
public:
  vtkTypeMacro(OffsetManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent)
    {
    this->Superclass::PrintSelf( os, indent );
    os << indent << "Offsets for cells index: " << endl;
    for ( size_t i; i < cells.size(); ++i )
      {
      os << indent.GetNextIndent() << "Thread " << i << ": " << cells[i] << endl;
      }
    os << indent << "Offsets for tuples: " << endl;
    for ( size_t i; i < tuples.size(); ++i )
      {
      os << indent.GetNextIndent() << "Thread " << i << ": " << tuples[i] << endl;
      }
    os << indent << "Number of cells: " << CellsOffset << endl;
    os << indent << "Number of tuples: " << TuplesOffset << endl;
    }
  static OffsetManager* New();

  void ManageValues ( vtkSMPThreadID tid, vtkCellArray* ca )
    {
    vtkIdType c = ca->GetNumberOfCells();
    vtkIdType t = ca->GetData()->GetNumberOfTuples();
    cells[tid] = CellsOffset;
    tuples[tid] = TuplesOffset;
    CellsOffset += c;
    TuplesOffset += t;
    }

  vtkIdType GetNumberOfCells() { return CellsOffset; }
  vtkIdType GetNumberOfTuples() { return TuplesOffset; }
  vtkIdType GetCellsOffset ( vtkSMPThreadID tid ) { return cells[tid]; }
  vtkIdType GetTuplesOffset ( vtkSMPThreadID tid ) { return tuples[tid]; }
};

vtkStandardNewMacro(OffsetManager);

class DummyMergeFunctor : public vtkFunctor
{
  DummyMergeFunctor ( const DummyMergeFunctor& );
  void operator =( const DummyMergeFunctor& );

protected:
  vtkIdType NumberOfCells;

  DummyMergeFunctor ()
    {
    vertOffset = OffsetManager::New();
    lineOffset = OffsetManager::New();
    polyOffset = OffsetManager::New();
    stripOffset = OffsetManager::New();

    vertOffset->Register( this );
    lineOffset->Register( this );
    polyOffset->Register( this );
    stripOffset->Register( this );

    vertOffset->Delete();
    lineOffset->Delete();
    polyOffset->Delete();
    stripOffset->Delete();

    Maps = vtkSMP::vtkThreadLocal<vtkIdList>::New();
    Maps->Register( this );
    Maps->Delete();

    Locators = 0;
    InPoints = 0;
    outputLocator = 0;
    }
  ~DummyMergeFunctor ()
    {
    vertOffset->UnRegister( this );
    lineOffset->UnRegister( this );
    polyOffset->UnRegister( this );
    stripOffset->UnRegister( this );

    Maps->UnRegister( this );
    }

public:
  vtkTypeMacro(DummyMergeFunctor,vtkFunctor);
  static DummyMergeFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  vtkSMP::vtkThreadLocal<vtkSMPMergePoints>* Locators;
  vtkSMP::vtkThreadLocal<vtkPoints>* InPoints;

  vtkSMP::vtkThreadLocal<vtkPointData>* InPd;
  vtkSMP::vtkThreadLocal<vtkCellData>* InCd;

  vtkSMP::vtkThreadLocal<vtkIdList>* Maps;

  vtkSMP::vtkThreadLocal<vtkCellArray>* InVerts;
  vtkSMP::vtkThreadLocal<vtkCellArray>* InLines;
  vtkSMP::vtkThreadLocal<vtkCellArray>* InPolys;
  vtkSMP::vtkThreadLocal<vtkCellArray>* InStrips;

  vtkSMPMergePoints* outputLocator;
  vtkCellArray* outputVerts;
  vtkCellArray* outputLines;
  vtkCellArray* outputPolys;
  vtkCellArray* outputStrips;
  vtkCellData* outputCd;
  vtkPointData* outputPd;

  OffsetManager* vertOffset;
  OffsetManager* lineOffset;
  OffsetManager* polyOffset;
  OffsetManager* stripOffset;

  void operator ()( vtkIdType pointId, vtkSMPThreadID tid ) const
    {
    outputLocator->AddPointIdInBucket( pointId );
    }

  vtkIdType GetNumberOfCells() const { return NumberOfCells; }

  void InitializeNeeds( vtkSMP::vtkThreadLocal<vtkSMPMergePoints>* _locator,
                        vtkSMP::vtkThreadLocal<vtkPoints>* _points,
                        vtkSMPMergePoints* _outlocator,
                        vtkSMP::vtkThreadLocal<vtkCellArray>* _inverts,
                        vtkCellArray* _outverts,
                        vtkSMP::vtkThreadLocal<vtkCellArray>* _inlines,
                        vtkCellArray* _outlines,
                        vtkSMP::vtkThreadLocal<vtkCellArray>* _inpolys,
                        vtkCellArray* _outpolys,
                        vtkSMP::vtkThreadLocal<vtkCellArray>* _instrips,
                        vtkCellArray* _outstrips,
                        vtkSMP::vtkThreadLocal<vtkPointData>* _inpd,
                        vtkPointData* _outpd,
                        vtkSMP::vtkThreadLocal<vtkCellData>* _incd,
                        vtkCellData* _outcd )
    {
    Locators = _locator;
    InPoints = _points;
    outputLocator = _outlocator;

    InVerts = _inverts;
    outputVerts = _outverts;
    InLines = _inlines;
    outputLines = _outlines;
    InPolys = _inpolys;
    outputPolys = _outpolys;
    InStrips = _instrips;
    outputStrips = _outstrips;

    InPd = _inpd;
    outputPd = _outpd;

    InCd = _incd;
    outputCd = _outcd;

    vtkIdType NumberOfPoints = 0;
    vtkSMPThreadID max = vtkSMP::GetNumberOfThreads();
    for ( vtkSMPThreadID tid = 0; tid < max; ++tid )
      {
      if ( this->InVerts ) vertOffset->ManageValues( tid, this->InVerts->GetLocal( tid ) );
      if ( this->InLines ) lineOffset->ManageValues( tid, this->InLines->GetLocal( tid ) );
      if ( this->InPolys ) polyOffset->ManageValues( tid, this->InPolys->GetLocal( tid ) );
      if ( this->InStrips ) stripOffset->ManageValues( tid, this->InStrips->GetLocal( tid ) );

      vtkIdType n = (_points ? this->InPoints->GetLocal( tid ) : this->Locators->GetLocal( tid )->GetPoints())->GetNumberOfPoints();
      vtkIdList* map = this->Maps->NewLocal( tid );
      map->Allocate( n );
      NumberOfPoints += n;
      }

    NumberOfCells = vertOffset->GetNumberOfCells() +
        lineOffset->GetNumberOfCells() +
        polyOffset->GetNumberOfCells() +
        stripOffset->GetNumberOfCells();

    if ( this->outputVerts ) this->outputVerts->GetData()->Resize( vertOffset->GetNumberOfTuples() );
    if ( this->outputLines ) this->outputLines->GetData()->Resize( lineOffset->GetNumberOfTuples() );
    if ( this->outputPolys ) this->outputPolys->GetData()->Resize( polyOffset->GetNumberOfTuples() );
    if ( this->outputStrips ) this->outputStrips->GetData()->Resize( stripOffset->GetNumberOfTuples() );
    // Copy on itself means resize
    this->outputPd->CopyAllocate( this->outputPd, NumberOfPoints, NumberOfPoints );
    this->outputCd->CopyAllocate( this->outputCd, NumberOfCells, NumberOfCells );
    outputLocator->GetPoints()->GetData()->Resize( NumberOfPoints );
    outputLocator->GetPoints()->SetNumberOfPoints( NumberOfPoints );
    }

  void CellsMerge ( vtkSMPThreadID tid ) const
    {
    vtkIdList* map = this->Maps->GetLocal( tid );
    vtkCellData* clData = this->InCd->GetLocal( tid );

    vtkIdType *pts, totalNumber, newId, n, clIndex;
    vtkCellArray* computedCells;

    if ( outputVerts )
      {
      computedCells = this->InVerts->GetLocal( tid );
      computedCells->InitTraversal();
      totalNumber = vertOffset->GetTuplesOffset( tid );
      newId = vertOffset->GetCellsOffset( tid ) - 1; // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = outputVerts->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }

    if ( outputLines )
      {
      computedCells = this->InLines->GetLocal( tid );
      computedCells->InitTraversal();
      totalNumber = lineOffset->GetTuplesOffset( tid );
      newId = lineOffset->GetCellsOffset( tid ) - 1 + vertOffset->GetNumberOfCells(); // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = outputLines->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }

    if ( outputPolys )
      {
      computedCells = this->InPolys->GetLocal( tid );
      computedCells->InitTraversal();
      totalNumber = polyOffset->GetTuplesOffset( tid );
      newId = polyOffset->GetCellsOffset( tid ) - 1 + lineOffset->GetNumberOfCells(); // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = outputPolys->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }

    if ( outputStrips )
      {
      computedCells = this->InStrips->GetLocal( tid );
      computedCells->InitTraversal();
      totalNumber = stripOffset->GetTuplesOffset( tid );
      newId = stripOffset->GetCellsOffset( tid ) - 1 + polyOffset->GetNumberOfCells(); // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = outputStrips->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }
    }
};

vtkStandardNewMacro(DummyMergeFunctor);

typedef vtkIdType *vtkIdTypePtr;
vtkIdType** TreatedTable = 0;

int MustTreatBucket( vtkIdType idx )
  {
  if ( !TreatedTable ) return 0;
  return !__sync_fetch_and_add(&(TreatedTable[idx]), 1);
  }

struct ParallelPointMerger : public vtkSMPCommand
{
  vtkTypeMacro(ParallelPointMerger,vtkSMPCommand);
  static ParallelPointMerger* New() { return new ParallelPointMerger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute(const vtkObject *caller, unsigned long eventId, void *callData) const
    {
    const DummyMergeFunctor* self = static_cast<const DummyMergeFunctor*>(caller);
    vtkSMPThreadID tid = *(static_cast<vtkSMPThreadID*>(callData));

    vtkSMPThreadID NumberOfThreads = vtkSMP::GetNumberOfThreads();
    vtkIdType NumberOfBuckets = self->outputLocator->GetNumberOfBuckets();

    for ( vtkIdType i = 0; i < NumberOfBuckets; ++i )
      {
      if ( self->Locators->GetLocal(tid)->GetNumberOfIdInBucket(i) )
        if ( MustTreatBucket(i) )
          for ( vtkSMPThreadID j = 1; j < NumberOfThreads; ++j )
            self->outputLocator->Merge( self->Locators->GetLocal(j), i, self->outputPd, self->InPd->GetLocal(j), self->Maps->GetLocal(j) );
      }
    }
protected:
  ParallelPointMerger() { }
  ~ParallelPointMerger() { }

private:


  ParallelPointMerger(const ParallelPointMerger&);
  void operator =(const ParallelPointMerger&);
};

struct ParallelCellMerger : public vtkSMPCommand
{
  vtkTypeMacro(ParallelCellMerger,vtkSMPCommand);
  static ParallelCellMerger* New() { return new ParallelCellMerger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute(const vtkObject *caller, unsigned long eventId, void *callData) const
    {
    const DummyMergeFunctor* self = static_cast<const DummyMergeFunctor*>(caller);
    vtkSMPThreadID tid = *(static_cast<vtkSMPThreadID*>(callData));

    self->CellsMerge( tid );
    }
protected:
  ParallelCellMerger() {}
  ~ParallelCellMerger() {}
private:
  ParallelCellMerger(const ParallelCellMerger&);
  void operator =(const ParallelCellMerger&);
};

struct Merger : public vtkSMPCommand
{
  vtkTypeMacro(Merger,vtkSMPCommand);
  static Merger* New() { return new Merger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute(const vtkObject *caller, unsigned long eventId, void *callData) const
    {
    const DummyMergeFunctor* self = static_cast<const DummyMergeFunctor*>(caller);
    vtkSMPThreadID tid = *(static_cast<vtkSMPThreadID*>(callData));

    vtkPointData* ptData = self->InPd->GetLocal( tid );
    vtkPoints* Points = self->InPoints->GetLocal( tid );
    vtkIdType newId, NumberOfPoints = Points->GetNumberOfPoints();
    vtkIdList* map = self->Maps->GetLocal( tid );
    double pt[3];

    for ( vtkIdType i = 0; i < NumberOfPoints; ++i )
      {
      Points->GetPoint( i, pt );
      if ( self->outputLocator->SetUniquePoint( pt, newId ) ) self->outputPd->SetTuple( newId, i, ptData );
      map->SetId( i, newId );
      }

    self->CellsMerge( tid );
    }
protected:
  Merger() {}
  ~Merger() {}
private:
  Merger(const Merger&);
  void operator =(const Merger&);
};

namespace vtkSMP
{
  void MergePoints(vtkSMPMergePoints *outPoints, vtkThreadLocal<vtkSMPMergePoints> *inPoints, vtkPointData *outPtsData, vtkThreadLocal<vtkPointData> *inPtsData, vtkCellArray *outVerts, vtkThreadLocal<vtkCellArray> *inVerts, vtkCellArray *outLines, vtkThreadLocal<vtkCellArray> *inLines, vtkCellArray *outPolys, vtkThreadLocal<vtkCellArray> *inPolys, vtkCellArray *outStrips, vtkThreadLocal<vtkCellArray> *inStrips, vtkCellData *outCellsData, vtkThreadLocal<vtkCellData> *inCellsData, int SkipThreads)
    {
    TreatedTable = new vtkIdTypePtr[outPoints->GetNumberOfBuckets()];
    memset( TreatedTable, 0, outPoints->GetNumberOfBuckets() * sizeof(vtkIdTypePtr) );

    DummyMergeFunctor* DummyFunctor = DummyMergeFunctor::New();
    DummyFunctor->InitializeNeeds( inPoints, 0, outPoints, inVerts, outVerts, inLines, outLines, inPolys, outPolys, inStrips, outStrips, inPtsData, outPtsData, inCellsData, outCellsData );

    ParallelPointMerger* TheMerge = ParallelPointMerger::New();
    Parallel( DummyFunctor, TheMerge, SkipThreads );
    TheMerge->Delete();

    ParallelCellMerger* TheCellMerge = ParallelCellMerger::New();
    Parallel( DummyFunctor, TheCellMerge, SkipThreads );
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

  void MergePoints(vtkPoints *outPoints, vtkThreadLocal<vtkPoints> *inPoints, vtkPointData *outPtsData, vtkThreadLocal<vtkPointData> *inPtsData, vtkCellArray *outVerts, vtkThreadLocal<vtkCellArray> *inVerts, vtkCellArray *outLines, vtkThreadLocal<vtkCellArray> *inLines, vtkCellArray *outPolys, vtkThreadLocal<vtkCellArray> *inPolys, vtkCellArray *outStrips, vtkThreadLocal<vtkCellArray> *inStrips, vtkCellData *outCellsData, vtkThreadLocal<vtkCellData> *inCellsData, int SkipThreads)
    {
    DummyMergeFunctor* Functor = DummyMergeFunctor::New();
    vtkSMPMergePoints* outputLocator = vtkSMPMergePoints::New();
    double bounds[6];
    inPoints->GetLocal(0)->GetBounds( bounds );
    outputLocator->InitLockInsertion( outPoints, bounds, inPoints->GetLocal(0)->GetNumberOfPoints() );
    Functor->outputLocator = outputLocator;

    vtkIdType PointsAlreadyPresent = outPoints->GetNumberOfPoints();
    if ( PointsAlreadyPresent ) ForEach( 0, PointsAlreadyPresent, Functor );

    Functor->InitializeNeeds( 0, inPoints, outputLocator, inVerts, outVerts, inLines, outLines, inPolys, outPolys, inStrips, outStrips, inPtsData, outPtsData, inCellsData, outCellsData );
    Merger* TheMerge = Merger::New();
    Parallel( Functor, TheMerge, SkipThreads );
    TheMerge->Delete();

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
}
