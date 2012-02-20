#include "vtkSMPContourFilter.h"
#include "vtkObjectFactory.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageData.h"
#include "vtkUniformGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSynchronizedTemplates2D.h"
#include "vtkSynchronizedTemplates3D.h"
#include "vtkRectilinearGrid.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkRectilinearSynchronizedTemplates.h"
#include "vtkScalarTree.h"
#include "vtkSimpleScalarTree.h"
#include "vtkGenericCell.h"
#include "vtkCell.h"
#include "vtkGridSynchronizedTemplates3D.h"
#include "vtkContourGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkCellArray.h"
#include "vtkCutter.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkMergePoints.h"

#include "vtkSMP.h"
#include "vtkSMPMergePoints.h"
#include "vtkCommand.h"

#include "vtkBenchTimer.h"

vtkStandardNewMacro(vtkSMPContourFilter);

vtkSMPContourFilter::vtkSMPContourFilter() : vtkContourFilter() { }
vtkSMPContourFilter::~vtkSMPContourFilter() { }

void vtkSMPContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

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

class ThreadsFunctor : public vtkFunctor//Initialisable
{
  ThreadsFunctor( const ThreadsFunctor& );
  void operator =( const ThreadsFunctor& );

protected:
  ThreadsFunctor()
    {
    Locator = vtkSMP::vtkThreadLocal<vtkIncrementalPointLocator>::New();
    newPts = vtkSMP::vtkThreadLocal<vtkPoints>::New();
    newVerts = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
    newLines = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
    newPolys = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
    outPd = vtkSMP::vtkThreadLocal<vtkPointData>::New();
    outCd = vtkSMP::vtkThreadLocal<vtkCellData>::New();
    Cells = vtkSMP::vtkThreadLocal<vtkGenericCell>::New();
    CellsScalars = vtkSMP::vtkThreadLocal<vtkDataArray>::New();

    numberOfPoints = 0;
    vertOffset = OffsetManager::New();
    lineOffset = OffsetManager::New();
    polyOffset = OffsetManager::New();
    }

  ~ThreadsFunctor()
    {
    Locator->Delete();
    newPts->Delete();
    newVerts->Delete();
    newLines->Delete();
    newPolys->Delete();
    outPd->Delete();
    outCd->Delete();
    Cells->Delete();
    CellsScalars->Delete();

    vertOffset->Delete();
    lineOffset->Delete();
    polyOffset->Delete();
    }

public:
  vtkTypeMacro(ThreadsFunctor,vtkFunctor);
  static ThreadsFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
  {
    this->Superclass::PrintSelf(os,indent);
  }

  vtkSMP::vtkThreadLocal<vtkIncrementalPointLocator>* Locator;
  vtkSMP::vtkThreadLocal<vtkPoints>* newPts;
  vtkSMP::vtkThreadLocal<vtkCellArray>* newVerts;
  vtkSMP::vtkThreadLocal<vtkCellArray>* newLines;
  vtkSMP::vtkThreadLocal<vtkCellArray>* newPolys;
  vtkSMP::vtkThreadLocal<vtkPointData>* outPd;
  vtkSMP::vtkThreadLocal<vtkCellData>* outCd;
  vtkSMP::vtkThreadLocal<vtkGenericCell>* Cells;
  vtkSMP::vtkThreadLocal<vtkDataArray>* CellsScalars;

  vtkDataArray* inScalars;
  vtkDataSet* input;
  vtkPointData* inPd;
  vtkCellData* inCd;
  vtkIdType estimatedSize;
  double* values;
  int numContours;
  int computeScalars;
  vtkIdType numberOfPoints;

  OffsetManager* vertOffset;
  OffsetManager* lineOffset;
  OffsetManager* polyOffset;

  vtkSMPMergePoints* refLocator;
  vtkCellArray* outputVerts;
  vtkCellArray* outputLines;
  vtkCellArray* outputPolys;
  vtkCellData* outputCd;
  vtkPointData* outputPd;

