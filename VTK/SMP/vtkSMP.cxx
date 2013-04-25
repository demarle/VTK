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
vtkFunctorInitialisable::vtkFunctorInitialisable() :
    vtkFunctor(), IsInitialized(vtkSMP::InternalGetNumberOfThreads(), 0)
  {
  }

vtkFunctorInitialisable::~vtkFunctorInitialisable()
  {
  IsInitialized.clear();
  }

bool vtkFunctorInitialisable::ShouldInitialize( ) const
  {
  return !IsInitialized[vtkSMP::InternalGetTid()];
  }

void vtkFunctorInitialisable::Initialized( ) const
  {
  IsInitialized[vtkSMP::InternalGetTid()] = 1;
  }

void vtkFunctorInitialisable::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Is initialized: " << endl;
  for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < IsInitialized.size(); ++i )
    {
    os << indent.GetNextIndent() << "Id " << i << ": ";
    if ( IsInitialized[i] )
      os << "true";
    else
      os << "false";
    os << endl;
    }
  }

//--------------------------------------------------------------------------------
vtkTask::vtkTask() { }

vtkTask::~vtkTask() { }

void vtkTask::PrintSelf(ostream &os, vtkIndent indent)
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
  vtkstd::vector<vtkIdType>::iterator itCells;
  vtkstd::vector<vtkIdType>::iterator itTuples;

protected:
  OffsetManager() : vtkObject(), cells(vtkSMP::InternalGetNumberOfThreads(), 0), tuples(vtkSMP::InternalGetNumberOfThreads(), 0) { }
  ~OffsetManager() { }
