#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkSMPMergePoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkMutexLock.h"

#include "vtkBenchTimer.h"

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
  vtkSMPThreadID NumThreads = vtkSMP::GetNumberOfThreads();
  IsInitialized = new vtkIdType[NumThreads];
  memset( IsInitialized, 0, NumThreads * sizeof(vtkIdType) );
  }

vtkFunctorInitialisable::~vtkFunctorInitialisable()
  {
  delete [] IsInitialized;
  }

bool vtkFunctorInitialisable::ShouldInitialize( vtkSMPThreadID tid ) const
  {
  return !IsInitialized[tid];
  }

void vtkFunctorInitialisable::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Is initialized: " << IsInitialized << endl;
  }

//--------------------------------------------------------------------------------
vtkTask::vtkTask() { }

vtkTask::~vtkTask() { }

void vtkTask::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

//--------------------------------------------------------------------------------
vtkTreeIndex::vtkTreeIndex(vtkIdType i, int l)
  {
  index = i;
  level = l;
  }

vtkTreeIndex::vtkTreeIndex()
  {
  index = -1;
  level = 0;
  }

//--------------------------------------------------------------------------------
vtkTreeTraversalHelper::vtkTreeTraversalHelper()
  {
  this->indexes = 0;
  this->current_head = 0;
  this->size = 0;
  }

void vtkTreeTraversalHelper::Init( vtkIdType s, vtkIdType i, int l )
  {
  this->size = s;
  this->indexes = new vtkTreeIndex[s];
  this->current_head = s - 1;
  this->indexes[this->current_head] = vtkTreeIndex( i, l );
  }

vtkTreeTraversalHelper::~vtkTreeTraversalHelper()
  {
  delete [] this->indexes;
  this->size = 0;
  this->current_head = 0;
  this->indexes = 0;
  }

vtkTreeIndex* vtkTreeTraversalHelper::Get( )
  {
  vtkIdType i = (this->current_head)++;
  while ( i < 0 )
    i += this->size;
  return &(this->indexes[ i ]);
  }

vtkTreeIndex* vtkTreeTraversalHelper::Steal( vtkIdType i )
  {
  while ( i < 0 )
    i += this->size;
  return &(this->indexes[ i ]);
  }

void vtkTreeTraversalHelper::push_head ( vtkIdType index, int level )
  { // currently same as push_tail
  vtkIdType i = --(this->current_head);
  while ( i < 0 )
    i += this->size;
  this->indexes[ i ] = vtkTreeIndex( index, level );
  }

