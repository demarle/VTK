/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataSet.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDataSet.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkGenericCell.h"
#include "vtkIdList.h"
#include "vtkPointData.h"
#include "vtkSource.h"

#include <math.h>

vtkCxxRevisionMacro(vtkDataSet, "1.92");

//----------------------------------------------------------------------------
// Constructor with default bounds (0,1, 0,1, 0,1).
vtkDataSet::vtkDataSet ()
{
  this->Bounds[0] = VTK_LARGE_FLOAT;
  this->Bounds[1] = -VTK_LARGE_FLOAT;
  this->Bounds[2] = VTK_LARGE_FLOAT;
  this->Bounds[3] = -VTK_LARGE_FLOAT;
  this->Bounds[4] = VTK_LARGE_FLOAT;
  this->Bounds[5] = -VTK_LARGE_FLOAT;

  this->PointData = vtkPointData::New();
  this->CellData = vtkCellData::New();
  this->ScalarRange[0] = 0.0;
  this->ScalarRange[1] = 1.0;
}

//----------------------------------------------------------------------------
vtkDataSet::~vtkDataSet ()
{
  this->PointData->Delete();
  this->CellData->Delete();
}

//----------------------------------------------------------------------------
void vtkDataSet::Initialize()
{
  // We don't modify ourselves because the "ReleaseData" methods depend upon
  // no modification when initialized.
  vtkDataObject::Initialize();

  this->CellData->Initialize();
  this->PointData->Initialize();
}