  unsigned char cellTypeDimensions[VTK_NUMBER_OF_CELL_TYPES];

  int dimensionality;

  void SetData( vtkDataSet* _input, vtkPoints* _inPts, vtkCellData* _incd,
                  vtkPointData* _inpd, vtkSMPMergePoints* _locator,
                  vtkIdType& _size, double* _values, int _number,
                  vtkDataArray* _scalars, int _compute, vtkCellArray* _outputVerts,
                  vtkCellArray* _outputLines, vtkCellArray* _outputPolys,
                  vtkCellData* _outputCd, vtkPointData* _outputPd )
    {
    vtkCutter::GetCellTypeDimensions(cellTypeDimensions);

    this->inCd = _incd;
    this->inPd = _inpd;
    this->input = _input;
    this->refLocator = _locator;
    this->estimatedSize = _size;
    this->values = _values;
    this->numContours = _number;
    this->inScalars = _scalars;
    this->computeScalars = _compute;

    outputVerts = _outputVerts;
    outputLines = _outputLines;
    outputPolys = _outputPolys;
    outputCd = _outputCd;
    outputPd = _outputPd;

    if ( !computeScalars )
      {
      _outputPd->CopyScalarsOff();
      }
    _outputPd->InterpolateAllocate( inPd, estimatedSize, estimatedSize );
    _outputCd->CopyAllocate( inCd, estimatedSize, estimatedSize );
    Locator->SetLocal( 0, _locator );
    newPts->SetLocal( 0, _inPts );
    newVerts->SetLocal( 0, _outputVerts );
    newLines->SetLocal( 0, _outputLines );
    newPolys->SetLocal( 0, _outputPolys );
    outPd->SetLocal( 0, _outputPd );
    outCd->SetLocal( 0, _outputCd );
    Cells->NewLocal( 0 );
    vtkDataArray* cScalars = CellsScalars->NewLocal( 0, inScalars );
    cScalars->SetNumberOfComponents( inScalars->GetNumberOfComponents() );
    cScalars->Allocate( cScalars->GetNumberOfComponents() * VTK_CELL_SIZE );
    }

  vtkIdType GetNumberOfPoints() { return numberOfPoints; }
  vtkIdType GetNumberOfVerts() { return vertOffset->GetNumberOfCells(); }
  vtkIdType GetNumberOfLines() { return lineOffset->GetNumberOfCells(); }
  vtkIdType GetNumberOfPolys() { return polyOffset->GetNumberOfCells(); }
  vtkIdType GetNumberOfVertTuples() { return vertOffset->GetNumberOfTuples(); }
  vtkIdType GetNumberOfLineTuples() { return lineOffset->GetNumberOfTuples(); }
  vtkIdType GetNumberOfPolyTuples() { return polyOffset->GetNumberOfTuples(); }

  void operator ()( vtkIdType cellId, vtkSMPThreadID tid ) const
    {
    vtkGenericCell *cell = this->Cells->GetLocal( tid );
    int cellType = input->GetCellType(cellId);
    if (cellType >= VTK_NUMBER_OF_CELL_TYPES)
      { // Protect against new cell types added.
//      vtkErrorMacro("Unknown cell type " << cellType);
      return;
      }
    if (cellTypeDimensions[cellType] != dimensionality)
      {
      return;
      }
    input->GetCell(cellId,cell);
    vtkIdList* cellPts = cell->GetPointIds();
    vtkDataArray* cellScalars = this->CellsScalars->GetLocal( tid );
    if ( cellScalars->GetSize() / cellScalars->GetNumberOfComponents() < cellPts->GetNumberOfIds() )
      {
      cellScalars->Allocate(cellScalars->GetNumberOfComponents()*cellPts->GetNumberOfIds());
      }
    inScalars->GetTuples(cellPts,cellScalars);

    for (int i = 0; i < numContours; i++)
      {
      double v = values[i];
      cell->Contour( v, cellScalars, this->Locator->GetLocal( tid ),
                     newVerts->GetLocal( tid ), newLines->GetLocal( tid ), newPolys->GetLocal( tid ),
                     inPd, outPd->GetLocal( tid ), inCd, cellId, outCd->GetLocal( tid ) );
      }
    }

