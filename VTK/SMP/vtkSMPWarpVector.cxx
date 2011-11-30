/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPWarpVector.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMPWarpVector.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"

#include "vtkBitArrayIterator.h"
#include "vtkArrayIteratorTemplate.h"

#include "vtkSMP.h"

#define vtkDataArrayIteratorTemplateMacro(call)                                 \
  vtkArrayIteratorTemplateMacroCase(VTK_DOUBLE, double, call);              \
  vtkArrayIteratorTemplateMacroCase(VTK_FLOAT, float, call);                             \
  vtkArrayIteratorTemplateMacroCase_ll(VTK_LONG_LONG, long long, call);                  \
  vtkArrayIteratorTemplateMacroCase_ll(VTK_UNSIGNED_LONG_LONG, unsigned long long, call);\
  vtkArrayIteratorTemplateMacroCase_si64(VTK___INT64, __int64, call);                    \
  vtkArrayIteratorTemplateMacroCase_ui64(VTK_UNSIGNED___INT64, unsigned __int64, call);  \
  vtkArrayIteratorTemplateMacroCase(VTK_ID_TYPE, vtkIdType, call);                       \
  vtkArrayIteratorTemplateMacroCase(VTK_LONG, long, call);                               \
  vtkArrayIteratorTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, call);             \
  vtkArrayIteratorTemplateMacroCase(VTK_INT, int, call);                                 \
  vtkArrayIteratorTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, call);               \
  vtkArrayIteratorTemplateMacroCase(VTK_SHORT, short, call);                             \
  vtkArrayIteratorTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, call);           \
  vtkArrayIteratorTemplateMacroCase(VTK_CHAR, char, call);                               \
  vtkArrayIteratorTemplateMacroCase(VTK_SIGNED_CHAR, signed char, call);                 \
  vtkArrayIteratorTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, call);


vtkStandardNewMacro(vtkSMPWarpVector);

//----------------------------------------------------------------------------
vtkSMPWarpVector::vtkSMPWarpVector()
{
  this->ScaleFactor = 1.0;

  // by default process active point vectors
  this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,
                               vtkDataSetAttributes::VECTORS);
}

//----------------------------------------------------------------------------
vtkSMPWarpVector::~vtkSMPWarpVector()
{
}

//----------------------------------------------------------------------------
template <class T1, class T2>
struct vtkSMPWarpVectorOp : public vtkFunctor
{
  vtkArrayIteratorTemplate<T1> *inIter;
  vtkArrayIteratorTemplate<T1> *outIter;
  vtkArrayIteratorTemplate<T2> *inVecIter;
  T1 scaleFactor;

  void  operator()(vtkIdType index) const
    {
    T1* inTuple = inIter->GetTuple(index);
    T1* outTuple = outIter->GetTuple(index);
    T2* inVecTuple = inVecIter->GetTuple(index);

    outTuple[0] = inTuple[0] + scaleFactor * (T1)(inVecTuple[0]);
    outTuple[1] = inTuple[1] + scaleFactor * (T1)(inVecTuple[1]);
    outTuple[2] = inTuple[2] + scaleFactor * (T1)(inVecTuple[2]);
    }
};

//----------------------------------------------------------------------------
template <class T1, class T2>
void vtkSMPWarpVectorExecute2(vtkSMPWarpVector *self,
                              vtkArrayIteratorTemplate<T1> *inIter,
                              vtkArrayIteratorTemplate<T1> *outIter,
                              vtkArrayIteratorTemplate<T2> *inVecIter,
                              vtkIdType size)
{
  vtkSMPWarpVectorOp<T1, T2> op;
  op.inIter = inIter;
  op.outIter = outIter;
  op.inVecIter = inVecIter;
  op.scaleFactor = (T1)self->GetScaleFactor();

  vtkSMP::ForEach( 0, size, op);
}

//----------------------------------------------------------------------------
template <class T>
void vtkSMPWarpVectorExecute(vtkSMPWarpVector *self,
                          vtkArrayIteratorTemplate<T> *inIter,
                          vtkArrayIteratorTemplate<T> *outIter,
                          vtkIdType size,
                          vtkDataArray *vectors)
{
  vtkArrayIterator *inVecIter;
  inVecIter = vectors->NewIterator();

  // call templated function
  switch (vectors->GetDataType())
    {
    vtkDataArrayIteratorTemplateMacro(
      vtkSMPWarpVectorExecute2(self, inIter, outIter,
                            (VTK_TT *)(inVecIter), size));
    default:
      break;
    }

  inVecIter->Delete();
}

//----------------------------------------------------------------------------
int vtkSMPWarpVector::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPointSet *input = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet *output = vtkPointSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *points;
  vtkIdType numPts;

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  if (input == NULL || input->GetPoints() == NULL)
    {
    return 1;
    }
  numPts = input->GetPoints()->GetNumberOfPoints();

  vtkDataArray *vectors = this->GetInputArrayToProcess(0,inputVector);

  if ( !vectors || !numPts)
    {
    vtkDebugMacro(<<"No input data");
    return 1;
    }

  // SETUP AND ALLOCATE THE OUTPUT
  numPts = input->GetNumberOfPoints();
  points = input->GetPoints()->NewInstance();
  points->SetDataType(input->GetPoints()->GetDataType());
  points->Allocate(numPts);
  points->SetNumberOfPoints(numPts);
  output->SetPoints(points);
  points->Delete();

  vtkDataArray* inpts = input->GetPoints()->GetData();
  vtkDataArray* outpts = output->GetPoints()->GetData();

  vtkArrayIterator* inIter = inpts->NewIterator();
  vtkArrayIterator* outIter = outpts->NewIterator();

  // call templated function
  //VTK_CHECK_SMP_INITIALIZED
  switch (input->GetPoints()->GetDataType())
    {
    vtkDataArrayIteratorTemplateMacro(
      vtkSMPWarpVectorExecute( this, (VTK_TT *)(inIter),
                            (VTK_TT *)(outIter), numPts, vectors) );
    default:
      break;
    }

  inIter->Delete();
  outIter->Delete();

  // now pass the data.
  output->GetPointData()->CopyNormalsOff(); // distorted geometry
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

  return 1;
}

//----------------------------------------------------------------------------
void vtkSMPWarpVector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}
