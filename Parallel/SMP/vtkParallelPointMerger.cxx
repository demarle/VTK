/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelPointMerger.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkParallelPointMerger.h"

#include "vtkDummyMergeFunctor.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSMPMergePoints.h"
#include "vtkTask.h"

vtkParallelPointMerger::vtkParallelPointMerger()
  {
  TreatedTable = 0;
  }

vtkParallelPointMerger::~vtkParallelPointMerger()
  {
  TreatedTable = 0;
  }

int vtkParallelPointMerger::MustTreatBucket( vtkIdType idx ) const
  {
  if ( !TreatedTable ) return 0;
  return !__sync_fetch_and_add(&(TreatedTable[idx]), 1);
  }

void vtkParallelPointMerger::SetUsefullData( vtkDummyMergeFunctor* f, vtkIdType** t )
  {
  self = f;
  TreatedTable = t;
  }

//------------------------------------------------------------------------------
vtkParallelPointMerger* vtkParallelPointMerger::New()
  {
  return new vtkParallelPointMerger;
  }

//------------------------------------------------------------------------------
void vtkParallelPointMerger::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

//------------------------------------------------------------------------------
void vtkParallelPointMerger::Execute( vtkSMPMergePoints* locator ) const
  {
  if ( !locator ) return;

  vtkIdType NumberOfBuckets = self->outputLocator->GetNumberOfBuckets();
  vtkSMPMergePoints* l;

  for ( vtkIdType i = 0; i < NumberOfBuckets; ++i )
    {
    if ( locator->GetNumberOfIdInBucket(i) )
      {
      if ( MustTreatBucket(i) )
        {
        vtkThreadLocal<vtkPointData>::iterator itPd = self->InPd->Begin( 1 );
        vtkThreadLocal<vtkIdList>::iterator itMaps = self->Maps->Begin( 1 );
        for ( vtkThreadLocal<vtkSMPMergePoints>::iterator itLocator = self->Locators->Begin( 1 );
              itLocator != self->Locators->End(); ++itLocator, ++itPd, ++itMaps )
          if ( (l = *itLocator) )
            self->outputLocator->Merge( l, i, self->outputPd, *itPd, *itMaps );
        }
      }
    }
  }
