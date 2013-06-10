/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPImageGaussianSmooth.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMPImageGaussianSmooth.h"
#include "vtkSMP.h"

#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <math.h>

vtkStandardNewMacro(vtkSMPImageGaussianSmooth);

//----------------------------------------------------------------------------
vtkSMPImageGaussianSmooth::vtkSMPImageGaussianSmooth()
{
  this->Dimensionality = 3; // note: this overrides Standard deviation.
  this->StandardDeviations[0] = 2.0;
  this->StandardDeviations[1] = 2.0;
  this->StandardDeviations[2] = 2.0;
  this->RadiusFactors[0] = 1.5;
  this->RadiusFactors[1] = 1.5;
  this->RadiusFactors[2] = 1.5;
}

//----------------------------------------------------------------------------
vtkSMPImageGaussianSmooth::~vtkSMPImageGaussianSmooth()
{
}

//----------------------------------------------------------------------------
void vtkSMPImageGaussianSmooth::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  // int idx;

  //os << indent << "BoundaryRescale: " << this->BoundaryRescale << "\n";

  os << indent << "Dimensionality: " << this->Dimensionality << "\n";

  os << indent << "RadiusFactors: ( "
     << this->RadiusFactors[0] << ", "
     << this->RadiusFactors[1] << ", "
     << this->RadiusFactors[2] << " )\n";

  os << indent << "StandardDeviations: ( "
     << this->StandardDeviations[0] << ", "
     << this->StandardDeviations[1] << ", "
     << this->StandardDeviations[2] << " )\n";
}

//----------------------------------------------------------------------------
void vtkSMPImageGaussianSmooth::ComputeKernel(double *kernel, int min, int max,
                                           double std)
{
  int x;
  double sum;

  // handle special case
  if (std == 0.0)
    {
    kernel[0] = 1.0;
    return;
    }

  // fill in kernel
  sum = 0.0;
  for (x = min; x <= max; ++x)
    {
    sum += kernel[x-min] =
      exp(- (static_cast<double>(x*x)) / (std * std * 2.0));
    }

  // normalize
  for (x = min; x <= max; ++x)
    {
    kernel[x-min] /= sum;
    }
}

//----------------------------------------------------------------------------
int vtkSMPImageGaussianSmooth::RequestUpdateExtent (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  int wholeExtent[6], inExt[6];

  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inExt);

  // Expand filtered axes
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);

  this->InternalRequestUpdateExtent(inExt, wholeExtent);

  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inExt, 6);

  return 1;
}

//----------------------------------------------------------------------------
void vtkSMPImageGaussianSmooth::InternalRequestUpdateExtent(int *inExt,
                                                         int *wholeExtent)
{
  int idx, radius;

  // Expand filtered axes
  for (idx = 0; idx < this->Dimensionality; ++idx)
    {
    radius = static_cast<int>(this->StandardDeviations[idx]
                              * this->RadiusFactors[idx]);
    inExt[idx*2] -= radius;
    if (inExt[idx*2] < wholeExtent[idx*2])
      {
      inExt[idx*2] = wholeExtent[idx*2];
      }

    inExt[idx*2+1] += radius;
    if (inExt[idx*2+1] > wholeExtent[idx*2+1])
      {
      inExt[idx*2+1] = wholeExtent[idx*2+1];
      }
    }
}

template <class T>
class ImageGaussianSmoothExecutor : public vtkFancyFunctor<vtkIdType,vtkIdType>
{
  ImageGaussianSmoothExecutor(const ImageGaussianSmoothExecutor&);
  void operator =(const ImageGaussianSmoothExecutor&);

protected:
  ImageGaussianSmoothExecutor() : inGlobalPtr(0), outGlobalPtr(0)
    {
    inPtr = vtkSMP::vtkThreadLocal<T>::New();
    outPtr = vtkSMP::vtkThreadLocal<T>::New();
    }
  ~ImageGaussianSmoothExecutor()
    {
    inPtr->Delete();
    outPtr->Delete();
    }