  void PreMerge ( )
  {
    vtkSMPThreadID max = vtkSMP::GetNumberOfThreads();
    for ( vtkSMPThreadID i = 0; i < max; ++i )
      pre_merge( i );
  }

  /*void Init ( vtkSMPThreadID tid ) const
    {
    vtkPoints* pts = newPts->NewLocal( tid );
    pts->Allocate( estimatedSize, estimatedSize );
    vtkIncrementalPointLocator* l = Locator->NewLocal( tid, refLocator );
    l->InitPointInsertion( pts, input->GetBounds(), estimatedSize );

    vtkCellArray* c = newVerts->NewLocal( tid );
    c->Allocate( estimatedSize, estimatedSize );
    c = newLines->NewLocal( tid );
    c->Allocate( estimatedSize, estimatedSize );
    c = newPolys->NewLocal( tid );
    c->Allocate( estimatedSize, estimatedSize );

    vtkPointData* pd = outPd->NewLocal( tid );
    if ( !computeScalars )
      {
      pd->CopyScalarsOff();
      }
    pd->InterpolateAllocate( inPd, estimatedSize, estimatedSize );

    vtkCellData* cd = outCd->NewLocal( tid );
    cd->CopyAllocate( inCd, estimatedSize, estimatedSize );

    Cells->NewLocal( tid );

    vtkDataArray* cScalars = CellsScalars->NewLocal( tid, inScalars );
    cScalars->SetNumberOfComponents( inScalars->GetNumberOfComponents() );
    cScalars->Allocate( cScalars->GetNumberOfComponents() * VTK_CELL_SIZE );
    }*/

  void pre_merge ( vtkSMPThreadID tid )
    {
    vertOffset->ManageValues( tid, this->newVerts->GetLocal( tid ) );
    lineOffset->ManageValues( tid, this->newLines->GetLocal( tid ) );
    polyOffset->ManageValues( tid, this->newPolys->GetLocal( tid ) );

    numberOfPoints += this->newPts->GetLocal( tid )->GetNumberOfPoints();
    }

  void ContourMerge ( vtkSMPThreadID tid ) const
    {
    vtkPointData* ptData = this->outPd->GetLocal( tid );
    vtkCellData* clData = this->outCd->GetLocal( tid );

    vtkPoints* computedPoints = this->newPts->GetLocal( tid );
    vtkIdType n = computedPoints->GetNumberOfPoints(), newId, clIndex = -1;
    vtkstd::vector<vtkIdType> map( n );
    for ( vtkIdType i = 0; i < n; ++i )
      {
      double* pt = computedPoints->GetPoint( i );
      int inserted = refLocator->SetUniquePoint( pt, newId );
      if ( inserted )
        outputPd->SetTuple( newId, i, ptData );
      map[i] = newId;
      }

    vtkIdType *pts, totalNumber;
    vtkCellArray* computedCells = this->newVerts->GetLocal( tid );
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
        *writePtr++ = map[pts[i]];
        }
      outputCd->SetTuple(++newId, ++clIndex, clData);
      totalNumber += n + 1;
      }

    computedCells = this->newLines->GetLocal( tid );
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
        *writePtr++ = map[pts[i]];
        }
      outputCd->SetTuple(++newId, ++clIndex, clData);
      totalNumber += n + 1;
      }

    computedCells = this->newPolys->GetLocal( tid );
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
        *writePtr++ = map[pts[i]];
        }
      outputCd->SetTuple(++newId, ++clIndex, clData);
      totalNumber += n + 1;
      }

    }
};

