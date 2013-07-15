/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMP2ContourFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSMP2ContourFilter.h"

#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkContourGrid.h"
#include "vtkContourValues.h"
#include "vtkCutter.h"
#include "vtkFunctorInitializable.h"
#include "vtkGarbageCollector.h"
#include "vtkGenericCell.h"
#include "vtkGridSynchronizedTemplates3D.h"
#include "vtkImageData.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkParallelOperators.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkRectilinearSynchronizedTemplates.h"
#include "vtkSimpleScalarTree.h"
#include "vtkSMPMergePoints.h"
#include "vtkSMPMinMaxTree.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkSynchronizedTemplates2D.h"
#include "vtkSynchronizedTemplates3D.h"
#include "vtkThreadLocal.h"
#include "vtkTimerLog.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkStandardNewMacro(vtkSMP2ContourFilter);
vtkCxxSetObjectMacro(vtkSMP2ContourFilter,ScalarTree,vtkScalarTree);

//==============================================================================
/* ================================================================================
  Generic contouring: Functors for parallel execution without ScalarTree
 ================================================================================ */
class ThreadsFunctor2 : public vtkFunctorInitializable
{
  //Description:
  //?
public:
  vtkTypeMacro(ThreadsFunctor2,vtkFunctorInitializable);
  static ThreadsFunctor2* New();
  void PrintSelf(ostream &os, vtkIndent indent)
  {
    this->Superclass::PrintSelf(os,indent);
  }

  //Description:
  //?
  int dimensionality;

  //Description:
  //?
  void SetData( vtkDataSet* _input, vtkIncrementalPointLocator* _locator,
                  vtkIdType& _size, double* _values, int _number,
                  vtkDataArray* _scalars, int _compute,
                vtkThreadLocal<vtkDataObject>* _outputs)
    {
    vtkCutter::GetCellTypeDimensions(cellTypeDimensions);

    this->input = _input;
    this->inPd = _input->GetPointData();
    this->inCd = _input->GetCellData();
    this->refLocator = _locator;
    this->estimatedSize = _size;
    this->values = _values;
    this->numContours = _number;
    this->inScalars = _scalars;
    this->computeScalars = _compute;

    this->outputs = _outputs;

    this->Init();
    }

  //Description:
  //?
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

    vtkPolyData* output = this->PolyOutputs->GetLocal();
    vtkPointData* outPd = output->GetPointData();
    vtkCellData* outCd = output->GetCellData();
    for (int i = 0; i < numContours; i++)
      {
      double v = values[i];
      cell->Contour( v, cellScalars, this->Locator->GetLocal( ),
                     this->newVerts->GetLocal( ), this->newLines->GetLocal( ),
                     this->newPolys->GetLocal( ), this->inPd, outPd,
                     this->inCd, cellId, outCd );
      }
    }

  //Description:
  //?
  void Init( ) const
    {
    vtkPolyData* output = vtkPolyData::SafeDownCast(this->outputs->NewLocal());
    this->PolyOutputs->SetLocal(output);

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

    vtkPointData *pd=output->GetPointData();
    if ( !this->computeScalars )
      {
      pd->CopyScalarsOff();
      }
    pd->InterpolateAllocate( this->inPd, this->estimatedSize, this->estimatedSize );

    vtkCellData *cd=output->GetCellData();
    cd->CopyAllocate( this->inCd, this->estimatedSize, this->estimatedSize );

    this->Cells->NewLocal( );

    vtkDataArray* cScalars = this->CellsScalars->NewLocal( this->inScalars );
    cScalars->SetNumberOfComponents( this->inScalars->GetNumberOfComponents() );
    cScalars->Allocate( cScalars->GetNumberOfComponents() * VTK_CELL_SIZE );

    Initialized();
    }

  //TODO: Can be private and marked not implemented?
  ThreadsFunctor2( const ThreadsFunctor2& );
  void operator =( const ThreadsFunctor2& );

  // for convenience sake, do not make accessors
  vtkThreadLocal<vtkPolyData>* PolyOutputs;
  vtkThreadLocal<vtkPoints>* newPts;
  vtkThreadLocal<vtkCellArray>* newVerts;
  vtkThreadLocal<vtkCellArray>* newLines;
  vtkThreadLocal<vtkCellArray>* newPolys;