  T *inGlobalPtr, *outGlobalPtr;
  vtkSMP::vtkThreadLocal<T> *inPtr, *outPtr;
  int kernelSize, maxC, max0, max1;
  vtkIdType inInc0, outInc0, inInc1, outInc1, inIncK;
  double *kernel;

public:
#define MY_COMMA() ,
  vtkTypeMacro(ImageGaussianSmoothExecutor,vtkFancyFunctor<vtkIdType MY_COMMA() vtkIdType>);
#undef MY_COMMA
  static ImageGaussianSmoothExecutor<T>* New() { return new ImageGaussianSmoothExecutor<T>; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void SetBasePointer(T* inPtrC, T* outPtrC)
    {
    this->inGlobalPtr = inPtrC;
    this->outGlobalPtr = outPtrC;
    }

  void ThreadedMoveBasePointer(vtkIdType off0, vtkIdType off1) const
    {
    this->inPtr->SetLocal(this->inGlobalPtr + off0 * inInc0 + off1 * inInc1);
    this->outPtr->SetLocal(this->outGlobalPtr + off0 * outInc0 + off1 * inInc1);
    }

  void SetData(int axis, int kernelSize, double *kernel, vtkImageData* inData,
               vtkImageData* outData)
    {
    this->kernelSize = kernelSize;
    this->kernel = kernel;

    vtkIdType *inIncs, *outIncs;
    // Do the correct shuffling of the axes (increments, extents)
    inIncs = inData->GetIncrements();
    outIncs = outData->GetIncrements();
    inIncK = inIncs[axis];
    maxC = outData->GetNumberOfScalarComponents();
    switch (axis)
      {
      case 0:
        inInc0 = inIncs[1];  inInc1 = inIncs[2];
        outInc0 = outIncs[1];  outInc1 = outIncs[2];
        break;
      case 1:
        inInc0 = inIncs[0];  inInc1 = inIncs[2];
        outInc0 = outIncs[0];  outInc1 = outIncs[2];
        break;
      case 2:
        inInc0 = inIncs[0];  inInc1 = inIncs[1];
        outInc0 = outIncs[0];  outInc1 = outIncs[1];
        break;
      }
    }

  void operator ()() const
    {
    T *inPtrC, *inPtrK, *outPtrC;
    double *ptrK, sum;
    int idxC, idxK;

    inPtrC = inPtr->GetLocal();
    outPtrC = outPtr->GetLocal();
    inPtr->SetLocal(inPtrC + inInc0);
    outPtr->SetLocal(outPtrC + outInc0);

    for (idxC = 0; idxC < maxC; ++idxC)
      {
      inPtrK = inPtrC;
      ptrK = kernel;
      sum = 0.0;
      // too bad this short loop has to be the inner most loop
      for (idxK = 0; idxK < kernelSize; ++idxK)
        {
        sum += *ptrK * static_cast<double>(*inPtrK);
        ++ptrK;
        inPtrK += inIncK;
        }
      *outPtrC = static_cast<T>(sum);
      ++inPtrC;
      ++outPtrC;
      }
    }
};

//----------------------------------------------------------------------------
// For a given position along the convolution axis, this method loops over
// all other axes, and performs the convolution. Boundary conditions handled
// previously.
template <class T>
void
vtkSMPImageGaussianSmoothExecute(vtkSMPImageGaussianSmooth *self, int axis,
                              double *kernel, int kernelSize,
                              vtkImageData *inData, T *inPtrC,
                              vtkImageData *outData, int outExt[6], T* outPtrC)
//                              T *outPtrC, int *pcycle, int target,
//                              int *pcount, int total)
{
  ImageGaussianSmoothExecutor<T>* functor = ImageGaussianSmoothExecutor<T>::New();
  functor->SetData(axis, kernelSize, kernel, inData, outData);
  functor->SetBasePointer(inPtrC, outPtrC);
  switch(axis)
    {
    case 0:
      vtkSMP::ForEach(outExt[2], outExt[3] + 1, outExt[4], outExt[5] + 1, functor);
      break;
    case 1:
      vtkSMP::ForEach(outExt[0], outExt[1] + 1, outExt[4], outExt[5] + 1, functor);
      break;
    case 2:
      vtkSMP::ForEach(outExt[0], outExt[1] + 1, outExt[2], outExt[3] + 1, functor);
      break;
    }
  functor->Delete();
}

//----------------------------------------------------------------------------
template <class T>
size_t vtkSMPImageGaussianSmoothGetTypeSize(T*)
{
  return sizeof(T);
}

//----------------------------------------------------------------------------
// This method convolves over one axis. It loops over the convolved axis,
// and handles boundary conditions.
void vtkSMPImageGaussianSmooth::ExecuteAxis(int axis,
                                         vtkImageData *inData, int inExt[6],
                                         vtkImageData *outData, int outExt[6],
//                                         int *pcycle, int target,
//                                         int *pcount, int total,
                                         vtkInformation *inInfo)
{
  int idxA, max;
  int wholeExtent[6], wholeMax, wholeMin;
  double *kernel;
  // previousClip and currentClip rembers that the previous was not clipped
  // keeps from recomputing kernels for center pixels.
  int kernelSize = 0;
  int kernelLeftClip, kernelRightClip;
  int previousClipped, currentClipped;
  int radius, size;
  void *inPtr;
  void *outPtr;
  int coords[3];
  vtkIdType *outIncs, outIncA;

  // Get the correct starting pointer of the output
  outPtr = outData->GetScalarPointerForExtent(outExt);
  outIncs = outData->GetIncrements();
  outIncA = outIncs[axis];

  // trick to account for the scalar type of the output(used to be only float)
  switch (outData->GetScalarType())
    {
    vtkTemplateMacro(
      outIncA *= vtkSMPImageGaussianSmoothGetTypeSize(static_cast<VTK_TT*>(0))
      );
    default:
      vtkErrorMacro("Unknown scalar type");
      return;
    }

  // Determine default starting position of input
  coords[0] = inExt[0];
  coords[1] = inExt[2];
  coords[2] = inExt[4];

  // get whole extent for boundary checking ...
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);
  wholeMin = wholeExtent[axis*2];
  wholeMax = wholeExtent[axis*2+1];

