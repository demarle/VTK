/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkPParticlePathFilter.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPParticlePathFilter.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkDataArray.h"
#include "vtkCharArray.h"
#include "vtkFloatArray.h"
#include "vtkNew.h"
#include <assert.h>

vtkStandardNewMacro(vtkPParticlePathFilter);


vtkPParticlePathFilter::vtkPParticlePathFilter()
{
  this->It.Initialize(this);
}

void vtkPParticlePathFilter::ResetCache()
{
  this->It.Reset();
}


void vtkPParticlePathFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


int vtkPParticlePathFilter::OutputParticles(vtkPolyData* particles)
{
  vtkNew<vtkPolyData> tailPoly;
  tailPoly->SetPoints(vtkSmartPointer<vtkPoints>::New());

  vtkPointData* tailPD= tailPoly->GetPointData();
  assert(tailPD);
  tailPD->CopyAllocate(particles->GetPointData());

  for(unsigned int i=0; i<this->Tail.size();i++)
    {
    vtkParticleTracerBaseNamespace::ParticleInformation& info(this->Tail[i].Previous);
    vtkPointData* pd = this->Tail[i].PreviousPD;

    const double    *coord = info.CurrentPosition.x;
    vtkIdType tempId = tailPoly->GetPoints()->InsertNextPoint(coord);
    for(int j=0; j<pd->GetNumberOfArrays();j++)
      {
      vtkDataArray* arrFrom = pd->GetArray(j);
      vtkDataArray* arrTo = tailPD->GetArray(arrFrom->GetName());
      assert(arrTo);
      assert(arrTo->GetNumberOfComponents()==arrFrom->GetNumberOfComponents());
      arrTo->InsertTuple(tempId,arrFrom->GetTuple(0));
      }

    this->GetParticleIds(tailPD)->InsertValue(tempId,info.UniqueParticleId);
    this->GetParticleSourceIds(tailPD)->InsertValue(tempId,info.SourceID);
    this->GetInjectedPointIds(tailPD)->InsertValue(tempId, info.InjectedPointId);
    this->GetInjectedStepIds(tailPD)->InsertValue(tempId, info.InjectedStepId);
    this->GetErrorCodeArr(tailPD)->InsertValue(tempId, info.ErrorCode);
    this->GetParticleAge(tailPD)->InsertValue(tempId, info.age);

    if(this->GetComputeVorticity())
      {
//      this->GetParticleVorticity(tailPD)->InsertNextTuple(info.vorticity); missing in implementation
      this->GetParticleAngularVel(tailPD)->InsertValue(tempId,info.angularVel);
      this->GetParticleRotation(tailPD)->InsertValue(tempId,info.rotation);
      }
    }

  this->It.OutputParticles(tailPoly.GetPointer());
  return this->It.OutputParticles(particles);
}

void vtkPParticlePathFilter::Finalize()
{
  this->It.Finalize();
}
