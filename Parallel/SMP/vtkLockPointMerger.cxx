/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLockPointMerger.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkLockPointMerger.h"

#include "vtkDummyMergeFunctor.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSMPMergePoints.h"
#include "vtkThreadLocal.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkLockPointMerger);

//------------------------------------------------------------------------------
void vtkLockPointMerger::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//------------------------------------------------------------------------------
void vtkLockPointMerger::operator()( vtkIdType id ) const
{
  vtkThreadLocal<vtkPoints>::iterator itPoints = this->Functor->InPoints->Begin();
  vtkThreadLocal<vtkPointData>::iterator itPd = this->Functor->InPd->Begin();
  vtkThreadLocal<vtkIdList>::iterator itMaps = this->Functor->Maps->Begin();

  vtkIdType NumberOfPoints = NumberOfPointsFirstThread, NewId;
  while ( id >= NumberOfPoints )
    {
    id -= NumberOfPoints;
    ++itPoints; ++itPd; ++itMaps;
    NumberOfPoints = (*itPoints)->GetNumberOfPoints();
    }

  double* pt = new double[3];
  (*itPoints)->GetPoint( id, pt );
  if ( this->Functor->outputLocator->SetUniquePoint( pt, NewId ) )
    {
    this->Functor->outputPd->SetTuple( NewId, id, (*itPd) );
    }
  (*itMaps)->SetId( id, NewId );
  delete [] pt;
}