vtkStandardNewMacro(ThreadsFunctor);

struct MyInit : public vtkSMPCommand
{
  vtkTypeMacro(MyInit,vtkSMPCommand);
  static MyInit* New() { return new MyInit; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute(const vtkObject *caller, unsigned long eventId, void *callData) const
    {
    const ThreadsFunctor* self = static_cast<const ThreadsFunctor*>(caller);
    vtkSMPThreadID tid = *(static_cast<vtkSMPThreadID*>(callData));
    vtkPoints* pts = self->newPts->NewLocal( tid );
    pts->Allocate( self->estimatedSize, self->estimatedSize );
    vtkIncrementalPointLocator* l = self->Locator->NewLocal( tid, self->refLocator );
    l->InitPointInsertion( pts, self->input->GetBounds(), self->estimatedSize );

    vtkCellArray* c = self->newVerts->NewLocal( tid );
    c->Allocate( self->estimatedSize, self->estimatedSize );
    c = self->newLines->NewLocal( tid );
    c->Allocate( self->estimatedSize, self->estimatedSize );
    c = self->newPolys->NewLocal( tid );
    c->Allocate( self->estimatedSize, self->estimatedSize );

    vtkPointData* pd = self->outPd->NewLocal( tid );
    if ( !self->computeScalars )
      {
      pd->CopyScalarsOff();
      }
    pd->InterpolateAllocate( self->inPd, self->estimatedSize, self->estimatedSize );

    vtkCellData* cd = self->outCd->NewLocal( tid );
    cd->CopyAllocate( self->inCd, self->estimatedSize, self->estimatedSize );

    self->Cells->NewLocal( tid );

    vtkDataArray* cScalars = self->CellsScalars->NewLocal( tid, self->inScalars );
    cScalars->SetNumberOfComponents( self->inScalars->GetNumberOfComponents() );
    cScalars->Allocate( cScalars->GetNumberOfComponents() * VTK_CELL_SIZE );
    }
protected:
  MyInit() {}
  ~MyInit() {}
private:
  MyInit(const MyInit&);
  void operator =(const MyInit&);
};

struct MyMerge : public vtkSMPCommand
{
  vtkTypeMacro(MyMerge,vtkSMPCommand);
  static MyMerge* New() { return new MyMerge; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Execute(const vtkObject *caller, unsigned long eventId, void *callData) const
    {
    const ThreadsFunctor* self = static_cast<const ThreadsFunctor*>(caller);
    vtkSMPThreadID tid = *(static_cast<vtkSMPThreadID*>(callData));
    self->ContourMerge(tid);
    }
protected:
  MyMerge() {}
  ~MyMerge() {}
private:
  MyMerge(const MyMerge&);
  void operator =(const MyMerge&);
};

// General contouring filter.  Handles arbitrary input.
//
int vtkSMPContourFilter::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the input
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!input)
    {
    return 0;
    }

  // get the contours
  int numContours=this->ContourValues->GetNumberOfContours();
  double *values=this->ContourValues->GetValues();
  int i;

  // is there data to process?
  if (!this->GetInputArrayToProcess(0, inputVector))
    {
    return 1;
    }

  int sType = this->GetInputArrayToProcess(0, inputVector)->GetDataType();

