/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDummyMergeFunctor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkDummyMergeFunctor.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkObjectFactory.h"
#include "vtkOffsetManager.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSMPMergePoints.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkDummyMergeFunctor);

//------------------------------------------------------------------------------
vtkDummyMergeFunctor::vtkDummyMergeFunctor ()
{
  vertOffset = vtkOffsetManager::New();
  lineOffset = vtkOffsetManager::New();
  polyOffset = vtkOffsetManager::New();
  stripOffset = vtkOffsetManager::New();

  vertOffset->Register( this );
  lineOffset->Register( this );
  polyOffset->Register( this );
  stripOffset->Register( this );

  vertOffset->Delete();
  lineOffset->Delete();
  polyOffset->Delete();
  stripOffset->Delete();

  Maps = vtkThreadLocal<vtkIdList>::New();
  Maps->Register( this );
  Maps->Delete();

  Locators = 0;
  InPoints = 0;
  outputLocator = 0;
}

//------------------------------------------------------------------------------
vtkDummyMergeFunctor::~vtkDummyMergeFunctor ()
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

//------------------------------------------------------------------------------
void vtkDummyMergeFunctor::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//------------------------------------------------------------------------------
void vtkDummyMergeFunctor::operator ()( vtkIdType pointId ) const
{
  outputLocator->AddPointIdInBucket( pointId );
}

//------------------------------------------------------------------------------
void vtkDummyMergeFunctor::InitializeNeeds(
  vtkThreadLocal<vtkSMPMergePoints>* _locator,
  vtkThreadLocal<vtkPoints>* _points,
  vtkSMPMergePoints* _outlocator,
  vtkThreadLocal<vtkCellArray>* _inverts,
  vtkCellArray* _outverts,
  vtkThreadLocal<vtkCellArray>* _inlines,
  vtkCellArray* _outlines,
  vtkThreadLocal<vtkCellArray>* _inpolys,
  vtkCellArray* _outpolys,
  vtkThreadLocal<vtkCellArray>* _instrips,
  vtkCellArray* _outstrips,
  vtkThreadLocal<vtkPointData>* _inpd,
  vtkPointData* _outpd,
  vtkThreadLocal<vtkCellData>* _incd,
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
    InVerts = vtkThreadLocal<vtkCellArray>::New();
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
    InLines = vtkThreadLocal<vtkCellArray>::New();
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
    InPolys = vtkThreadLocal<vtkCellArray>::New();
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
    InStrips = vtkThreadLocal<vtkCellArray>::New();
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
  vtkThreadLocal<vtkSMPMergePoints>::iterator itLocator;
  if ( _locator )
    itLocator = Locators->Begin();
  vtkThreadLocal<vtkPoints>::iterator itPoints;
  if ( _points )
    itPoints = InPoints->Begin();
  vtkThreadLocal<vtkCellArray>::iterator itVerts = InVerts->Begin();
  vtkThreadLocal<vtkCellArray>::iterator itLines = InLines->Begin();
  vtkThreadLocal<vtkCellArray>::iterator itPolys = InPolys->Begin();
  vtkThreadLocal<vtkCellArray>::iterator itStrips = InStrips->Begin();
  vertOffset->InitManageValues();
  lineOffset->InitManageValues();
  polyOffset->InitManageValues();
  stripOffset->InitManageValues();
  for ( vtkThreadLocal<vtkIdList>::iterator itMaps = Maps->Begin(); itMaps != this->Maps->End(); ++itMaps )
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
