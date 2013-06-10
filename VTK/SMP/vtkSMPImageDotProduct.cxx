/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageDotProduct.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMPImageDotProduct.h"
#include "vtkSMP.h"

#include "vtkImageData.h"
#include "vtkImageProgressIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkSMPImageDotProduct);

//----------------------------------------------------------------------------
vtkSMPImageDotProduct::vtkSMPImageDotProduct()
{
  this->SetNumberOfInputPorts(2);
}

//----------------------------------------------------------------------------
// Colapse the first axis
int vtkSMPImageDotProduct::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector ** vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector)
{
  vtkDataObject::SetPointDataActiveScalarInfo(
    outputVector->GetInformationObject(0), -1, 1);
  return 1;
}

template <class T>
class DotProductFunctor : public vtkFancyFunctor<vtkIdType,vtkIdType,vtkIdType>
{
    DotProductFunctor( const DotProductFunctor& );
    void operator =( const DotProductFunctor& );
  protected:
    DotProductFunctor() : in1GlobalPointer(0), in2GlobalPointer(0), outGlobalPointer(0)
      {
      in1Increment[0] = in1Increment[1] = in1Increment[2] = 0;
      in2Increment[0] = in2Increment[1] = in2Increment[2] = 0;
      outIncrement[0] = outIncrement[1] = outIncrement[2] = 0;
      inPtr1 = vtkSMP::vtkThreadLocal<T>::New();
      inPtr2 = vtkSMP::vtkThreadLocal<T>::New();
      outPtr = vtkSMP::vtkThreadLocal<T>::New();
      }
    ~DotProductFunctor()
      {
      inPtr1->Delete();
      inPtr2->Delete();
      outPtr->Delete();
      }

    T* in1GlobalPointer;
    T* in2GlobalPointer;
    T* outGlobalPointer;

    vtkSMP::vtkThreadLocal<T> *inPtr1, *inPtr2, *outPtr;

    vtkIdType in1Increment[3];
    vtkIdType in2Increment[3];
    vtkIdType outIncrement[3];

  public:
#define MY_COMMA() ,
    vtkTypeMacro(DotProductFunctor,vtkFancyFunctor<vtkIdType MY_COMMA() vtkIdType MY_COMMA() vtkIdType>);
#undef MY_COMMA
    static DotProductFunctor<T>* New() { return new DotProductFunctor<T>; }
    void PrintSelf(ostream &os, vtkIndent indent)
      {
      this->Superclass::PrintSelf(os, indent);
      }

    void SetData(vtkImageData* in1Data, vtkImageData* in2Data, vtkImageData* outData, int outExt[6])
      {
      in1GlobalPointer = static_cast<T*>(in1Data->GetScalarPointerForExtent(outExt));
      in2GlobalPointer = static_cast<T*>(in2Data->GetScalarPointerForExtent(outExt));
      outGlobalPointer = static_cast<T*>(outData->GetScalarPointerForExtent(outExt));

      in1Data->GetIncrements(in1Increment[0], in1Increment[1], in1Increment[2]);
      in2Data->GetIncrements(in2Increment[0], in2Increment[1], in2Increment[2]);
      outData->GetIncrements(outIncrement[0], outIncrement[1], outIncrement[2]);
      }

    void ThreadedMoveBasePointer(vtkIdType idx0, vtkIdType idx1, vtkIdType idx2) const
      {
      inPtr1->SetLocal(in1GlobalPointer + (idx0 * in1Increment[0])
          + (idx1 * in1Increment[1])
          + (idx2 * in1Increment[2]));
      inPtr2->SetLocal(in2GlobalPointer + (idx0 * in2Increment[0])
          + (idx1 * in2Increment[1])
          + (idx2 * in2Increment[2]));
      outPtr->SetLocal(outGlobalPointer + (idx0 * outIncrement[0])
          + (idx1 * outIncrement[1])
          + (idx2 * outIncrement[2]));
      }

    void operator ()() const
      {
      T* inSI1 = inPtr1->GetLocal();
      T* inSI2 = inPtr2->GetLocal();
      T* outSI = outPtr->GetLocal();

      float dot;
      int idxC, maxC = in1Increment[0];

      // now process the components
      dot = 0.0;
      for (idxC = 0; idxC < maxC; idxC++)
        {
        dot += static_cast<float>(*inSI1 * *inSI2);
        ++inSI1;
        ++inSI2;
        }
      *outSI = static_cast<T>(dot);

      inPtr1->SetLocal(inSI1);
      inPtr2->SetLocal(inSI2);
      outPtr->SetLocal(outSI + outIncrement[0]);
      }
};

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the two input operations
template <class T>
void vtkSMPImageDotProductExecute(vtkImageData *in1Data,
                                  vtkImageData *in2Data,
                                  vtkImageData *outData,
                                  int outExt[6], T *)
{
  DotProductFunctor<T>* functor = DotProductFunctor<T>::New();
  functor->SetData(in1Data, in2Data, outData, outExt);
  vtkSMP::ForEach(outExt[0], outExt[1] + 1, outExt[2], outExt[3] + 1, outExt[4], outExt[5] + 1, functor);
  functor->Delete();
}


//----------------------------------------------------------------------------
// This method is passed a input and output regions, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the regions data types.
int vtkSMPImageDotProduct::RequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkImageData* inData0 = vtkImageData::SafeDownCast(
        inputVector[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* inData1 = vtkImageData::SafeDownCast(
        inputVector[1]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

  vtkImageData* outData = vtkImageData::SafeDownCast(
        outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
  this->AllocateOutputData(outData);

  // this filter expects that input is the same type as output.
  if (inData0->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input1 ScalarType, "
                  <<  inData0->GetScalarType()
                  << ", must match output ScalarType "
                  << outData->GetScalarType());
    return 0;
    }

  if (inData1->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input2 ScalarType, "
                  << inData1->GetScalarType()
                  << ", must match output ScalarType "
                  << outData->GetScalarType());
    return 0;
    }

  // this filter expects that inputs that have the same number of components
  if (inData0->GetNumberOfScalarComponents() !=
      inData1->GetNumberOfScalarComponents())
    {
    vtkErrorMacro(<< "Execute: input1 NumberOfScalarComponents, "
                  << inData0->GetNumberOfScalarComponents()
                  << ", must match out input2 NumberOfScalarComponents "
                  << inData1->GetNumberOfScalarComponents());
    return 0;
    }

  int outExt[6];
  outputVector->GetInformationObject(0)->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outExt);

  switch (inData0->GetScalarType())
    {
    vtkTemplateMacro(
      vtkSMPImageDotProductExecute(inData0, inData1,
                                   outData, outExt,
                                   static_cast<VTK_TT *>(0)));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return 0;
    }
  return 1;
}
