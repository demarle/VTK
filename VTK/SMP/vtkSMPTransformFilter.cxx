/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTransformFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMPTransformFilter.h"
#include "vtkSMP.h"
#include "vtkMath.h"

#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLinearTransform.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"

#include <time.h>
#include <sys/time.h>
const int vtKaapiRuns = 500;

vtkStandardNewMacro(vtkSMPTransformFilter);

vtkSMPTransformFilter::vtkSMPTransformFilter()
{
  this->Transform = NULL;
}

vtkSMPTransformFilter::~vtkSMPTransformFilter()
{
  this->SetTransform(NULL);
}

struct MyPointIterator : public vtkFunctor
{
  vtkPoints* inPts;
  vtkPoints* outPts;
  vtkDataArray* inNormals;
  vtkDataArray* outNormals;
  vtkDataArray* inVectors;
  vtkDataArray* outVectors;
  vtkAbstractTransform* Transform;

  void operator() ( vtkIdType id ) const
  {
    double coord[3];
    double matrix[3][3];
    inPts->GetPoint( id, coord );
    Transform->InternalTransformDerivative( coord, coord, matrix );
    outPts->SetPoint( id, coord );

    if ( inVectors )
      {
      inVectors->GetTuple( id, coord );
      vtkMath::Multiply3x3( matrix, coord, coord );
      outVectors->SetTuple( id, coord );
      }

    if ( inNormals )
      {
      inNormals->GetTuple( id, coord );
      vtkMath::Transpose3x3( matrix, matrix );
      vtkMath::LinearSolve3x3( matrix, coord, coord );
      vtkMath::Normalize( coord );
      outNormals->SetTuple( id, coord );
      }
  }
};

struct MyVectorIterator : public vtkFunctor
{
  vtkDataArray* inVectors;
  vtkDataArray* outVectors;
  vtkLinearTransform* Transform;

  void operator() ( vtkIdType id ) const
  {
    double coord[3];
    inVectors->GetTuple( id, coord );
    Transform->InternalTransformVector( coord, coord );
    outVectors->SetTuple( id, coord );
  }
};

struct MyNormalIterator : public vtkFunctor
{
  vtkDataArray* inVectors;
  vtkDataArray* outVectors;
  vtkLinearTransform* Transform;

  void operator() ( vtkIdType id ) const
  {
    double coord[3];
    inVectors->GetTuple( id, coord );
    // ineficient: invert and transpose linear matrix each time
    Transform->InternalTransformNormal( coord, coord );
    outVectors->SetTuple( id, coord );
  }
};

int vtkSMPTransformFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  struct timespec t0, t1;
  if (clock_getres(CLOCK_REALTIME, &t0) == 0) cout << "clock resolution " << t0.tv_sec << "s and " << t0.tv_nsec << "ns" << endl;

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPointSet *input = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet *output = vtkPointSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *inPts;
  vtkPoints *newPts;
  vtkDataArray *inVectors, *inCellVectors;;
  vtkFloatArray *newVectors=NULL, *newCellVectors=NULL;
  vtkDataArray *inNormals, *inCellNormals;
  vtkFloatArray *newNormals=NULL, *newCellNormals=NULL;
  vtkIdType numPts, numCells;
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();

  vtkDebugMacro(<<"Executing transform filter");

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  // Check input
  //
  if ( this->Transform == NULL )
    {
    vtkErrorMacro(<<"No transform defined!");
    return 1;
    }

  inPts = input->GetPoints();
  inVectors = pd->GetVectors();
  inNormals = pd->GetNormals();
  inCellVectors = cd->GetVectors();
  inCellNormals = cd->GetNormals();

  if ( !inPts )
    {
    vtkErrorMacro(<<"No input data");
    return 1;
    }

  numPts = inPts->GetNumberOfPoints();
  numCells = input->GetNumberOfCells();

  newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(numPts);