protected:
  ThreadsFunctor2()
    {
    Locator = vtkThreadLocal<vtkIncrementalPointLocator>::New();
    newPts = vtkThreadLocal<vtkPoints>::New();
    newVerts = vtkThreadLocal<vtkCellArray>::New();
    newLines = vtkThreadLocal<vtkCellArray>::New();
    newPolys = vtkThreadLocal<vtkCellArray>::New();
    Cells = vtkThreadLocal<vtkGenericCell>::New();
    CellsScalars = vtkThreadLocal<vtkDataArray>::New();
    PolyOutputs = vtkThreadLocal<vtkPolyData>::New();
    }

  ~ThreadsFunctor2()
    {
    Locator->Delete();
    newPts->Delete();
    newVerts->Delete();
    newLines->Delete();
    newPolys->Delete();
    Cells->Delete();
    CellsScalars->Delete();
    PolyOutputs->Delete();
    }

  vtkThreadLocal<vtkDataObject>* outputs;
  vtkThreadLocal<vtkIncrementalPointLocator>* Locator;
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

  unsigned char cellTypeDimensions[VTK_NUMBER_OF_CELL_TYPES];
};

vtkStandardNewMacro(ThreadsFunctor2);

//==============================================================================
/* ================================================================================
  Generic contouring: Functors for parallel execution with ScalarTree
 ================================================================================ */
class AcceleratedFunctor2 : public ThreadsFunctor2
{
// Description:
// ?
public:
  vtkTypeMacro(AcceleratedFunctor2,ThreadsFunctor2);
  static AcceleratedFunctor2* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  // Description:
  // ?
  void operator ()( vtkIdType id ) const
    {
    vtkGenericCell* cell = this->Cells->GetLocal( );
    vtkDataArray* scalars = this->CellsScalars->GetLocal( );

    this->input->GetCell( id, cell );
    vtkIdList* cellPts = cell->GetPointIds();
    scalars->SetNumberOfTuples( cellPts->GetNumberOfIds() );
    this->inScalars->GetTuples( cellPts, scalars );

    vtkPolyData* output = this->PolyOutputs->GetLocal();
    cell->Contour( ScalarValue, scalars, this->Locator->GetLocal( ),
                   this->newVerts->GetLocal( ), this->newLines->GetLocal( ),
                   this->newPolys->GetLocal( ), this->inPd, output->GetPointData(),
                   this->inCd, id, output->GetCellData());
    }

  //TODO Can be private and marked not implemented?
  AcceleratedFunctor2( const AcceleratedFunctor2& );
  void operator =( const AcceleratedFunctor2& );

  //Description:
  //?
  double ScalarValue;

protected:
  AcceleratedFunctor2() { }
  ~AcceleratedFunctor2() { }
};

vtkStandardNewMacro(AcceleratedFunctor2);

//==============================================================================
// Construct object with initial range (0,1) and single contour value
// of 0.0.
vtkSMP2ContourFilter::vtkSMP2ContourFilter()
{
  this->ContourValues = vtkContourValues::New();

  this->ComputeNormals = 1;
  this->ComputeGradients = 0;
  this->ComputeScalars = 1;

  this->Locator = NULL;

  this->UseScalarTree = 0;
  this->ScalarTree = NULL;

  this->SynchronizedTemplates2D = vtkSynchronizedTemplates2D::New();
  this->SynchronizedTemplates3D = vtkSynchronizedTemplates3D::New();
  this->GridSynchronizedTemplates = vtkGridSynchronizedTemplates3D::New();
  this->RectilinearSynchronizedTemplates = vtkRectilinearSynchronizedTemplates::New();

  // by default process active point scalars
  this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,
                               vtkDataSetAttributes::SCALARS);

  this->GetInformation()->Set(vtkAlgorithm::PRESERVES_RANGES(), 1);
  this->GetInformation()->Set(vtkAlgorithm::PRESERVES_BOUNDS(), 1);
}

