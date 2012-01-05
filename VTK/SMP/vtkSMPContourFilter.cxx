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

vtkStandardNewMacro(vtkSMPContourFilter);

vtkSMPContourFilter::vtkSMPContourFilter() : vtkContourFilter() { }
vtkSMPContourFilter::~vtkSMPContourFilter() { }

void vtkSMPContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

class Reducer : public vtkObject
{
  Reducer( const Reducer& );
  void operator =( const Reducer& );
  vtkIdType value;
  vtkMutexLock* Lock;

protected:
  Reducer() { value = 0; Lock = vtkMutexLock::New(); }
  ~Reducer() { Lock->Delete(); }

public:
  vtkTypeMacro(Reducer,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent)
  {
    this->Superclass::PrintSelf( os, indent );
    os << indent << "Value: " << value << endl;
  }
  static Reducer* New();

  void IncrementValue ( vtkIdType v )
  {
//    this->Lock->Lock();
    value += v;
//    this->Lock->Unlock();
  }

  vtkIdType GetValue() { return value; }
};

vtkStandardNewMacro(Reducer);

class OffsetManager : public vtkObject
{
  OffsetManager( const OffsetManager& );
  void operator =( const OffsetManager& );
  vtkstd::map<vtkSMPThreadID, vtkIdType> cells;
  vtkstd::map<vtkSMPThreadID, vtkIdType> tuples;
  vtkIdType CellsOffset;
  vtkIdType TuplesOffset;
  vtkMutexLock* Lock;
protected:
  OffsetManager() { CellsOffset = 0; TuplesOffset = 0; Lock = vtkMutexLock::New(); }
  ~OffsetManager() { Lock->Delete(); }
public:
  vtkTypeMacro(OffsetManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent)
  {
    this->Superclass::PrintSelf( os, indent );
    os << indent << "Offsets for cells index: " << endl;
    for ( vtkstd::map<vtkSMPThreadID, vtkIdType>::iterator it = cells.begin();
          it != cells.end(); ++it )
      {
      os << indent.GetNextIndent() << "Thread " << it->first << ": " << it->second << endl;
      }
    os << indent << "Offsets for tuples: " << endl;
    for ( vtkstd::map<vtkSMPThreadID, vtkIdType>::iterator it = tuples.begin();
          it != tuples.end(); ++it )
      {
      os << indent.GetNextIndent() << "Thread " << it->first << ": " << it->second << endl;
      }
    os << indent << "Number of cells: " << CellsOffset << endl;
    os << indent << "Number of tuples: " << TuplesOffset << endl;
  }
  static OffsetManager* New();

  void ManageValues ( vtkSMPThreadID tid, vtkCellArray* ca )
  {
    vtkIdType c = ca->GetNumberOfCells();
    vtkIdType t = ca->GetData()->GetNumberOfTuples();
    this->Lock->Lock();
    cells[tid] = CellsOffset;
    tuples[tid] = TuplesOffset;
    CellsOffset += c;
    TuplesOffset += t;
    this->Lock->Unlock();
  }

  vtkIdType GetNumberOfCells() { return CellsOffset; }
  vtkIdType GetNumberOfTuples() { return TuplesOffset; }
  vtkIdType GetCellsOffset ( vtkSMPThreadID tid ) { return cells[tid]; }
  vtkIdType GetTuplesOffset ( vtkSMPThreadID tid ) { return tuples[tid]; }
};

vtkStandardNewMacro(OffsetManager);

struct ThreadsFunctor : public vtkFunctorInitialisable, public vtkMergeableInitialisable {
  vtkSMP::vtkThreadLocal* Locator;
  vtkSMP::vtkThreadLocal* newPts;
  vtkSMP::vtkThreadLocal* newVerts;
  vtkSMP::vtkThreadLocal* newLines;
  vtkSMP::vtkThreadLocal* newPolys;
  vtkSMP::vtkThreadLocal* outPd;
  vtkSMP::vtkThreadLocal* outCd;
  vtkSMP::vtkThreadLocal* Cells;
  vtkSMP::vtkThreadLocal* CellsScalars;

  unsigned char cellTypeDimensions[VTK_NUMBER_OF_CELL_TYPES];
  int dimensionality;

  vtkDataArray* inScalars;
  vtkDataSet* input;
  vtkPointData* inPd;
  vtkCellData* inCd;
  vtkIdType estimatedSize;
  double* values;
  int numContours;
  int computeScalars;
  Reducer* numberOfPoints;

  OffsetManager* vertOffset;
  OffsetManager* lineOffset;
  OffsetManager* polyOffset;

  vtkIncrementalPointLocator* refLocator;
  vtkCellArray* outputVerts;
  vtkCellArray* outputLines;
  vtkCellArray* outputPolys;
  vtkCellData* outputCd;
  vtkPointData* outputPd;

  vtkMutexLock* Lock;