//----------------------------------------------------------------------------
// Compute the data bounding box from data points.
void vtkDataSet::ComputeBounds()
{
  int j;
  vtkIdType i;
  float *x;

  if ( this->GetMTime() > this->ComputeTime )
    {
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] =  VTK_LARGE_FLOAT;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_LARGE_FLOAT;
    for (i=0; i<this->GetNumberOfPoints(); i++)
      {
      x = this->GetPoint(i);
      for (j=0; j<3; j++)
        {
        if ( x[j] < this->Bounds[2*j] )
          {
          this->Bounds[2*j] = x[j];
          }
        if ( x[j] > this->Bounds[2*j+1] )
          {
          this->Bounds[2*j+1] = x[j];
          }
        }
      }

    this->ComputeTime.Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDataSet::GetScalarRange(double range[2])
{
  vtkDataArray *ptScalars, *cellScalars;
  ptScalars = this->PointData->GetScalars();
  cellScalars = this->CellData->GetScalars();
  
  if ( ptScalars && cellScalars)
    {
    double r1[2], r2[2];
    ptScalars->GetRange(r1,0);
    cellScalars->GetRange(r2,0);
    range[0] = (r1[0] < r2[0] ? r1[0] : r2[0]);
    range[1] = (r1[1] > r2[1] ? r1[1] : r2[1]);
    }
  else if ( ptScalars )
    {
    ptScalars->GetRange(range,0);
    }
  else if ( cellScalars )
    {
    cellScalars->GetRange(range,0);
    }
  else
    {
    range[0] = 0.0;
    range[1] = 1.0;
    }
}

//----------------------------------------------------------------------------
double *vtkDataSet::GetScalarRange()
{
  this->GetScalarRange(this->ScalarRange);
  return this->ScalarRange;
}

//----------------------------------------------------------------------------
// Return a pointer to the geometry bounding box in the form
// (xmin,xmax, ymin,ymax, zmin,zmax).
float *vtkDataSet::GetBounds()
{
  this->ComputeBounds();
  return this->Bounds;
}
  
//----------------------------------------------------------------------------
void vtkDataSet::GetBounds(float bounds[6])
{
  this->ComputeBounds();
  for (int i=0; i<6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}
  
//----------------------------------------------------------------------------
// Get the center of the bounding box.
float *vtkDataSet::GetCenter()
{
  this->ComputeBounds();
  for (int i=0; i<3; i++)
    {
    this->Center[i] = (this->Bounds[2*i+1] + this->Bounds[2*i]) / 2.0;
    }
  return this->Center;
}

//----------------------------------------------------------------------------
void vtkDataSet::GetCenter(float center[3])
{
  this->ComputeBounds();
  for (int i=0; i<3; i++)
    {
    center[i] = (this->Bounds[2*i+1] + this->Bounds[2*i]) / 2.0;
    }
}
  
//----------------------------------------------------------------------------
// Return the length of the diagonal of the bounding box.
float vtkDataSet::GetLength()
{
  double diff, l=0.0;
  int i;

  this->ComputeBounds();
  for (i=0; i<3; i++)
    {
    diff = static_cast<double>(this->Bounds[2*i+1]) - 
      static_cast<double>(this->Bounds[2*i]);
    l += diff * diff;
    }
  diff = sqrt(l);
  if(diff > VTK_LARGE_FLOAT)
    {
    return VTK_LARGE_FLOAT;
    }
  return static_cast<float>(diff);
}

//----------------------------------------------------------------------------
unsigned long int vtkDataSet::GetMTime()
{
  unsigned long mtime, result;
  
  result = vtkDataObject::GetMTime();
  
  mtime = this->PointData->GetMTime();
  result = ( mtime > result ? mtime : result );

  mtime = this->CellData->GetMTime();
  return ( mtime > result ? mtime : result );
}

//----------------------------------------------------------------------------
vtkCell *vtkDataSet::FindAndGetCell (float x[3], vtkCell *cell,
                                     vtkIdType cellId, float tol2, int& subId,
                                     float pcoords[3], float *weights)
{
  int newCell = this->FindCell(x,cell,cellId,tol2,subId,pcoords,weights);
  if (newCell >= 0 )
    {
    cell = this->GetCell (newCell);
    }
  else
    {
    return NULL;
    }
  return cell;
}

//----------------------------------------------------------------------------
void vtkDataSet::GetCellNeighbors(vtkIdType cellId, vtkIdList *ptIds,
                                  vtkIdList *cellIds)
{
  vtkIdType i, numPts;
  vtkIdList *otherCells = vtkIdList::New();
  otherCells->Allocate(VTK_CELL_SIZE);

  // load list with candidate cells, remove current cell
  this->GetPointCells(ptIds->GetId(0), cellIds);
  cellIds->DeleteId(cellId);

  // now perform multiple intersections on list
  if ( cellIds->GetNumberOfIds() > 0 )
    {
    for ( numPts=ptIds->GetNumberOfIds(), i=1; i < numPts; i++)
      {
      this->GetPointCells(ptIds->GetId(i), otherCells);
      cellIds->IntersectWith(*otherCells);
      }
    }
  
  otherCells->Delete();
}

//----------------------------------------------------------------------------
void vtkDataSet::GetCellTypes(vtkCellTypes *types)
{
  vtkIdType cellId, numCells=this->GetNumberOfCells();
  unsigned char type;

  types->Reset();
  for (cellId=0; cellId < numCells; cellId++)
    {
    type = this->GetCellType(cellId);
    if ( ! types->IsType(type) )
      {
      types->InsertNextType(type);
      }
    }
}


//----------------------------------------------------------------------------
// Default implementation. This is very slow way to compute this information.
// Subclasses should override this method for efficiency.
void vtkDataSet::GetCellBounds(vtkIdType cellId, float bounds[6])
{
  vtkGenericCell *cell = vtkGenericCell::New();

  this->GetCell(cellId, cell);
  cell->GetBounds(bounds);

  cell->Delete();
}

//----------------------------------------------------------------------------
void vtkDataSet::Squeeze()
{
  this->CellData->Squeeze();
  this->PointData->Squeeze();
}

//----------------------------------------------------------------------------
unsigned long vtkDataSet::GetActualMemorySize()
{
  unsigned long size=this->vtkDataObject::GetActualMemorySize();
  size += this->PointData->GetActualMemorySize();
  size += this->CellData->GetActualMemorySize();
  return size;
}

//----------------------------------------------------------------------------
void vtkDataSet::ShallowCopy(vtkDataObject *dataObject)
{
  vtkDataSet *dataSet = vtkDataSet::SafeDownCast(dataObject);

  if ( dataSet != NULL )
    {
    this->InternalDataSetCopy(dataSet);
    this->CellData->ShallowCopy(dataSet->GetCellData());
    this->PointData->ShallowCopy(dataSet->GetPointData());
    }
  // Do superclass
  this->vtkDataObject::ShallowCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkDataSet::DeepCopy(vtkDataObject *dataObject)
{
  vtkDataSet *dataSet = vtkDataSet::SafeDownCast(dataObject);
 
  if ( dataSet != NULL )
    {
    this->InternalDataSetCopy(dataSet);
    this->CellData->DeepCopy(dataSet->GetCellData());
    this->PointData->DeepCopy(dataSet->GetPointData());
    }

  // Do superclass
  this->vtkDataObject::DeepCopy(dataObject);
}

//----------------------------------------------------------------------------
// This copies all the local variables (but not objects).
void vtkDataSet::InternalDataSetCopy(vtkDataSet *src)
{
  int idx;

  this->ComputeTime = src->ComputeTime;
  this->ScalarRange[0] = src->ScalarRange[0];
  this->ScalarRange[1] = src->ScalarRange[1];
  for (idx = 0; idx < 3; ++idx)
    {
    this->Bounds[2*idx] = src->Bounds[2*idx];
    this->Bounds[2*idx+1] = src->Bounds[2*idx+1];
    }
}


//----------------------------------------------------------------------------
int vtkDataSet::CheckAttributes()
{
  int numPts, numCells;
  int numArrays, idx;
  vtkDataArray *array;
  int numTuples;
  const char* name;

  numPts = this->GetNumberOfPoints();
  numCells = this->GetNumberOfCells();

  numArrays = this->GetPointData()->GetNumberOfArrays();
  for (idx = 0; idx < numArrays; ++idx)
    {
    array = this->GetPointData()->GetArray(idx);
    numTuples = array->GetNumberOfTuples();
    name = array->GetName();
    if (name == NULL)
      {
      name = "";
      }
    if (numTuples < numPts)
      {
      vtkErrorMacro("Point array " << name << " with " 
                    << array->GetNumberOfComponents()
                    << " components, only has " << numTuples << " tuples but there are " 
                    << numPts << " points");
      return 1;
      }
    if (numTuples > numPts)
      {
      vtkWarningMacro("Point array " << name << " with " 
                    << array->GetNumberOfComponents()
                    << " components, has " << numTuples << " tuples but there are only " 
                    << numPts << " points");
      }
    }

  numArrays = this->GetCellData()->GetNumberOfArrays();
  for (idx = 0; idx < numArrays; ++idx)
    {
    array = this->GetCellData()->GetArray(idx);
    numTuples = array->GetNumberOfTuples();
    name = array->GetName();
    if (name == NULL)
      {
      name = "";
      }
    if (numTuples < numCells)
      {
      vtkErrorMacro("Cell array " << name << " with " 
                    << array->GetNumberOfComponents()
                    << " components, has only " << numTuples << " tuples but there are "
                    << numCells << " cells");
      return 1;
      }
    if (numTuples > numCells)
      {
      vtkWarningMacro("Cell array " << name << " with " 
                    << array->GetNumberOfComponents() 
                    << " components, has " << numTuples << " tuples but there are only " 
                    << numCells << " cells");
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkDataSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  float *bounds;

  os << indent << "Number Of Points: " << this->GetNumberOfPoints() << "\n";
  os << indent << "Number Of Cells: " << this->GetNumberOfCells() << "\n";

  os << indent << "Cell Data:\n";
  this->CellData->PrintSelf(os,indent.GetNextIndent());

  os << indent << "Point Data:\n";
  this->PointData->PrintSelf(os,indent.GetNextIndent());

  bounds = this->GetBounds();
  os << indent << "Bounds: \n";
  os << indent << "  Xmin,Xmax: (" <<bounds[0] << ", " << bounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" <<bounds[2] << ", " << bounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" <<bounds[4] << ", " << bounds[5] << ")\n";
  os << indent << "Compute Time: " <<this->ComputeTime.GetMTime() << "\n";
  os << indent << "Release Data: " << (this->ReleaseDataFlag ? "On\n" : "Off\n");
}

