#include "vtkSMPContourFilter.h"
#include "vtkObjectFactory.h"
#include "vtkMergeDataSets.h"

#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkContourGrid.h"
#include "vtkCutter.h"
#include "vtkFunctorInitializable.h"
#include "vtkGenericCell.h"
#include "vtkGridSynchronizedTemplates3D.h"
#include "vtkImageData.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergePoints.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkRectilinearSynchronizedTemplates.h"
#include "vtkScalarTree.h"
#include "vtkSimpleScalarTree.h"
#include "vtkParallelOperators.h"
#include "vtkFunctorInitializable.h"
#include "vtkSMPMergePoints.h"
#include "vtkSMPMinMaxTree.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkSynchronizedTemplates2D.h"
#include "vtkSynchronizedTemplates3D.h"
#include "vtkUniformGrid.h"

vtkStandardNewMacro(vtkSMPContourFilter);

vtkSMPContourFilter::vtkSMPContourFilter() : vtkContourFilter() { }
vtkSMPContourFilter::~vtkSMPContourFilter() { }

void vtkSMPContourFilter::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

/* ================================================================================
  Generic contouring: Functors for parallel execution without ScalarTree
 ================================================================================ */
class ThreadsFunctor : public vtkFunctorInitializable
{
  ThreadsFunctor( const ThreadsFunctor& );
  void operator =( const ThreadsFunctor& );

protected:
  ThreadsFunctor()
    {
    Locator = vtkThreadLocal<vtkIncrementalPointLocator>::New();
    newPts = vtkThreadLocal<vtkPoints>::New();
    newVerts = vtkThreadLocal<vtkCellArray>::New();
    newLines = vtkThreadLocal<vtkCellArray>::New();
    newPolys = vtkThreadLocal<vtkCellArray>::New();
    outPd = vtkThreadLocal<vtkPointData>::New();
    outCd = vtkThreadLocal<vtkCellData>::New();
    Cells = vtkThreadLocal<vtkGenericCell>::New();
    CellsScalars = vtkThreadLocal<vtkDataArray>::New();
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
    }

public:
  vtkTypeMacro(ThreadsFunctor,vtkFunctorInitializable);
  static ThreadsFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
  {
    this->Superclass::PrintSelf(os,indent);
  }

  vtkThreadLocal<vtkIncrementalPointLocator>* Locator;
  vtkThreadLocal<vtkPoints>* newPts;
  vtkThreadLocal<vtkCellArray>* newVerts;
  vtkThreadLocal<vtkCellArray>* newLines;
  vtkThreadLocal<vtkCellArray>* newPolys;
  vtkThreadLocal<vtkPointData>* outPd;
  vtkThreadLocal<vtkCellData>* outCd;
  vtkThreadLocal<vtkGenericCell>* Cells;
  vtkThreadLocal<vtkDataArray>* CellsScalars;

  vtkDataArray* inScalars;
  vtkDataSet* input;
  vtkPointData* inPd;
  vtkCellData* inCd;
  vtkIdType estimatedSize;
  double* values;
  int numContours;
  int computeScalars;

  vtkIncrementalPointLocator* refLocator;
  vtkCellArray* outputVerts;
  vtkCellArray* outputLines;
  vtkCellArray* outputPolys;
  vtkCellData* outputCd;
  vtkPointData* outputPd;

  unsigned char cellTypeDimensions[VTK_NUMBER_OF_CELL_TYPES];

  int dimensionality;

  void SetData( vtkDataSet* _input, vtkPoints* _inPts, vtkCellData* _incd,
                  vtkPointData* _inpd, vtkIncrementalPointLocator* _locator,
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
    Locator->SetLocal( _locator );
    newPts->SetLocal( _inPts );
    newVerts->SetLocal( _outputVerts );
    newLines->SetLocal( _outputLines );
    newPolys->SetLocal( _outputPolys );
    outPd->SetLocal( _outputPd );
    outCd->SetLocal( _outputCd );
    Cells->NewLocal( );
    vtkDataArray* cScalars = CellsScalars->NewLocal( inScalars );
    cScalars->SetNumberOfComponents( inScalars->GetNumberOfComponents() );
    cScalars->Allocate( cScalars->GetNumberOfComponents() * VTK_CELL_SIZE );

    Initialized();
    }

  void operator ()( vtkIdType cellId ) const
    {
    vtkGenericCell *cell = this->Cells->GetLocal( );
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
    vtkDataArray* cellScalars = this->CellsScalars->GetLocal( );
    if ( cellScalars->GetSize() / cellScalars->GetNumberOfComponents() < cellPts->GetNumberOfIds() )
      {
      cellScalars->Allocate(cellScalars->GetNumberOfComponents()*cellPts->GetNumberOfIds());
      }
    inScalars->GetTuples(cellPts,cellScalars);

    for (int i = 0; i < numContours; i++)
      {
      double v = values[i];
      cell->Contour( v, cellScalars, this->Locator->GetLocal( ),
                     newVerts->GetLocal( ), newLines->GetLocal( ), newPolys->GetLocal( ),
                     inPd, outPd->GetLocal( ), inCd, cellId, outCd->GetLocal( ) );
      }
    }