  // allocate memory for the kernel
  radius = static_cast<int>(this->StandardDeviations[axis]
                            * this->RadiusFactors[axis]);
  size = 2*radius + 1;
  kernel = new double[size];

  // loop over the convolution axis
  previousClipped = currentClipped = 1;
  max = outExt[axis*2+1];
  for (idxA = outExt[axis*2]; idxA <= max; ++idxA)
    {
    // left boundary condition
    coords[axis] = idxA - radius;
    kernelLeftClip = wholeMin - coords[axis];
    if (kernelLeftClip > 0)
      { // front of kernel is cut off ("kernelStart" samples)
      coords[axis] += kernelLeftClip;
      }
    else
      {
      kernelLeftClip = 0;
      }
    // Right boundary condition
    kernelRightClip = (idxA + radius) - wholeMax;
    if (kernelRightClip < 0)
      {
      kernelRightClip = 0;
      }

    // We can only use previous kernel if it is not clipped and new
    // kernel is also not clipped.
    currentClipped = kernelLeftClip + kernelRightClip;
    if (currentClipped || previousClipped)
      {
      this->ComputeKernel(kernel, -radius+kernelLeftClip,
                          radius-kernelRightClip,
                          static_cast<double>(this->StandardDeviations[axis]));
      kernelSize = size - kernelLeftClip - kernelRightClip;
      }
    previousClipped = currentClipped;

    /* now do the convolution on the rest of the axes */
    inPtr = inData->GetScalarPointer(coords);
    switch (inData->GetScalarType())
      {
      vtkTemplateMacro(
        vtkSMPImageGaussianSmoothExecute(this, axis, kernel, kernelSize,
                                      inData, static_cast<VTK_TT*>(inPtr),
                                      outData, outExt,
                                      static_cast<VTK_TT*>(outPtr))
//                                      pcycle, target, pcount, total)
        );
      default:
        vtkErrorMacro("Unknown scalar type");
        return;
      }
    outPtr = static_cast<void *>(
      static_cast<unsigned char *>(outPtr) + outIncA);
    }

  // get rid of temporary kernel
  delete [] kernel;
}

