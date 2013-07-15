/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelCellMerger.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkParallelCellMerger.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDummyMergeFunctor.h"
#include "vtkObjectFactory.h"
#include "vtkOffsetManager.h"
#include "vtkTask.h"

//------------------------------------------------------------------------------
vtkParallelCellMerger * vtkParallelCellMerger::New()
{
  return new vtkParallelCellMerger;
}

//------------------------------------------------------------------------------
void vtkParallelCellMerger::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//------------------------------------------------------------------------------
void vtkParallelCellMerger::Execute( vtkIdList* map,
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
  if ( !map )
    {
    return;
    }

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

    // usage of ++newId instead of newId++
    newId = lineCellOffset - 1 + self->vertOffset->GetNumberOfCells();
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
    // usage of ++newId instead of newId++
    newId = polyCellOffset - 1 + self->lineOffset->GetNumberOfCells();
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
    // usage of ++newId instead of newId++
    newId = stripCellOffset - 1 + self->polyOffset->GetNumberOfCells();
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