//------------------------------------------------------------------------------
vtkSMP2ContourFilter::~vtkSMP2ContourFilter()
{
  this->ContourValues->Delete();
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( this->ScalarTree )
    {
    this->ScalarTree->Delete();
    this->ScalarTree = 0;
    }
  this->SynchronizedTemplates2D->Delete();
  this->SynchronizedTemplates3D->Delete();
  this->GridSynchronizedTemplates->Delete();
  this->RectilinearSynchronizedTemplates->Delete();
}

//------------------------------------------------------------------------------
// Overload standard modified time function. If contour values are modified,
// then this object is modified as well.
unsigned long vtkSMP2ContourFilter::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  if (this->ContourValues)
    {
    time = this->ContourValues->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
  if (this->Locator)
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

//------------------------------------------------------------------------------
int vtkSMP2ContourFilter::RequestUpdateExtent(vtkInformation* request,
                                          vtkInformationVector** inputVector,
                                          vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input =
    vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numContours=this->ContourValues->GetNumberOfContours();
  double *values=this->ContourValues->GetValues();

  vtkInformation *fInfo =
    vtkDataObject::GetActiveFieldInformation(inInfo,
                                             vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                             vtkDataSetAttributes::SCALARS);
  int sType = VTK_DOUBLE;
  if (fInfo)
    {
    sType = fInfo->Get(vtkDataObject::FIELD_ARRAY_TYPE());
    }

  // handle 2D images
  int i;
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
      this->SynchronizedTemplates2D->SetComputeScalars(this->ComputeScalars);
      return this->SynchronizedTemplates2D->
        ProcessRequest(request,inputVector,outputVector);
      }
    else if (dim == 3)
      {
      this->SynchronizedTemplates3D->SetNumberOfContours(numContours);
      for (i=0; i < numContours; i++)
        {
        this->SynchronizedTemplates3D->SetValue(i,values[i]);
        }
      this->SynchronizedTemplates3D->SetComputeNormals(this->ComputeNormals);
      this->SynchronizedTemplates3D->SetComputeGradients(this->ComputeGradients);
      this->SynchronizedTemplates3D->SetComputeScalars(this->ComputeScalars);
      return this->SynchronizedTemplates3D->
        ProcessRequest(request,inputVector,outputVector);
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
      return this->RectilinearSynchronizedTemplates->
        ProcessRequest(request,inputVector,outputVector);
      }
    } //if 3D RGrid

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
      return this->GridSynchronizedTemplates->
        ProcessRequest(request,inputVector,outputVector);
      }
    } //if 3D SGrid

  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
  return 1;
}