void vtkTreeTraversalHelper::push_tail ( vtkIdType index, int level )
  {
  vtkIdType i = --(this->current_head);
  while ( i < 0 )
    i += this->size;
  this->indexes[ i ] = vtkTreeIndex( index, level );
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
  vtkIdType NumberOfPoints;

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
  vtkIdType GetNumberOfPoints() const { return NumberOfPoints; }

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

    NumberOfPoints = 0;
    vtkSMPThreadID max = vtkSMP::GetNumberOfThreads();
    for ( vtkSMPThreadID tid = 0; tid < max; ++tid )
      {
      vtkCellArray* _ca_ = this->InVerts->GetLocal( tid );
      if ( !_ca_ ) continue;
      if ( this->InVerts ) vertOffset->ManageValues( tid, _ca_ );
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
    if ( !map ) return;
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

struct ParallelPointMerger : public vtkTask
{
  vtkTypeMacro(ParallelPointMerger,vtkTask);
  static ParallelPointMerger* New() { return new ParallelPointMerger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute( vtkSMPThreadID tid, const vtkObject* data ) const
    {
    const DummyMergeFunctor* self = static_cast<const DummyMergeFunctor*>(data);
    vtkSMPMergePoints *l, *locator = self->Locators->GetLocal( tid );
    if ( !locator ) return;

    vtkSMPThreadID NumberOfThreads = vtkSMP::GetNumberOfThreads();
    vtkIdType NumberOfBuckets = self->outputLocator->GetNumberOfBuckets();

    for ( vtkIdType i = 0; i < NumberOfBuckets; ++i )
      {
      if ( self->Locators->GetLocal(tid)->GetNumberOfIdInBucket(i) )
        if ( MustTreatBucket(i) )
          for ( vtkSMPThreadID j = 1; j < NumberOfThreads; ++j )
            if ( (l = self->Locators->GetLocal(j)) )
              self->outputLocator->Merge( l, i, self->outputPd, self->InPd->GetLocal(j), self->Maps->GetLocal(j) );
      }
    }
protected:
  ParallelPointMerger() { }
  ~ParallelPointMerger() { }

private:
  ParallelPointMerger(const ParallelPointMerger&);
  void operator =(const ParallelPointMerger&);
};

struct ParallelCellMerger : public vtkTask
{
  vtkTypeMacro(ParallelCellMerger,vtkTask);
  static ParallelCellMerger* New() { return new ParallelCellMerger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute( vtkSMPThreadID tid, const vtkObject* data ) const
    {
    static_cast<const DummyMergeFunctor*>(data)->CellsMerge( tid );
    }
protected:
  ParallelCellMerger() {}
  ~ParallelCellMerger() {}
private:
  ParallelCellMerger(const ParallelCellMerger&);
  void operator =(const ParallelCellMerger&);
};

struct LockPointMerger : public vtkFunctor
{
  DummyMergeFunctor* Functor;
  vtkIdType NumberOfPointsFirstThread;

  vtkTypeMacro(LockPointMerger,vtkFunctor);
  static LockPointMerger* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void operator()( vtkIdType id, vtkSMPThreadID tid ) const
    {
    vtkSMPThreadID threadID = 0;
    vtkIdType NumberOfPoints = NumberOfPointsFirstThread, NewId;
    while ( id >= NumberOfPoints )
      {
      id -= NumberOfPoints;
      ++threadID;
      NumberOfPoints = this->Functor->InPoints->GetLocal(threadID)->GetNumberOfPoints();
      }

    double* pt = new double[3];
    this->Functor->InPoints->GetLocal( threadID )->GetPoint( id, pt );
    if ( this->Functor->outputLocator->SetUniquePoint( pt, NewId ) )
      this->Functor->outputPd->SetTuple( NewId, id, this->Functor->InPd->GetLocal( threadID ) );
    this->Functor->Maps->GetLocal( threadID )->SetId( id, NewId );
    delete [] pt;
    }
protected:
  LockPointMerger() {}
  ~LockPointMerger() {}
private:
  LockPointMerger(const LockPointMerger&);
  void operator =(const LockPointMerger&);
};

vtkStandardNewMacro(LockPointMerger);

namespace vtkSMP
{
  void MergePoints(vtkSMPMergePoints *outPoints, vtkThreadLocal<vtkSMPMergePoints> *inPoints, vtkPointData *outPtsData, vtkThreadLocal<vtkPointData> *inPtsData, vtkCellArray *outVerts, vtkThreadLocal<vtkCellArray> *inVerts, vtkCellArray *outLines, vtkThreadLocal<vtkCellArray> *inLines, vtkCellArray *outPolys, vtkThreadLocal<vtkCellArray> *inPolys, vtkCellArray *outStrips, vtkThreadLocal<vtkCellArray> *inStrips, vtkCellData *outCellsData, vtkThreadLocal<vtkCellData> *inCellsData, int SkipThreads)
    {
    TreatedTable = new vtkIdTypePtr[outPoints->GetNumberOfBuckets()];
    memset( TreatedTable, 0, outPoints->GetNumberOfBuckets() * sizeof(vtkIdTypePtr) );

    DummyMergeFunctor* DummyFunctor = DummyMergeFunctor::New();
    DummyFunctor->InitializeNeeds( inPoints, 0, outPoints, inVerts, outVerts, inLines, outLines, inPolys, outPolys, inStrips, outStrips, inPtsData, outPtsData, inCellsData, outCellsData );

    ParallelPointMerger* TheMerge = ParallelPointMerger::New();
    Parallel( TheMerge, DummyFunctor, SkipThreads );
    TheMerge->Delete();

    ParallelCellMerger* TheCellMerge = ParallelCellMerger::New();
    Parallel( TheCellMerge, DummyFunctor, SkipThreads );
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

  void MergePoints(vtkPoints *outPoints, vtkThreadLocal<vtkPoints> *inPoints, const double bounds[6], vtkPointData *outPtsData, vtkThreadLocal<vtkPointData> *inPtsData, vtkCellArray *outVerts, vtkThreadLocal<vtkCellArray> *inVerts, vtkCellArray *outLines, vtkThreadLocal<vtkCellArray> *inLines, vtkCellArray *outPolys, vtkThreadLocal<vtkCellArray> *inPolys, vtkCellArray *outStrips, vtkThreadLocal<vtkCellArray> *inStrips, vtkCellData *outCellsData, vtkThreadLocal<vtkCellData> *inCellsData, int SkipThreads)
    {
    vtkBenchTimer* timer = vtkBenchTimer::New();
    timer->start_bench_timer();
    DummyMergeFunctor* Functor = DummyMergeFunctor::New();
    vtkSMPMergePoints* outputLocator = vtkSMPMergePoints::New();
    vtkIdType NumberOfInPointsThread0 = inPoints->GetLocal(0)->GetNumberOfPoints();
    outputLocator->InitLockInsertion( outPoints, bounds, NumberOfInPointsThread0 );
    Functor->outputLocator = outputLocator;

    vtkIdType PointsAlreadyPresent = outPoints->GetNumberOfPoints();
    if ( PointsAlreadyPresent ) ForEach( 0, PointsAlreadyPresent, Functor );

    Functor->InitializeNeeds( 0, inPoints, outputLocator, inVerts, outVerts, inLines, outLines, inPolys, outPolys, inStrips, outStrips, inPtsData, outPtsData, inCellsData, outCellsData );
    timer->end_bench_timer();

    timer->start_bench_timer();
    vtkIdType StartPoint = SkipThreads ? NumberOfInPointsThread0 : 0;
    LockPointMerger* TheMerge = LockPointMerger::New();
    TheMerge->Functor = Functor;
    TheMerge->NumberOfPointsFirstThread = NumberOfInPointsThread0;
    ForEach( StartPoint, Functor->GetNumberOfPoints(), TheMerge );
    TheMerge->Delete();

    ParallelCellMerger* TheCellMerge = ParallelCellMerger::New();
    Parallel( TheCellMerge, Functor, SkipThreads );
    TheCellMerge->Delete();
    timer->end_bench_timer();

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
