/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageConvolve.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMPImageConvolve.h"
#include "vtkSMP.h"

#include "vtkImageData.h"
#include "vtkImageEllipsoidSource.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkSMPImageConvolve);

//----------------------------------------------------------------------------
// Construct an instance of vtkImageConvolve fitler.
// By default zero values are eroded.
vtkSMPImageConvolve::vtkSMPImageConvolve()
{
  int idx;
  for (idx = 0; idx < 343; idx++)
    {
    this->Kernel[idx] = 0.0;
    }

  // Construct a primary id function kernel that does nothing at all
  double kernel[9];
  for (idx = 0; idx < 9; idx++)
    {
    kernel[idx] = 0.0;
    }
  kernel[4] = 1.0;
  this->SetKernel3x3(kernel);
}

//----------------------------------------------------------------------------
// Destructor
vtkSMPImageConvolve::~vtkSMPImageConvolve()
{
}

//----------------------------------------------------------------------------
void vtkSMPImageConvolve::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "KernelSize: (" <<
    this->KernelSize[0] << ", " <<
    this->KernelSize[1] << ", " <<
    this->KernelSize[2] << ")\n";

  os << indent << "Kernel: (";
  for (int k = 0; k < this->KernelSize[2]; k++)
    {
    for (int j = 0; j < this->KernelSize[1]; j++)
      {
      for (int i = 0; i < this->KernelSize[0]; i++)
        {
        os << this->Kernel[this->KernelSize[1]*this->KernelSize[0]*k +
                           this->KernelSize[0]*j +
                           i];

        if (i != this->KernelSize[0] - 1)
          {
          os << ", ";
          }
        }
      if (j != this->KernelSize[1] - 1 || k != this->KernelSize[2] - 1)
        {
        os << ",\n" << indent << "         ";
        }
      }
    }
  os << ")\n";
}

//----------------------------------------------------------------------------
// Set a 3x3 kernel
void vtkSMPImageConvolve::SetKernel3x3(const double kernel[9])
{
  // Fill the kernel
  this->SetKernel(kernel, 3, 3, 1);
}

//----------------------------------------------------------------------------
// Set a 5x5 kernel
void vtkSMPImageConvolve::SetKernel5x5(const double kernel[25])
{
  // Fill the kernel
  this->SetKernel(kernel, 5, 5, 1);
}

//----------------------------------------------------------------------------
// Set a 7x7 kernel
void vtkSMPImageConvolve::SetKernel7x7(double kernel[49])
{
  // Fill the kernel
  this->SetKernel(kernel, 7, 7, 1);
}

//----------------------------------------------------------------------------
// Set a 3x3x3 kernel
void vtkSMPImageConvolve::SetKernel3x3x3(const double kernel[27])
{
  // Fill the kernel
  this->SetKernel(kernel, 3, 3, 3);
}

//----------------------------------------------------------------------------
// Set a 5x5x5 kernel
void vtkSMPImageConvolve::SetKernel5x5x5(double kernel[125])
{
  // Fill the kernel
  this->SetKernel(kernel, 5, 5, 5);
}

//----------------------------------------------------------------------------
// Set a 7x7x7 kernel
void vtkSMPImageConvolve::SetKernel7x7x7(double kernel[343])
{
  // Fill the kernel
  this->SetKernel(kernel, 7, 7, 7);
}