//------------------------------------------------------------------------------
// General contouring filter.  Handles arbitrary input.
//
int vtkSMP2ContourFilter::RequestData(
    vtkInformation* vtkNotUsed(request),
    vtkInformationVector** inputVector,
    vtkInformationVector* vtkNotUsed(outputVector),
    vtkThreadLocal<vtkDataObject>** outputs)
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
    vtkWarningMacro(<< "vtkImageData input not supported");
    return 0;
    } //if image data

  // handle 3D RGrids
  if (vtkRectilinearGrid::SafeDownCast(input) && sType != VTK_BIT)
    {
    vtkWarningMacro(<< "vtkRectilinearGrid input not supported");
    return 0;
    } // if 3D Rgrid

  // handle 3D SGrids
  if (vtkStructuredGrid::SafeDownCast(input) && sType != VTK_BIT)
    {
    vtkWarningMacro(<< "vtkStructuredGrid input not supported");
    } //if 3D SGrid

  vtkDataArray *inScalars;
  vtkIdType numCells, estimatedSize;

  if (!outputs[0])
    {
    return 0;
    }

  vtkDebugMacro(<< "Executing contour filter");
  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID && !this->UseScalarTree)
    {
    vtkDebugMacro(<< "vtkUnstructuredGrid input not supported");
    return 0;
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

    // locator used to merge potentially duplicate points
    if ( this->Locator == NULL )
      {
      this->CreateDefaultLocator();
      }

    // If enabled, build a scalar tree to accelerate search
    //
    vtkSMPMinMaxTree* parallelTree = vtkSMPMinMaxTree::SafeDownCast(this->ScalarTree);
    if ( !this->UseScalarTree || parallelTree )
      {
      // Init (thread local init is drown into first ForEach)
      input->GetCellType( 0 ); // Build cell representation so that Threads can access them safely
      ThreadsFunctor2* my_contour;
      if ( this->UseScalarTree )
        {
        my_contour = AcceleratedFunctor2::New();
        }
      else
        {
        my_contour = ThreadsFunctor2::New();
        }
      my_contour->SetData( input, this->Locator, estimatedSize, values,
                           numContours, inScalars, this->ComputeScalars, outputs[0] );

      // Exec
      if ( this->UseScalarTree )
        {
        AcceleratedFunctor2* TreeContour = static_cast<AcceleratedFunctor2*>(my_contour);
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
      // No more merge in V2 but we finalize each output
      // Easier in sequential
      vtkThreadLocal<vtkPolyData>::iterator itOutput;
      vtkThreadLocal<vtkPoints>::iterator newPts;
      vtkThreadLocal<vtkCellArray>::iterator newVerts;
      vtkThreadLocal<vtkCellArray>::iterator newLines;
      vtkThreadLocal<vtkCellArray>::iterator newPolys;
      for ( itOutput = my_contour->PolyOutputs->Begin(),
            newPts = my_contour->newPts->Begin(),
            newVerts = my_contour->newVerts->Begin(),
            newLines = my_contour->newLines->Begin(),
            newPolys = my_contour->newPolys->Begin();
            itOutput != my_contour->PolyOutputs->End();
            ++itOutput, ++newPts, ++newVerts, ++newLines, ++newPolys)
        {
        vtkDebugMacro(<<"Created: "
                      << (*newPts)->GetNumberOfPoints() << " points, "
                      << (*newVerts)->GetNumberOfCells() << " verts, "
                      << (*newLines)->GetNumberOfCells() << " lines, "
                      << (*newPolys)->GetNumberOfCells() << " triangles");
        (*itOutput)->SetPoints(*newPts);

        if ((*newVerts)->GetNumberOfCells())
          {
          (*itOutput)->SetVerts(*newVerts);
          }

        if ((*newLines)->GetNumberOfCells())
          {
          (*itOutput)->SetLines(*newLines);
          }

        if ((*newPolys)->GetNumberOfCells())
          {
          (*itOutput)->SetPolys(*newPolys);
          }
        (*itOutput)->Squeeze();
        }

      my_contour->Delete();
      } //if using scalar tree
    else
      {
      vtkWarningMacro(<< "Scalar trees that are not a vtkSMPMinMaxTree not supported");
      return 0;
      } //using scalar tree

    this->Locator->Initialize();//releases leftover memory
    } //else if not vtkUnstructuredGrid

  return 1;
}

//------------------------------------------------------------------------------
// Specify a spatial locator for merging points. By default,
// an instance of vtkMergePoints is used.
void vtkSMP2ContourFilter::SetLocator(vtkIncrementalPointLocator *locator)
{
  if ( this->Locator == locator )
    {
    return;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( locator )
    {
    locator->Register(this);
    }
  this->Locator = locator;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkSMP2ContourFilter::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    this->Locator->Register(this);
    this->Locator->Delete();
    }
}

//------------------------------------------------------------------------------
void vtkSMP2ContourFilter::SetArrayComponent( int comp )
{
  this->SynchronizedTemplates2D->SetArrayComponent( comp );
  this->SynchronizedTemplates3D->SetArrayComponent( comp );
  this->RectilinearSynchronizedTemplates->SetArrayComponent( comp );
}