//  newPts->Allocate(numPts);
  if ( inVectors )
    {
    newVectors = vtkFloatArray::New();
    newVectors->SetNumberOfComponents(3);
    newVectors->SetNumberOfTuples(numPts);
//    newVectors->Allocate(3*numPts);
    newVectors->SetName(inVectors->GetName());
    }
  if ( inNormals )
    {
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(numPts);
//    newNormals->Allocate(3*numPts);
    newNormals->SetName(inNormals->GetName());
    }

  this->UpdateProgress (.2);
  // Loop over all points, updating position
  //

  MyPointIterator mypointiterator;
  mypointiterator.inPts = inPts;
  mypointiterator.outPts = newPts;
  mypointiterator.inNormals = inNormals;
  mypointiterator.outNormals = newNormals;
  mypointiterator.inVectors = inVectors;
  mypointiterator.outVectors = newVectors;
  this->Transform->Update();
  mypointiterator.Transform = this->Transform;
  for (int i = 0; i < vtKaapiRuns; ++i)
  {
    clock_gettime(CLOCK_REALTIME, &t0);
    vtkSMP::ForEach( 0, inPts->GetNumberOfPoints(), mypointiterator );
    clock_gettime(CLOCK_REALTIME, &t1);

    int s = t1.tv_sec - t0.tv_sec;
    int ns = t1.tv_nsec - t0.tv_nsec;
    if ( ns < 0 ) { s -= 1; ns += 1000000000; }
    if (s) cout << s;
    cout << ns << endl;
  }

  this->UpdateProgress (.6);

  // Can only transform cell normals/vectors if the transform
  // is linear.
  vtkLinearTransform* lt=vtkLinearTransform::SafeDownCast(this->Transform);
  if (lt)
    {
    if ( inCellVectors )
      {
      newCellVectors = vtkFloatArray::New();
      newCellVectors->SetNumberOfComponents(3);
      newCellVectors->SetNumberOfTuples(numCells);
//      newCellVectors->Allocate(3*numCells);
      newCellVectors->SetName( inCellVectors->GetName() );
      MyVectorIterator myvectoriterator;
      myvectoriterator.inVectors = inCellVectors;
      myvectoriterator.outVectors = newCellVectors;
      myvectoriterator.Transform = lt;
      vtkSMP::ForEach( 0, inCellVectors->GetNumberOfTuples(), myvectoriterator );
//      lt->TransformVectors(inCellVectors,newCellVectors);
      }
    if ( inCellNormals )
      {
      newCellNormals = vtkFloatArray::New();
      newCellNormals->SetNumberOfComponents(3);
      newCellNormals->SetNumberOfTuples(numCells);
//      newCellNormals->Allocate(3*numCells);
      newCellNormals->SetName( inCellNormals->GetName() );
      MyNormalIterator mynormaliterator;
      mynormaliterator.inVectors = inCellNormals;
      mynormaliterator.outVectors = newCellNormals;
      mynormaliterator.Transform = lt;
      vtkSMP::ForEach( 0, inCellNormals->GetNumberOfTuples(), mynormaliterator );
//      lt->TransformNormals(inCellNormals,newCellNormals);
      }
    }

  this->UpdateProgress (.8);

  // Update ourselves and release memory
  //
  output->SetPoints(newPts);
  newPts->Delete();

  if (newNormals)
    {
    outPD->SetNormals(newNormals);
    newNormals->Delete();
    outPD->CopyNormalsOff();
    }

  if (newVectors)
    {
    outPD->SetVectors(newVectors);
    newVectors->Delete();
    outPD->CopyVectorsOff();
    }

  if (newCellNormals)
    {
    outCD->SetNormals(newCellNormals);
    newCellNormals->Delete();
    outCD->CopyNormalsOff();
    }

  if (newCellVectors)
    {
    outCD->SetVectors(newCellVectors);
    newCellVectors->Delete();
    outCD->CopyVectorsOff();
    }

  outPD->PassData(pd);
  outCD->PassData(cd);

  vtkFieldData* inFD = input->GetFieldData();
  if (inFD)
    {
    vtkFieldData* outFD = output->GetFieldData();
    if (!outFD)
      {
      outFD = vtkFieldData::New();
      output->SetFieldData(outFD);
      // We can still use outFD since it's registered
      // by the output
      outFD->Delete();
      }
    outFD->PassData(inFD);
    }

  return 1;
}

void vtkSMPTransformFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Transform: " << this->Transform << "\n";
}