  ThreadsFunctor ( vtkDataSet* _input, vtkCellData* _incd,
                   vtkPointData* _inpd, vtkIncrementalPointLocator* _locator,
                   vtkIdType& _size, double* _values, int _number,
                   vtkDataArray* _scalars, int _compute, vtkCellArray* _outputVerts,
                   vtkCellArray* _outputLines, vtkCellArray* _outputPolys,
                   vtkCellData* _outputCd, vtkPointData* _outputPd )
  {
    Locator = vtkSMP::vtkThreadLocal::New();
    newPts = vtkSMP::vtkThreadLocal::New();
    newVerts = vtkSMP::vtkThreadLocal::New();
    newLines = vtkSMP::vtkThreadLocal::New();
    newPolys = vtkSMP::vtkThreadLocal::New();
    outPd = vtkSMP::vtkThreadLocal::New();
    outCd = vtkSMP::vtkThreadLocal::New();
    Cells = vtkSMP::vtkThreadLocal::New();
    CellsScalars = vtkSMP::vtkThreadLocal::New();

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

    this->Lock = vtkMutexLock::New();

    outputVerts = _outputVerts;
    outputLines = _outputLines;
    outputPolys = _outputPolys;
    outputCd = _outputCd;
    outputPd = _outputPd;

    numberOfPoints = Reducer::New();
    vertOffset = OffsetManager::New();
    lineOffset = OffsetManager::New();
    polyOffset = OffsetManager::New();
  }

  void init ( vtkSMPThreadID tid ) const
  {
    this->Lock->Lock();
    cout << tid << endl;
    vtkPoints::New();
    vtkPoints* pts = newPts->NewLocal<vtkPoints>( tid );
    pts->Allocate(estimatedSize,estimatedSize);
//    this->Lock->Lock();
    vtkIncrementalPointLocator* l = Locator->NewLocal<vtkIncrementalPointLocator>( tid, refLocator );
//    this->Lock->Unlock();
    l->InitPointInsertion( pts, input->GetBounds(), estimatedSize );

    vtkCellArray* c = newVerts->NewLocal<vtkCellArray>( tid );
    c->Allocate(estimatedSize,estimatedSize);

    c = newLines->NewLocal<vtkCellArray>( tid );
    c->Allocate(estimatedSize,estimatedSize);

    c = newPolys->NewLocal<vtkCellArray>( tid );
    c->Allocate(estimatedSize,estimatedSize);

    vtkPointData* pd = outPd->NewLocal<vtkPointData>( tid );
    if (!computeScalars)
      {
      pd->CopyScalarsOff();
      }
//    this->Lock->Lock();
    pd->InterpolateAllocate( inPd, estimatedSize, estimatedSize );
//    this->Lock->Unlock();

    vtkCellData* cd = outCd->NewLocal<vtkCellData>( tid );
//    this->Lock->Lock();
    cd->CopyAllocate( inCd, estimatedSize, estimatedSize );
//    this->Lock->Unlock();

    Cells->NewLocal<vtkGenericCell>( tid );

    vtkDataArray* cScalars = CellsScalars->NewLocal<vtkDataArray>( tid, inScalars );
    cScalars->SetNumberOfComponents(inScalars->GetNumberOfComponents());
    cScalars->Allocate(cScalars->GetNumberOfComponents()*VTK_CELL_SIZE);

    cout << -tid << endl;
    this->Lock->Unlock();
  }

  void operator ()( vtkIdType cellId, vtkSMPThreadID tid ) const
  {
    vtkGenericCell *cell = vtkGenericCell::SafeDownCast(this->Cells->GetLocal( tid ));
    this->Lock->Lock();
    int cellType = input->GetCellType(cellId);
    this->Lock->Unlock();
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
    vtkDataArray* cellScalars = vtkDataArray::SafeDownCast(this->CellsScalars->GetLocal( tid ));
    if (cellScalars->GetSize()/cellScalars->GetNumberOfComponents() < cellPts->GetNumberOfIds())
      {
      cellScalars->Allocate(cellScalars->GetNumberOfComponents()*cellPts->GetNumberOfIds());
      }
    inScalars->GetTuples(cellPts,cellScalars);

    for (int i = 0; i < numContours; i++)
      {
      double v = values[i];
      cell->Contour(v, cellScalars,
                    vtkIncrementalPointLocator::SafeDownCast(this->Locator->GetLocal( tid )),
                    vtkCellArray::SafeDownCast(newVerts->GetLocal( tid )),
                    vtkCellArray::SafeDownCast(newLines->GetLocal( tid )),
                    vtkCellArray::SafeDownCast(newPolys->GetLocal( tid )),
                    inPd, vtkPointData::SafeDownCast(outPd->GetLocal( tid )),
                    inCd, cellId, vtkCellData::SafeDownCast(outCd->GetLocal( tid )));
      }
  }