int vtkSMP2ContourFilter::GetArrayComponent()
{
  return( this->SynchronizedTemplates2D->GetArrayComponent() );
}

//----------------------------------------------------------------------------
int vtkSMP2ContourFilter::ProcessRequest(vtkInformation* request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector)
{
  // generate the data
  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT_INFORMATION()))
    {
    // compute the priority for this UE
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the range of the input if available
    vtkInformation *fInfo = NULL;
    vtkDataArray *inscalars = this->GetInputArrayToProcess(0, inputVector);
    if (inscalars)
      {
      vtkInformationVector *miv = inInfo->Get(vtkDataObject::POINT_DATA_VECTOR());
      for (int index = 0; index < miv->GetNumberOfInformationObjects(); index++)
        {
        vtkInformation *mInfo = miv->GetInformationObject(index);
        const char *minfo_arrayname =
          mInfo->Get(vtkDataObject::FIELD_ARRAY_NAME());
        if (minfo_arrayname && !strcmp(minfo_arrayname, inscalars->GetName()))
          {
          fInfo = mInfo;
          break;
          }
        }
      }
    else
      {
      fInfo = vtkDataObject::GetActiveFieldInformation
        (inInfo, vtkDataObject::FIELD_ASSOCIATION_POINTS,
         vtkDataSetAttributes::SCALARS);
      }

    if (!fInfo)
      {
      return 1;
      }

    double *range = fInfo->Get(vtkDataObject::PIECE_FIELD_RANGE());
    int numContours = this->ContourValues->GetNumberOfContours();
    if (range && numContours)
      {
      // compute the priority
      // get the incoming priority if any
      double inPriority = 1;
      if (inInfo->Has(vtkStreamingDemandDrivenPipeline::PRIORITY()))
        {
        inPriority = inInfo->Get(vtkStreamingDemandDrivenPipeline::PRIORITY());
        }
      outInfo->Set(vtkStreamingDemandDrivenPipeline::PRIORITY(),inPriority);
      if (!inPriority)
        {
        return 1;
        }

      // do any contours intersect the range?
      double *values=this->ContourValues->GetValues();
      int i;
      for (i=0; i < numContours; i++)
        {
        if (values[i] >= range[0] && values[i] <= range[1])
          {
          return 1;
          }
        }

      double inRes = 1.0;
      if (inInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_RESOLUTION()))
        {
        inRes = inInfo->Get
          (vtkStreamingDemandDrivenPipeline::UPDATE_RESOLUTION());
        }
      if (inRes >= 0.99)
        {
        outInfo->Set(vtkStreamingDemandDrivenPipeline::PRIORITY(),0.0);
        }
      else
        {
        outInfo->Set
          (vtkStreamingDemandDrivenPipeline::PRIORITY(),inPriority*0.1);
        }
      }
    return 1;
    }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//------------------------------------------------------------------------------
int vtkSMP2ContourFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//------------------------------------------------------------------------------
void vtkSMP2ContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Compute Gradients: "
     << (this->ComputeGradients ? "On\n" : "Off\n");
  os << indent << "Compute Normals: "
     << (this->ComputeNormals ? "On\n" : "Off\n");
  os << indent << "Compute Scalars: "
     << (this->ComputeScalars ? "On\n" : "Off\n");

  this->ContourValues->PrintSelf(os,indent.GetNextIndent());

  os << indent << "Use Scalar Tree: "
     << (this->UseScalarTree ? "On\n" : "Off\n");
  if ( this->ScalarTree )
    {
    os << indent << "Scalar Tree: " << this->ScalarTree << "\n";
    }
  else
    {
    os << indent << "Scalar Tree: (none)\n";
    }

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}

//----------------------------------------------------------------------------
void vtkSMP2ContourFilter::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  vtkGarbageCollectorReport(collector, this->ScalarTree, "ScalarTree");
}
