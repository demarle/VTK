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

vtkStandardNewMacro(vtkSMPContourFilter);

vtkSMPContourFilter::vtkSMPContourFilter() : vtkContourFilter() { }
vtkSMPContourFilter::~vtkSMPContourFilter() { }

void vtkSMPContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

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
  int abortExecute=0;
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
    newVerts->Allocate(estimatedSize,estimatedSize);
    newLines = vtkCellArray::New();
    newLines->Allocate(estimatedSize,estimatedSize);
    newPolys = vtkCellArray::New();
    newPolys->Allocate(estimatedSize,estimatedSize);
    cellScalars = inScalars->NewInstance();
    cellScalars->SetNumberOfComponents(inScalars->GetNumberOfComponents());
    cellScalars->Allocate(cellScalars->GetNumberOfComponents()*VTK_CELL_SIZE);

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
    outPd->InterpolateAllocate(inPd,estimatedSize,estimatedSize);
    outCd->CopyAllocate(inCd,estimatedSize,estimatedSize);

    // If enabled, build a scalar tree to accelerate search
    //
    if ( !this->UseScalarTree )
      {
      vtkGenericCell *cell = vtkGenericCell::New();
      // Three passes over the cells to process lower dimensional cells first.
      // For poly data output cells need to be added in the order:
      // verts, lines and then polys, or cell data gets mixed up.
      // A better solution is to have an unstructured grid output.
      // I create a table that maps cell type to cell dimensionality,
      // because I need a fast way to get cell dimensionality.
      // This assumes GetCell is slow and GetCellType is fast.
      // I do not like hard coding a list of cell types here,
      // but I do not want to add GetCellDimension(vtkIdType cellId)
      // to the vtkDataSet API.  Since I anticipate that the output
      // will change to vtkUnstructuredGrid.  This temporary solution
      // is acceptable.
      //
      int cellType;
      unsigned char cellTypeDimensions[VTK_NUMBER_OF_CELL_TYPES];
      vtkCutter::GetCellTypeDimensions(cellTypeDimensions);
      int dimensionality;
      // We skip 0d cells (points), because they cannot be cut (generate no data).
      for (dimensionality = 1; dimensionality <= 3; ++dimensionality)
        {
        // Loop over all cells; get scalar values for all cell points
        // and process each cell.
        //
        for (cellId=0; cellId < numCells && !abortExecute; cellId++)
          {
          // I assume that "GetCellType" is fast.
          cellType = input->GetCellType(cellId);
          if (cellType >= VTK_NUMBER_OF_CELL_TYPES)
            { // Protect against new cell types added.
            vtkErrorMacro("Unknown cell type " << cellType);
            continue;
            }
          if (cellTypeDimensions[cellType] != dimensionality)
            {
            continue;
            }
          input->GetCell(cellId,cell);
          cellPts = cell->GetPointIds();
          if (cellScalars->GetSize()/cellScalars->GetNumberOfComponents() <
            cellPts->GetNumberOfIds())
            {
            cellScalars->Allocate(
              cellScalars->GetNumberOfComponents()*cellPts->GetNumberOfIds());
            }
          inScalars->GetTuples(cellPts,cellScalars);

          if (dimensionality == 3 &&  ! (cellId % 5000) )
            {
            vtkDebugMacro(<<"Contouring #" << cellId);
            this->UpdateProgress (static_cast<double>(cellId)/numCells);
            abortExecute = this->GetAbortExecute();
            }

          for (i=0; i < numContours; i++)
            {
            cell->Contour(values[i], cellScalars, this->Locator,
                          newVerts, newLines, newPolys, inPd, outPd,
                          inCd, cellId, outCd);

            } // for all contour values
          } // for all cells
        } // for all dimensions
      cell->Delete();
      } //if using scalar tree
    else
      {
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
