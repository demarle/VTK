/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkContourFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkContourFilter.h"

#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkContourGrid.h"
#include "vtkContourValues.h"
#include "vtkGarbageCollector.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSimpleScalarTree.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTimerLog.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkCxxRevisionMacro(vtkContourFilter, "1.110");
vtkStandardNewMacro(vtkContourFilter);
vtkCxxSetObjectMacro(vtkContourFilter,ScalarTree,vtkScalarTree);

// Construct object with initial range (0,1) and single contour value
// of 0.0.
vtkContourFilter::vtkContourFilter()
{
  this->ContourValues = vtkContourValues::New();

  this->ComputeNormals = 1;
  this->ComputeGradients = 0;
  this->ComputeScalars = 1;

  this->Locator = NULL;

  this->UseScalarTree = 0;
  this->ScalarTree = NULL;
  this->InputScalarsSelection = NULL;
}

vtkContourFilter::~vtkContourFilter()
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
  this->SetInputScalarsSelection(NULL);
}

// Overload standard modified time function. If contour values are modified,
// then this object is modified as well.
unsigned long vtkContourFilter::GetMTime()
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

int vtkContourFilter::ComputeInputUpdateExtent(vtkInformation*,
                                               vtkInformationVector** inputVector,
                                               vtkInformationVector*)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
  return 1;
}

// General contouring filter.  Handles arbitrary input.
//
int vtkContourFilter::RequestData(
  vtkInformation*,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkIdType cellId;
  int i, abortExecute=0;
  vtkIdList *cellPts;
  vtkDataArray *inScalars;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPts;
  vtkIdType numCells, estimatedSize;
  int numContours=this->ContourValues->GetNumberOfContours();
  double *values=this->ContourValues->GetValues();
  vtkDataArray *cellScalars;

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));
  if (!output) {return 0;}

  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!input) {return 0;}

  vtkPointData *inPd=input->GetPointData(), *outPd=output->GetPointData();
  vtkCellData *inCd=input->GetCellData(), *outCd=output->GetCellData();

  vtkDebugMacro(<< "Executing contour filter");
  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    vtkDebugMacro(<< "Processing unstructured grid");
    vtkContourGrid *cgrid;

    cgrid = vtkContourGrid::New();
    cgrid->SetInput((vtkUnstructuredGrid *)input);
    for (i = 0; i < numContours; i++)
      {
      cgrid->SetValue(i, values[i]);
      }
    cgrid->GetOutput()->SetUpdateExtent(output->GetUpdatePiece(),
                                        output->GetUpdateNumberOfPieces(),
                                        output->GetUpdateGhostLevel());
    cgrid->SelectInputScalars(this->InputScalarsSelection);
    cgrid->Update();
    output->ShallowCopy(cgrid->GetOutput());
    cgrid->SetInput(0);
    cgrid->Delete();
    } //if type VTK_UNSTRUCTURED_GRID
  else
    {
    numCells = input->GetNumberOfCells();
    inScalars = input->GetPointData()->GetScalars(this->InputScalarsSelection);
    if ( ! inScalars || numCells < 1 )
      {
      vtkDebugMacro(<<"No data to contour");
      return 1;
      }

    // Create objects to hold output of contour operation. First estimate
    // allocation size.
    //
    estimatedSize = (vtkIdType) pow ((double) numCells, .75);
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
      for (cellId=0; cellId < numCells && !abortExecute; cellId++)
        {
        input->GetCell(cellId,cell);
        cellPts = cell->GetPointIds();
        inScalars->GetTuples(cellPts,cellScalars);
        
        if ( ! (cellId % 5000) ) 
          {
          vtkDebugMacro(<<"Contouring #" << cellId);
          this->UpdateProgress ((double)cellId/numCells);
          abortExecute = this->GetAbortExecute();
          }
        
        for (i=0; i < numContours; i++)
          {
          cell->Contour(values[i], cellScalars, this->Locator, 
                        newVerts, newLines, newPolys, inPd, outPd,
                        inCd, cellId, outCd);
          
          } // for all contour values
        } // for all cells
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

// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkContourFilter::SetLocator(vtkPointLocator *locator)
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

void vtkContourFilter::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    this->Locator->Register(this);
    this->Locator->Delete();
    }
}

void vtkContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->InputScalarsSelection)
    {
    os << indent << "InputScalarsSelection: " 
       << this->InputScalarsSelection << endl;
    }

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
void vtkContourFilter::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  collector->ReportReference(this->ScalarTree, "ScalarTree");
}

//----------------------------------------------------------------------------
void vtkContourFilter::RemoveReferences()
{
  if(this->ScalarTree)
    {
    this->ScalarTree->Delete();
    this->ScalarTree = 0;
    }
  this->Superclass::RemoveReferences();
}

int vtkContourFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