  // handle 2D images
  if (vtkImageData::SafeDownCast(input) && sType != VTK_BIT &&
      !vtkUniformGrid::SafeDownCast(input))
    {
    int dim = 3;
    int *uExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    if (uExt[0] == uExt[1])
      {
      --dim;
      }
    if (uExt[2] == uExt[3])
      {
      --dim;
      }
    if (uExt[4] == uExt[5])
      {
      --dim;
      }

    if ( dim == 2 )
      {
      this->SynchronizedTemplates2D->SetNumberOfContours(numContours);
      for (i=0; i < numContours; i++)
        {
        this->SynchronizedTemplates2D->SetValue(i,values[i]);
        }
      this->SynchronizedTemplates2D->
        SetInputArrayToProcess(0,this->GetInputArrayInformation(0));
      return
        this->SynchronizedTemplates2D->ProcessRequest(request,inputVector,outputVector);
      }
    else if ( dim == 3 )
      {
      this->SynchronizedTemplates3D->SetNumberOfContours(numContours);
      for (i=0; i < numContours; i++)
        {
        this->SynchronizedTemplates3D->SetValue(i,values[i]);
        }
      this->SynchronizedTemplates3D->SetComputeNormals(this->ComputeNormals);
      this->SynchronizedTemplates3D->SetComputeGradients(this->ComputeGradients);
      this->SynchronizedTemplates3D->SetComputeScalars(this->ComputeScalars);
      this->SynchronizedTemplates3D->
        SetInputArrayToProcess(0,this->GetInputArrayInformation(0));

      return this->SynchronizedTemplates3D->ProcessRequest(request,inputVector,outputVector);
      }
    } //if image data

  // handle 3D RGrids
  if (vtkRectilinearGrid::SafeDownCast(input) && sType != VTK_BIT)
    {
    int *uExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    // if 3D
    if (uExt[0] < uExt[1] && uExt[2] < uExt[3] && uExt[4] < uExt[5])
      {
      this->RectilinearSynchronizedTemplates->SetNumberOfContours(numContours);
      for (i=0; i < numContours; i++)
        {
        this->RectilinearSynchronizedTemplates->SetValue(i,values[i]);
        }
      this->RectilinearSynchronizedTemplates->SetComputeNormals(this->ComputeNormals);
      this->RectilinearSynchronizedTemplates->SetComputeGradients(this->ComputeGradients);
      this->RectilinearSynchronizedTemplates->SetComputeScalars(this->ComputeScalars);
      this->RectilinearSynchronizedTemplates->
        SetInputArrayToProcess(0,this->GetInputArrayInformation(0));
      return this->RectilinearSynchronizedTemplates->
        ProcessRequest(request,inputVector,outputVector);
      }
    } // if 3D Rgrid

  // handle 3D SGrids
  if (vtkStructuredGrid::SafeDownCast(input) && sType != VTK_BIT)
    {
    int *uExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    // if 3D
    if (uExt[0] < uExt[1] && uExt[2] < uExt[3] && uExt[4] < uExt[5])
      {
      this->GridSynchronizedTemplates->SetNumberOfContours(numContours);
      for (i=0; i < numContours; i++)
        {
        this->GridSynchronizedTemplates->SetValue(i,values[i]);
        }
      this->GridSynchronizedTemplates->SetComputeNormals(this->ComputeNormals);
      this->GridSynchronizedTemplates->SetComputeGradients(this->ComputeGradients);
      this->GridSynchronizedTemplates->SetComputeScalars(this->ComputeScalars);
      this->GridSynchronizedTemplates->
        SetInputArrayToProcess(0,this->GetInputArrayInformation(0));
      return this->GridSynchronizedTemplates->
        ProcessRequest(request,inputVector,outputVector);
      }
    } //if 3D SGrid