  void Delete()
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

    Lock->Delete();

    vertOffset->Delete();
    lineOffset->Delete();
    polyOffset->Delete();
    numberOfPoints->Delete();
  }

  void pre_merge ( vtkSMPThreadID tid ) const
  {
    vertOffset->ManageValues( tid, vtkCellArray::SafeDownCast(this->newVerts->GetLocal( tid )) );
    lineOffset->ManageValues( tid, vtkCellArray::SafeDownCast(this->newLines->GetLocal( tid )) );
    polyOffset->ManageValues( tid, vtkCellArray::SafeDownCast(this->newPolys->GetLocal( tid )) );

    numberOfPoints->IncrementValue(vtkPoints::SafeDownCast( this->newPts->GetLocal( tid ) )->GetNumberOfPoints());
  }

  void merge( vtkSMPThreadID tid ) const
    {
    vtkPointData* ptData = vtkPointData::SafeDownCast(this->outPd->GetLocal( tid ));
    vtkCellData* clData = vtkCellData::SafeDownCast(this->outCd->GetLocal( tid ));

    vtkPoints* computedPoints = vtkPoints::SafeDownCast(this->newPts->GetLocal( tid ));
    vtkIdType n = computedPoints->GetNumberOfPoints(), newId, clIndex = -1;
    vtkstd::vector<vtkIdType> map( n );
    for ( vtkIdType i = 0; i < n; ++i )
      {
      double* pt = computedPoints->GetPoint( i );
      this->Lock->Lock();
      int inserted = refLocator->InsertUniquePoint( pt, newId );
      this->Lock->Unlock();
      if ( inserted )
        outputPd->SetTuple( newId, i, ptData );
      map[i] = newId;
      }

    vtkIdType *pts, totalNumber;
    vtkCellArray* computedCells = vtkCellArray::SafeDownCast(this->newVerts->GetLocal( tid ));
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

    computedCells = vtkCellArray::SafeDownCast(this->newLines->GetLocal( tid ));
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

    computedCells = vtkCellArray::SafeDownCast(this->newPolys->GetLocal( tid ));
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

    // locator used to merge potentially duplicate points
    if ( this->Locator == NULL )
      {
      this->CreateDefaultLocator();
      }
    this->Locator->InitPointInsertion (newPts,
                                       input->GetBounds(),estimatedSize);

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
      input->ComputeBounds();
      ThreadsFunctor my_contour( input, inCd, inPd, this->Locator,
                                 estimatedSize, values, numContours,
                                 inScalars, this->ComputeScalars,
                                 newVerts, newLines, newPolys, outCd, outPd  );

      for ( my_contour.dimensionality = 1; my_contour.dimensionality <= 3; ++(my_contour.dimensionality) )
        {
        vtkSMP::ForEach( 0, numCells, my_contour );
        }

      vtkSMP::PreMerge( my_contour );

      vtkIdType numberOfCells = my_contour.vertOffset->GetNumberOfCells() +
          my_contour.lineOffset->GetNumberOfCells() + my_contour.polyOffset->GetNumberOfCells();

      newVerts->GetData()->SetNumberOfTuples(my_contour.vertOffset->GetNumberOfTuples());
      newLines->GetData()->SetNumberOfTuples(my_contour.lineOffset->GetNumberOfTuples());
      newPolys->GetData()->SetNumberOfTuples(my_contour.polyOffset->GetNumberOfTuples());

      outPd->InterpolateAllocate( inPd, my_contour.numberOfPoints->GetValue(), my_contour.numberOfPoints->GetValue() );
      outCd->CopyAllocate( inCd, numberOfCells, numberOfCells );

      vtkSMP::Merge( my_contour );

      // Correcting size of arrays
      outPd->SetNumberOfTuples( my_contour.numberOfPoints->GetValue() );
      outCd->SetNumberOfTuples( numberOfCells );
      newVerts->SetNumberOfCells( my_contour.vertOffset->GetNumberOfCells() );
      newLines->SetNumberOfCells( my_contour.lineOffset->GetNumberOfCells() );
      newPolys->SetNumberOfCells( my_contour.polyOffset->GetNumberOfCells() );

      my_contour.Delete();

      } //if using scalar tree
    else
      {
      // Move of previously deleted Allocations
      newVerts->Allocate(estimatedSize,estimatedSize);
      newLines->Allocate(estimatedSize,estimatedSize);
      newPolys->Allocate(estimatedSize,estimatedSize);
      outPd->InterpolateAllocate(inPd,estimatedSize,estimatedSize);
      outCd->CopyAllocate(inCd,estimatedSize,estimatedSize);
      cellScalars->SetNumberOfComponents(inScalars->GetNumberOfComponents());
      cellScalars->Allocate(cellScalars->GetNumberOfComponents()*VTK_CELL_SIZE);

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
      for (i=0; i < numContours; i++)
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