  void Init( ) const
    {
    vtkPoints* pts = this->newPts->NewLocal( );
    pts->Allocate( this->estimatedSize, this->estimatedSize );
    vtkIncrementalPointLocator* l = this->Locator->NewLocal( this->refLocator );
    l->InitPointInsertion( pts, this->input->GetBounds(), this->estimatedSize );

    vtkCellArray* c = this->newVerts->NewLocal( );
    c->Allocate( this->estimatedSize, this->estimatedSize );
    c = this->newLines->NewLocal( );
    c->Allocate( this->estimatedSize, this->estimatedSize );
    c = this->newPolys->NewLocal( );
    c->Allocate( this->estimatedSize, this->estimatedSize );

    vtkPointData* pd = this->outPd->NewLocal( );
    if ( !this->computeScalars )
      {
      pd->CopyScalarsOff();
      }
    pd->InterpolateAllocate( this->inPd, this->estimatedSize, this->estimatedSize );

    vtkCellData* cd = this->outCd->NewLocal( );
    cd->CopyAllocate( this->inCd, this->estimatedSize, this->estimatedSize );

    this->Cells->NewLocal( );

    vtkDataArray* cScalars = this->CellsScalars->NewLocal( this->inScalars );
    cScalars->SetNumberOfComponents( this->inScalars->GetNumberOfComponents() );
    cScalars->Allocate( cScalars->GetNumberOfComponents() * VTK_CELL_SIZE );

    Initialized();
    }
};

vtkStandardNewMacro(ThreadsFunctor);

/* ================================================================================
  Generic contouring: Functors for parallel execution with ScalarTree
 ================================================================================ */
class AcceleratedFunctor : public ThreadsFunctor
{
  AcceleratedFunctor( const AcceleratedFunctor& );
  void operator =( const AcceleratedFunctor& );

protected:
  AcceleratedFunctor() { }
  ~AcceleratedFunctor() { }

public:
  vtkTypeMacro(AcceleratedFunctor,ThreadsFunctor);
  static AcceleratedFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  double ScalarValue;

  void operator ()( vtkIdType id ) const
    {
    vtkGenericCell* cell = this->Cells->GetLocal( );
    vtkDataArray* scalars = this->CellsScalars->GetLocal( );

    this->input->GetCell( id, cell );
    vtkIdList* cellPts = cell->GetPointIds();
    scalars->SetNumberOfTuples( cellPts->GetNumberOfIds() );
    this->inScalars->GetTuples( cellPts, scalars );

    cell->Contour( ScalarValue, scalars, this->Locator->GetLocal( ),
                   this->newVerts->GetLocal( ), this->newLines->GetLocal( ), this->newPolys->GetLocal( ),
                   this->inPd, this->outPd->GetLocal( ),
                   this->inCd, id, this->outCd->GetLocal( ));
    }
};

vtkStandardNewMacro(AcceleratedFunctor);

/* ================================================================================
 General contouring filter.  Handles arbitrary input.
 ================================================================================ */
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
  // Do not handle UnStructuredGrid since ForEach can't apply.
  // vtkContourGrid iterates over cells in a non-independant way

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
  this->Locator->InitPointInsertion (newPts, input->GetBounds(),estimatedSize);

  // interpolate data along edge
  // if we did not ask for scalars to be computed, don't copy them
  if (!this->ComputeScalars)
    {
    outPd->CopyScalarsOff();
    }

  // If enabled, build a scalar tree to accelerate search
  //
  vtkSMPMinMaxTree* parallelTree = vtkSMPMinMaxTree::SafeDownCast(this->ScalarTree);
  if ( !this->UseScalarTree || parallelTree )
    {
    vtkSMPMergePoints* parallelLocator = vtkSMPMergePoints::SafeDownCast( this->Locator );

    // Init (thread local init is drown into first ForEach)
    input->GetCellType( 0 ); // Build cell representation so that Threads can access them safely
    ThreadsFunctor* my_contour;
    if ( this->UseScalarTree )
      my_contour = AcceleratedFunctor::New();
    else
      my_contour = ThreadsFunctor::New();
    my_contour->SetData( input, newPts, inCd, inPd, this->Locator,
                         estimatedSize, values, numContours, inScalars, this->ComputeScalars,
                         newVerts, newLines, newPolys, outCd, outPd );

    // Exec
    if ( this->UseScalarTree )
      {
      AcceleratedFunctor* TreeContour = static_cast<AcceleratedFunctor*>(my_contour);
      parallelTree->SetDataSet(input);
      for ( i = 0; i < numContours; ++i )
        {
        TreeContour->ScalarValue = values[i];
        parallelTree->InitTraversal( values[i] );
        vtkParallelOperators::Traverse( parallelTree, TreeContour );
        }
      }
    else
      {
      for ( my_contour->dimensionality = 1; my_contour->dimensionality <= 3; ++(my_contour->dimensionality) )
        {
        vtkParallelOperators::ForEach( 0, numCells, my_contour );
        }
      }
    // Merge
    vtkMergeDataSets* mergeOp = vtkMergeDataSets::New();
    mergeOp->MasterThreadPopulatedOutputOn();
    if ( parallelLocator )
      {
      vtkThreadLocal<vtkSMPMergePoints>* SMPLocator = vtkThreadLocal<vtkSMPMergePoints>::New();
      my_contour->Locator->FillDerivedThreadLocal(SMPLocator);
      mergeOp->MergePolyData(
          parallelLocator, SMPLocator,
          outPd, my_contour->outPd,
          newVerts, my_contour->newVerts,
          newLines, my_contour->newLines,
          newPolys, my_contour->newPolys,
          0, 0, outCd, my_contour->outCd);
      SMPLocator->Delete();
      }
    else
      {
      mergeOp->MergePolyData(
          newPts, my_contour->newPts, input->GetBounds(),
          outPd, my_contour->outPd,
          newVerts, my_contour->newVerts,
          newLines, my_contour->newLines,
          newPolys, my_contour->newPolys,
          0, 0, outCd, my_contour->outCd);
      }
    mergeOp->Delete();
    my_contour->Delete();
    } //if using scalar tree
  else
    {
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

  return 1;
}