  vtkIdType cellId;
//  int abortExecute=0;
  vtkIdList *cellPts;
  vtkDataArray *inScalars;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPts;
  vtkIdType numCells, estimatedSize;
  vtkDataArray *cellScalars;

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));
  if (!output) {return 0;}


  vtkPointData *inPd=input->GetPointData(), *outPd=output->GetPointData();
  vtkCellData *inCd=input->GetCellData(), *outCd=output->GetCellData();

  vtkDebugMacro(<< "Executing contour filter");
  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    vtkDebugMacro(<< "Processing unstructured grid");
    vtkContourGrid *cgrid;

    cgrid = vtkContourGrid::New();
    cgrid->SetInput(input);
    if ( this->Locator )
      {
      cgrid->SetLocator( this->Locator );
      }

    for (i = 0; i < numContours; i++)
      {
      cgrid->SetValue(i, values[i]);
      }
    cgrid->GetOutput()->SetUpdateExtent(output->GetUpdatePiece(),
                                        output->GetUpdateNumberOfPieces(),
                                        output->GetUpdateGhostLevel());
    cgrid->SetInputArrayToProcess(0,this->GetInputArrayInformation(0));
    cgrid->Update();
    output->ShallowCopy(cgrid->GetOutput());
    cgrid->SetInput(0);
    cgrid->Delete();
    } //if type VTK_UNSTRUCTURED_GRID
  else
    {
    numCells = input->GetNumberOfCells();
    inScalars = this->GetInputArrayToProcess(0,inputVector);
    if ( ! inScalars || numCells < 1 )
      {
      vtkDebugMacro(<<"No data to contour");
      return 1;
      }

    // Create objects to hold output of contour operation. First estimate
    // allocation size.
    //
    estimatedSize=
      static_cast<vtkIdType>(pow(static_cast<double>(numCells),.75));
    estimatedSize *= numContours;
    estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
    if (estimatedSize < 1024)
      {
      estimatedSize = 1024;
      }

    newPts = vtkPoints::New();
    newPts->Allocate(estimatedSize,estimatedSize);
    newVerts = vtkCellArray::New();
    newLines = vtkCellArray::New();
    newPolys = vtkCellArray::New();
    cellScalars = inScalars->NewInstance();

    // interpolate data along edge
    // if we did not ask for scalars to be computed, don't copy them
    if (!this->ComputeScalars)
      {
      outPd->CopyScalarsOff();
      }

    // If enabled, build a scalar tree to accelerate search
    //
    if ( !this->UseScalarTree )
      {
      if ( !this->Locator || !this->Locator->IsA("vtkSMPMergePoints") )
        {
        if ( this->Locator )
          this->Locator->UnRegister(this);
        this->Locator = vtkSMPMergePoints::New();
        this->Locator->Register(this);
        this->Locator->Delete();
        }
      this->Locator->InitPointInsertion( newPts, input->GetBounds(), estimatedSize );
      vtkBenchTimer* timer = vtkBenchTimer::New();
      cout << endl;

      timer->start_bench_timer();
      input->GetCellType( 0 ); // Build cell representation so that Threads can access them safely
      ThreadsFunctor* my_contour = ThreadsFunctor::New();
      my_contour->SetData( input, newPts, inCd, inPd, vtkSMPMergePoints::SafeDownCast(this->Locator),
                           estimatedSize, values, numContours, inScalars, this->ComputeScalars,
                           newVerts, newLines, newPolys, outCd, outPd );


      // init
      MyInit* TheInit = MyInit::New();
      vtkSMP::Parallel( my_contour, TheInit );
      TheInit->Delete();
      timer->end_bench_timer();

      timer->start_bench_timer();
      for ( my_contour->dimensionality = 1; my_contour->dimensionality <= 3; ++(my_contour->dimensionality) )
        {
        vtkSMP::ForEach( 0, numCells, my_contour );
        }
      timer->end_bench_timer();

      // pre-merge
//      vtkSMP::Parallel( my_contour, 1 );
      timer->start_bench_timer();
      my_contour->PreMerge();

      vtkIdType numberOfCells = my_contour->GetNumberOfVerts() +
          my_contour->GetNumberOfLines() + my_contour->GetNumberOfPolys();

      newVerts->GetData()->Resize( my_contour->GetNumberOfVertTuples() );
      newLines->GetData()->Resize( my_contour->GetNumberOfLineTuples() );
      newPolys->GetData()->Resize( my_contour->GetNumberOfPolyTuples() );
      // Copy on itself means resize
      outPd->CopyAllocate( outPd, my_contour->GetNumberOfPoints(), my_contour->GetNumberOfPoints() );
      outCd->CopyAllocate( outCd, numberOfCells, numberOfCells );
      this->Locator->InitPointInsertion( 0, 0, my_contour->GetNumberOfPoints() ); //Rewrite with a resize meaning
      timer->end_bench_timer();

      // merge
      timer->start_bench_timer();
      MyMerge* TheMerge = MyMerge::New();
      vtkSMP::Parallel( my_contour, TheMerge );
      TheMerge->Delete();
      timer->end_bench_timer();

      // Correcting size of arrays
      this->Locator->InitPointInsertion( 0, 0, 0 );
      outPd->SetNumberOfTuples( newPts->GetNumberOfPoints() );
      outCd->SetNumberOfTuples( numberOfCells );
      newVerts->SetNumberOfCells( my_contour->GetNumberOfVerts() );
      newLines->SetNumberOfCells( my_contour->GetNumberOfLines() );
      newPolys->SetNumberOfCells( my_contour->GetNumberOfPolys() );

      my_contour->Delete();
      } //if using scalar tree
    else
      {
      // locator used to merge potentially duplicate points
      if ( this->Locator == NULL )
        {
        this->CreateDefaultLocator();
        }
      this->Locator->InitPointInsertion (newPts,
                                         input->GetBounds(),estimatedSize);

      // Move of previously deleted Allocations
      newVerts->Allocate( estimatedSize, estimatedSize );
      newLines->Allocate( estimatedSize, estimatedSize );
      newPolys->Allocate( estimatedSize, estimatedSize );
      outPd->InterpolateAllocate( inPd, estimatedSize, estimatedSize );
      outCd->CopyAllocate( inCd, estimatedSize, estimatedSize );
      cellScalars->SetNumberOfComponents( inScalars->GetNumberOfComponents() );
      cellScalars->Allocate( cellScalars->GetNumberOfComponents() * VTK_CELL_SIZE );

      vtkCell *cell;
      if ( this->ScalarTree == NULL )
        {
        this->ScalarTree = vtkSimpleScalarTree::New();
        }
      this->ScalarTree->SetDataSet(input);
      // Note: This will have problems when input contains 2D and 3D cells.
      // CellData will get scrabled because of the implicit ordering of
      // verts, lines and polys in vtkPolyData.  The solution
      // is to convert this filter to create unstructured grid.
      //
      // Loop over all contour values.  Then for each contour value,
      // loop over all cells.
      //
      for ( i=0; i < numContours; i++ )
        {
        for ( this->ScalarTree->InitTraversal(values[i]);
              (cell=this->ScalarTree->GetNextCell(cellId,cellPts,cellScalars)) != NULL; )
          {
          cell->Contour(values[i], cellScalars, this->Locator,
                        newVerts, newLines, newPolys, inPd, outPd,
                        inCd, cellId, outCd);

          } //for all cells
        } //for all contour values
      } //using scalar tree

    vtkDebugMacro(<<"Created: "
                  << newPts->GetNumberOfPoints() << " points, "
                  << newVerts->GetNumberOfCells() << " verts, "
                  << newLines->GetNumberOfCells() << " lines, "
                  << newPolys->GetNumberOfCells() << " triangles");

    // Update ourselves.  Because we don't know up front how many verts, lines,
    // polys we've created, take care to reclaim memory.
    //
    output->SetPoints(newPts);
    newPts->Delete();
    cellScalars->Delete();

    if (newVerts->GetNumberOfCells())
      {
      output->SetVerts(newVerts);
      }
    newVerts->Delete();

    if (newLines->GetNumberOfCells())
      {
      output->SetLines(newLines);
      }
    newLines->Delete();

    if (newPolys->GetNumberOfCells())
      {
      output->SetPolys(newPolys);
      }
    newPolys->Delete();

    this->Locator->Initialize();//releases leftover memory
    output->Squeeze();
    } //else if not vtkUnstructuredGrid

  return 1;
}