public:
  vtkTypeMacro(OffsetManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent)
    {
    this->Superclass::PrintSelf( os, indent );
    os << indent << "Offsets for cells index: " << endl;
    for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < cells.size(); ++i )
      {
      os << indent.GetNextIndent() << "Id " << i << ": " << cells[i];
      }
    os << indent << "Offsets for tuples: " << endl;
    for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < tuples.size(); ++i )
      {
      os << indent.GetNextIndent() << "Id " << i << ": " << tuples[i];
      }
    os << indent << "Number of cells: " << CellsOffset << endl;
    os << indent << "Number of tuples: " << TuplesOffset << endl;
    }
  static OffsetManager* New();

  void InitManageValues ()
    {
    CellsOffset = 0;
    TuplesOffset = 0;
    itCells = cells.begin();
    itTuples = tuples.begin();
    }

  void ManageNextValue ( vtkCellArray* ca )
    {
    *itCells = CellsOffset;
    *itTuples = TuplesOffset;
    if ( ca )
      {
      CellsOffset += ca->GetNumberOfCells();
      TuplesOffset += ca->GetData()->GetNumberOfTuples();
      }
    ++itCells;
    ++itTuples;
    }

  vtkIdType GetNumberOfCells() { return CellsOffset; }
  vtkIdType GetNumberOfTuples() { return TuplesOffset; }
  vtkstd::vector<vtkIdType>::iterator GetCellsOffset ( ) { return cells.begin(); }
  vtkstd::vector<vtkIdType>::iterator GetTuplesOffset ( ) { return tuples.begin(); }
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

    InVerts->UnRegister( this );
    InLines->UnRegister( this );
    InPolys->UnRegister( this );
    InStrips->UnRegister( this );
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

  void operator ()( vtkIdType pointId ) const
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

    if ( _inverts )
      {
      InVerts = _inverts;
      InVerts->Register(this);
      }
    else
      {
      InVerts = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
      InVerts->Register(this);
      InVerts->Delete();
      }
    if ( _inlines )
      {
      InLines = _inlines;
      InLines->Register(this);
      }
    else
      {
      InLines = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
      InLines->Register(this);
      InLines->Delete();
      }
    if ( _inpolys )
      {
      InPolys = _inpolys;
      InPolys->Register(this);
      }
    else
      {
      InPolys = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
      InPolys->Register(this);
      InPolys->Delete();
      }
    if ( _instrips )
      {
      InStrips = _instrips;
      InStrips->Register(this);
      }
    else
      {
      InStrips = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
      InStrips->Register(this);
      InStrips->Delete();
      }
    outputVerts = _outverts;
    outputLines = _outlines;
    outputPolys = _outpolys;
    outputStrips = _outstrips;

    InPd = _inpd;
    outputPd = _outpd;

    InCd = _incd;
    outputCd = _outcd;

    NumberOfPoints = 0;
    vtkSMP::vtkThreadLocal<vtkSMPMergePoints>::iterator itLocator;
    if ( _locator )
      itLocator = Locators->Begin();
    vtkSMP::vtkThreadLocal<vtkPoints>::iterator itPoints;
    if ( _points )
      itPoints = InPoints->Begin();
    vtkSMP::vtkThreadLocal<vtkCellArray>::iterator itVerts = InVerts->Begin();
    vtkSMP::vtkThreadLocal<vtkCellArray>::iterator itLines = InLines->Begin();
    vtkSMP::vtkThreadLocal<vtkCellArray>::iterator itPolys = InPolys->Begin();
    vtkSMP::vtkThreadLocal<vtkCellArray>::iterator itStrips = InStrips->Begin();
    vertOffset->InitManageValues();
    lineOffset->InitManageValues();
    polyOffset->InitManageValues();
    stripOffset->InitManageValues();
    for ( vtkSMP::vtkThreadLocal<vtkIdList>::iterator itMaps = Maps->Begin(); itMaps != this->Maps->End(); ++itMaps )
      {
      vtkIdType n;
      if ( this->InPoints )
        {
        n = *itPoints ? (*itPoints)->GetNumberOfPoints() : 0;
        ++itPoints;
        }
      else
        {
        n = *itLocator ? static_cast<vtkMergePoints*>(*itLocator)->GetPoints()->GetNumberOfPoints() : 0;
        ++itLocator;
        }

      if ( n )
        {
        vtkIdList* map = *itMaps = vtkIdList::New();
        map->Allocate( n );
        NumberOfPoints += n;
        }

      vertOffset->ManageNextValue( *itVerts++ );
      lineOffset->ManageNextValue( *itLines++ );
      polyOffset->ManageNextValue( *itPolys++ );
      stripOffset->ManageNextValue( *itStrips++ );
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
  DummyMergeFunctor* self;

  vtkTypeMacro(ParallelPointMerger,vtkTask);
  static ParallelPointMerger* New() { return new ParallelPointMerger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute( vtkSMPMergePoints* locator ) const
    {
    if ( !locator ) return;

    vtkIdType NumberOfBuckets = self->outputLocator->GetNumberOfBuckets();
    vtkSMPMergePoints* l;

    for ( vtkIdType i = 0; i < NumberOfBuckets; ++i )
      {
      if ( locator->GetNumberOfIdInBucket(i) )
        if ( MustTreatBucket(i) )
          {
          vtkSMP::vtkThreadLocal<vtkPointData>::iterator itPd = self->InPd->Begin( 1 );
          vtkSMP::vtkThreadLocal<vtkIdList>::iterator itMaps = self->Maps->Begin( 1 );
          for ( vtkSMP::vtkThreadLocal<vtkSMPMergePoints>::iterator itLocator = self->Locators->Begin( 1 );
                itLocator != self->Locators->End(); ++itLocator, ++itPd, ++itMaps )
            if ( (l = *itLocator) )
              self->outputLocator->Merge( l, i, self->outputPd, *itPd, *itMaps );
          }
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
  DummyMergeFunctor* self;

  vtkTypeMacro(ParallelCellMerger,vtkTask);
  static ParallelCellMerger* New() { return new ParallelCellMerger; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute( vtkIdList* map,
                vtkCellData* clData,
                vtkCellArray* verts,
                vtkCellArray* lines,
                vtkCellArray* polys,
                vtkCellArray* strips,
                vtkIdType vertCellOffset,
                vtkIdType vertTupleOffset,
                vtkIdType lineCellOffset,
                vtkIdType lineTupleOffset,
                vtkIdType polyCellOffset,
                vtkIdType polyTupleOffset,
                vtkIdType stripCellOffset,
                vtkIdType stripTupleOffset ) const
    {
    if ( !map ) return;

    vtkIdType *pts, totalNumber, newId, n, clIndex;
    vtkCellArray* computedCells;

    if ( self->outputVerts )
      {
      clIndex = 0;
      computedCells = verts;
      computedCells->InitTraversal();
      totalNumber = vertTupleOffset;
      newId = vertCellOffset - 1; // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = self->outputVerts->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        self->outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }

    if ( self->outputLines )
      {
      clIndex = 0;
      computedCells = lines;
      computedCells->InitTraversal();
      totalNumber = lineTupleOffset;
      newId = lineCellOffset - 1 + self->vertOffset->GetNumberOfCells(); // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = self->outputLines->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        self->outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }

    if ( self->outputPolys )
      {
      clIndex = 0;
      computedCells = polys;
      computedCells->InitTraversal();
      totalNumber = polyTupleOffset;
      newId = polyCellOffset - 1 + self->lineOffset->GetNumberOfCells(); // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = self->outputPolys->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        self->outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }

    if ( self->outputStrips )
      {
      clIndex = 0;
      computedCells = strips;
      computedCells->InitTraversal();
      totalNumber = stripTupleOffset;
      newId = stripCellOffset - 1 + self->polyOffset->GetNumberOfCells(); // usage of ++newId instead of newId++
      while (computedCells->GetNextCell( n, pts ))
        {
        vtkIdType* writePtr = self->outputStrips->WritePointer( 0, totalNumber );
        writePtr += totalNumber;
        *writePtr++ = n;
        for (vtkIdType i = 0; i < n; ++i)
          {
          *writePtr++ = map->GetId(pts[i]);
          }
        self->outputCd->SetTuple(++newId, ++clIndex, clData);
        totalNumber += n + 1;
        }
      }
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

  void operator()( vtkIdType id ) const
    {
    vtkSMP::vtkThreadLocal<vtkPoints>::iterator itPoints = this->Functor->InPoints->Begin();
    vtkSMP::vtkThreadLocal<vtkPointData>::iterator itPd = this->Functor->InPd->Begin();
    vtkSMP::vtkThreadLocal<vtkIdList>::iterator itMaps = this->Functor->Maps->Begin();

    vtkIdType NumberOfPoints = NumberOfPointsFirstThread, NewId;
    while ( id >= NumberOfPoints )
      {
      id -= NumberOfPoints;
      ++itPoints; ++itPd; ++itMaps;
      NumberOfPoints = (*itPoints)->GetNumberOfPoints();
      }

    double* pt = new double[3];
    (*itPoints)->GetPoint( id, pt );
    if ( this->Functor->outputLocator->SetUniquePoint( pt, NewId ) )
      this->Functor->outputPd->SetTuple( NewId, id, (*itPd) );
    (*itMaps)->SetId( id, NewId );
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
    TheMerge->self = DummyFunctor;
    Parallel<vtkSMPMergePoints>( TheMerge, DummyFunctor->Locators->Begin(), SkipThreads );
    TheMerge->Delete();

    ParallelCellMerger* TheCellMerge = ParallelCellMerger::New();
    TheCellMerge->self = DummyFunctor;
    Parallel<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>( TheCellMerge,
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

  void MergePoints(vtkPoints *outPoints, vtkThreadLocal<vtkPoints> *inPoints, const double bounds[6], vtkPointData *outPtsData, vtkThreadLocal<vtkPointData> *inPtsData, vtkCellArray *outVerts, vtkThreadLocal<vtkCellArray> *inVerts, vtkCellArray *outLines, vtkThreadLocal<vtkCellArray> *inLines, vtkCellArray *outPolys, vtkThreadLocal<vtkCellArray> *inPolys, vtkCellArray *outStrips, vtkThreadLocal<vtkCellArray> *inStrips, vtkCellData *outCellsData, vtkThreadLocal<vtkCellData> *inCellsData, int SkipThreads)
    {
    vtkBenchTimer* timer = vtkBenchTimer::New();
    timer->start_bench_timer();
    DummyMergeFunctor* Functor = DummyMergeFunctor::New();
    vtkSMPMergePoints* outputLocator = vtkSMPMergePoints::New();
    vtkIdType NumberOfInPointsThread0 = (*(inPoints->Begin( 0 )))->GetNumberOfPoints();
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
    TheCellMerge->self = Functor;
    Parallel<vtkIdList, vtkCellData, vtkCellArray, vtkCellArray, vtkCellArray, vtkCellArray>( TheCellMerge,
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

  void vtkSpawnTasks::PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf( os, indent );
    }

  vtkStandardNewMacro(vtkSpawnTasks);
}