//----------------------------------------------------------------------------
// This method decomposes the gaussian and smooths along each axis.
int vtkSMPImageGaussianSmooth::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  int inExt[6];
  int target, count, total, cycle;
  vtkImageData* inData = vtkImageData::SafeDownCast(
      inputVector[0]->GetInformationObject(0)->Get(
        vtkDataObject::DATA_OBJECT()));
  vtkImageData* outData = vtkImageData::SafeDownCast(
      outputVector->GetInformationObject(0)->Get(
        vtkDataObject::DATA_OBJECT()));
  this->AllocateOutputData(outData);
  int outExt[6];
  outputVector->GetInformationObject(0)->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outExt);

  // for feed back, determine line target to get 50 progress update
  // update is called every target lines. Progress is computed from
  // the number of pixels processed so far.
//  count = 0; target = 0; total = 0; cycle = 0;
//  if (id == 0)
//    {
//    // determine the number of pixels.
//    total = this->Dimensionality * (outExt[1] - outExt[0] + 1)
//      * (outExt[3] - outExt[2] + 1) * (outExt[5] - outExt[4] + 1)
//      * inData->GetNumberOfScalarComponents();
//    // pixels per update (50 updates)
//    target = total / 50;
//    }

  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro("Execute: input ScalarType, "
                  << inData->GetScalarType()
                  << ", must match out ScalarType "
                  << outData->GetScalarType());
    return 0;
    }

  // Decompose
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  int wholeExt[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExt);
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inExt);
  this->InternalRequestUpdateExtent(inExt, wholeExt);

  switch (this->Dimensionality)
    {
    case 1:
      this->ExecuteAxis(0, inData, inExt, outData, outExt, inInfo);
//                        &cycle, target, &count, total, inInfo);
      break;
    case 2:
      int tempExt[6];
      vtkImageData *tempData;
      // compute intermediate extent
      tempExt[0] = inExt[0];  tempExt[1] = inExt[1];
      tempExt[2] = outExt[2];  tempExt[3] = outExt[3];
      tempExt[4] = inExt[4];  tempExt[5] = inExt[5];
      // create a temp data for intermediate results
      tempData = vtkImageData::New();
      tempData->SetExtent(tempExt);
      tempData->SetNumberOfScalarComponents(
        inData->GetNumberOfScalarComponents());
      tempData->SetScalarType(inData->GetScalarType());
      this->ExecuteAxis(1, inData, inExt, tempData, tempExt, inInfo);
//                        &cycle, target, &count, total, inInfo);
      this->ExecuteAxis(0, tempData, tempExt, outData, outExt, inInfo);
//                        &cycle, target, &count, total, inInfo);
      // release temporary data
      tempData->Delete();
      break;
    case 3:
      // we do z first because it is most likely smallest
      int temp0Ext[6], temp1Ext[6];
      vtkImageData *temp0Data, *temp1Data;
      // compute intermediate extents
      temp0Ext[0] = inExt[0];  temp0Ext[1] = inExt[1];
      temp0Ext[2] = inExt[2];  temp0Ext[3] = inExt[3];
      temp0Ext[4] = outExt[4];  temp0Ext[5] = outExt[5];

      temp1Ext[0] = inExt[0];  temp1Ext[1] = inExt[1];
      temp1Ext[2] = outExt[2];  temp1Ext[3] = outExt[3];
      temp1Ext[4] = outExt[4];  temp1Ext[5] = outExt[5];

      // create a temp data for intermediate results
      temp0Data = vtkImageData::New();
      temp0Data->SetExtent(temp0Ext);
      temp0Data->SetNumberOfScalarComponents(
        inData->GetNumberOfScalarComponents());
      temp0Data->SetScalarType(inData->GetScalarType());

      temp1Data = vtkImageData::New();
      temp1Data->SetExtent(temp1Ext);
      temp1Data->SetNumberOfScalarComponents(
        inData->GetNumberOfScalarComponents());
      temp1Data->SetScalarType(inData->GetScalarType());
      this->ExecuteAxis(2, inData, inExt, temp0Data, temp0Ext, inInfo);
//                        &cycle, target, &count, total, inInfo);
      this->ExecuteAxis(1, temp0Data, temp0Ext, temp1Data, temp1Ext, inInfo);
//                        &cycle, target, &count, total, inInfo);
      temp0Data->Delete();
      this->ExecuteAxis(0, temp1Data, temp1Ext, outData, outExt, inInfo);
//                        &cycle, target, &count, total, inInfo);
      temp1Data->Delete();
      break;
    }
  return 1;
}