//----------------------------------------------------------------------------
// Set a kernel, this is an internal method
void vtkSMPImageConvolve::SetKernel(const double* kernel,
                                 int sizeX, int sizeY, int sizeZ)
{
  int modified=0;

  // Set the correct kernel size
  this->KernelSize[0] = sizeX;
  this->KernelSize[1] = sizeY;
  this->KernelSize[2] = sizeZ;

  int kernelLength = sizeX*sizeY*sizeZ;

  for (int idx = 0; idx < kernelLength; idx++)
    {
    if ( this->Kernel[idx] != kernel[idx] )
      {
      modified = 1;
      this->Kernel[idx] = kernel[idx];
      }
    }
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
// Get the 3x3 kernel
double* vtkSMPImageConvolve::GetKernel3x3()
{
  return this->GetKernel();
}

//----------------------------------------------------------------------------
// Get the 5x5 kernel
double* vtkSMPImageConvolve::GetKernel5x5()
{
  return this->GetKernel();
}

//----------------------------------------------------------------------------
// Get the 7x7 kernel
double* vtkSMPImageConvolve::GetKernel7x7()
{
  return this->GetKernel();
}

//----------------------------------------------------------------------------
// Get the 3x3x3 kernel
double* vtkSMPImageConvolve::GetKernel3x3x3()
{
  return this->GetKernel();
}

//----------------------------------------------------------------------------
// Get the 5x5x5 kernel
double* vtkSMPImageConvolve::GetKernel5x5x5()
{
  return this->GetKernel();
}

//----------------------------------------------------------------------------
// Get the 7x7x7 kernel
double* vtkSMPImageConvolve::GetKernel7x7x7()
{
  return this->GetKernel();
}

//----------------------------------------------------------------------------
// Get the kernel, this is an internal method
double* vtkSMPImageConvolve::GetKernel()
{
  return this->Kernel;
}

//----------------------------------------------------------------------------
// Get the kernel
void vtkSMPImageConvolve::GetKernel3x3(double kernel[9])
{
  this->GetKernel(kernel);
}

//----------------------------------------------------------------------------
// Get the kernel
void vtkSMPImageConvolve::GetKernel5x5(double kernel[25])
{
  this->GetKernel(kernel);
}

//----------------------------------------------------------------------------
// Get the kernel
void vtkSMPImageConvolve::GetKernel7x7(double kernel[49])
{
  this->GetKernel(kernel);
}

//----------------------------------------------------------------------------
// Get the kernel
void vtkSMPImageConvolve::GetKernel3x3x3(double kernel[27])
{
  this->GetKernel(kernel);
}

//----------------------------------------------------------------------------
// Get the kernel
void vtkSMPImageConvolve::GetKernel5x5x5(double kernel[125])
{
  this->GetKernel(kernel);
}

//----------------------------------------------------------------------------
// Get the kernel
void vtkSMPImageConvolve::GetKernel7x7x7(double kernel[343])
{
  this->GetKernel(kernel);
}

//----------------------------------------------------------------------------
// Get the kernel, this is an internal method
void vtkSMPImageConvolve::GetKernel(double *kernel)
{
  int kernelLength = this->KernelSize[0]*
    this->KernelSize[1]*this->KernelSize[2];

  for (int idx = 0; idx < kernelLength; idx++)
    {
    kernel[idx] = this->Kernel[idx];
    }
}

template <class T>
class ConvolveFunctor : public vtkFunctor
{
    ConvolveFunctor(const ConvolveFunctor&);
    void operator =(const ConvolveFunctor&);
  protected:
    ConvolveFunctor() : kernel(0), inGlobPtr(0), outGlobPtr(0), inImageExt(0)
      {
      kernelMiddle[0] = kernelMiddle[1] = kernelMiddle[2] = 0;
      }
    ~ConvolveFunctor(){}

    int kernelMiddle[3];
    int hoodMin0, hoodMax0, hoodMin1, hoodMax1, hoodMin2, hoodMax2;
    vtkIdType inInc0, inInc1, inInc2;
    vtkIdType outInc0, outInc1, outInc2;

    double *kernel;
    T *inGlobPtr, *outGlobPtr;
    int *inImageExt;
    int numInSlice, numInRow;

  public:
    vtkTypeMacro(ConvolveFunctor,vtkFunctor);
    static ConvolveFunctor* New() { return new ConvolveFunctor<T>; }
    void PrintSelf(ostream &os, vtkIndent indent)
      {
      this->Superclass::PrintSelf(os,indent);
      }

    void SetData(int kernelSize[3], double kern[343], T* inPtr, T* outPtr,
                 int inExt[6], vtkIdType inNumComp, vtkIdType outNumComp,
                 int outNumInRow, int outNumInSlice)
      {
      kernel = kern;
      inGlobPtr = inPtr;
      outGlobPtr = outPtr;
      inImageExt = inExt;
      numInRow = outNumInRow;
      numInSlice = outNumInSlice;

      inInc0 = inNumComp; inInc1 = inNumComp * (inExt[1] - inExt[0] + 1); inInc2 = inInc1 * (inExt[3] - inExt[2] + 1);
      outInc0 = outNumComp; outInc1 = outNumComp * numInRow; outInc2 = outNumComp * numInSlice;

      kernelMiddle[0] = kernelSize[0] / 2;
      kernelMiddle[1] = kernelSize[1] / 2;
      kernelMiddle[2] = kernelSize[2] / 2;

      hoodMin0 = -kernelMiddle[0];
      hoodMin1 = -kernelMiddle[1];
      hoodMin2 = -kernelMiddle[2];

      hoodMax0 = hoodMin0 + kernelSize[0] - 1;
      hoodMax1 = hoodMin1 + kernelSize[1] - 1;
      hoodMax2 = hoodMin2 + kernelSize[2] - 1;
      }

    void operator ()( vtkIdType outIdx0, vtkIdType outIdx1, vtkIdType outIdx2 ) const
      {
      // Compute spacial index for pixel id
      // this replace the three sequential for loops
      T* outPtr = outGlobPtr + (outIdx0 * outInc0) + (outIdx1 * outInc1) + (outIdx2 * outInc2);
      T* inPtr = inGlobPtr + (outIdx0 * inInc0) + (outIdx1 * inInc1) + (outIdx2 * inInc2);

      // For looping through hood pixels
      int hoodIdx0, hoodIdx1, hoodIdx2;
      T *hoodPtr0, *hoodPtr1, *hoodPtr2;

      // For looping through the kernel, and compute the kernel result
      int kernelIdx;
      double sum;

      for (int outIdxC = 0; outIdxC < outInc0; ++outIdxC) //outInc0 is outData->GetNumberOfScalarComponents()
        {
        // Inner loop where we compute the kernel

        // Set the sum to zero
        sum = 0;

        // loop through neighborhood pixels
        // as sort of a hack to handle boundaries,
        // input pointer will be marching through data that does not exist.
        hoodPtr2 = inPtr - kernelMiddle[0] * inInc0
                         - kernelMiddle[1] * inInc1
                         - kernelMiddle[2] * inInc2;

        // Set the kernel index to the starting position
        kernelIdx = 0;

        for (hoodIdx2 = hoodMin2; hoodIdx2 <= hoodMax2; ++hoodIdx2)
          {
          hoodPtr1 = hoodPtr2;

          for (hoodIdx1 = hoodMin1; hoodIdx1 <= hoodMax1; ++hoodIdx1)
            {
            hoodPtr0 = hoodPtr1;

            for (hoodIdx0 = hoodMin0; hoodIdx0 <= hoodMax0; ++hoodIdx0)
              {
              // A quick but rather expensive way to handle boundaries
              // This assumes the boundary values are zero
              if (outIdx0 + hoodIdx0 >= inImageExt[0] &&
                  outIdx0 + hoodIdx0 <= inImageExt[1] &&
                  outIdx1 + hoodIdx1 >= inImageExt[2] &&
                  outIdx1 + hoodIdx1 <= inImageExt[3] &&
                  outIdx2 + hoodIdx2 >= inImageExt[4] &&
                  outIdx2 + hoodIdx2 <= inImageExt[5])
                {
                sum += *hoodPtr0 * kernel[kernelIdx];

                // Take the next postion in the kernel
                kernelIdx++;
                }

              hoodPtr0 += inInc0;
              }

            hoodPtr1 += inInc1;
            }

          hoodPtr2 += inInc2;
          }

        // Set the output pixel to the correct value
        *outPtr = static_cast<T>(sum);

        ++inPtr;
        ++outPtr;
        }
      }
};

//----------------------------------------------------------------------------
// This templated function executes the filter on any region,
// whether it needs boundary checking or not.
// If the filter needs to be faster, the function could be duplicated
// for strictly center (no boundary) processing.
template <class T>
void vtkSMPImageConvolveExecute(vtkSMPImageConvolve *self,
                             vtkImageData *inData, int inImageExt[6],
                             vtkImageData *outData, T *outPtr,
                             int outExt[6])
{
  // Get the kernel, just use GetKernel7x7x7(kernel) if the kernel is smaller
  // it still works :)
  double kernel[343];
  self->GetKernel7x7x7(kernel);

  int numInRow = outExt[1] - outExt[0] + 1;
  int numInSlice = numInRow * (outExt[3] - outExt[2] + 1);

  ConvolveFunctor<T>* functor = ConvolveFunctor<T>::New();
  functor->SetData( self->GetKernelSize(), kernel,
                    static_cast<T*>(inData->GetScalarPointer(outExt[0], outExt[2], outExt[4])),
                    outPtr, inImageExt, inData->GetNumberOfScalarComponents(),
                    outData->GetNumberOfScalarComponents(), numInRow, numInSlice);

  // loop through components
  vtkSMP::ForEach(outExt[0], outExt[1] + 1, outExt[2], outExt[3] + 1, outExt[4], outExt[5] + 1, functor);
  functor->Delete();
}

//----------------------------------------------------------------------------
// This method contains the first switch statement that calls the correct
// templated function for the input and output Data types.
// It hanldes image boundaries, so the image does not shrink.
int vtkSMPImageConvolve::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData* inData = vtkImageData::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* outData = vtkImageData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));
  this->AllocateOutputData(outData);

  // this filter expects the output type to be same as input
  if (outData->GetScalarType() != inData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: output ScalarType, "
      << vtkImageScalarTypeNameMacro(outData->GetScalarType())
      << " must match input scalar type");
    return 0;
    }

  int wholeExtent[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);
  int updateExtent[6];
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), updateExtent);

  switch (inData->GetScalarType())
    {
    vtkTemplateMacro(
      vtkSMPImageConvolveExecute(this, inData, wholeExtent, outData,
                              static_cast<VTK_TT *>(outData->GetScalarPointer()),
                              updateExtent));

    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return 0;
    }
  return 1;
}
