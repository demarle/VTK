/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageHistogram.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageHistogram - Compute the histogram for an image.
// .SECTION Description
// vtkImageHistogram generates a histogram from its input, and optionally
// produces a 2D black-and-white image of the histogram as its output.
// Unlike the class vtkImageAccumulate, a multi-component image does not
// result in a multi-dimensional histogram.  Instead, the resulting
// histogram will be the sum of the histograms of each of the individual
// components, unless SetActiveComponent is used to choose a single
// component.
// .SECTION Thanks
// Thanks to David Gobbi at the Seaman Family MR Centre and Dept. of Clinical
// Neurosciences, Foothills Medical Centre, Calgary, for providing this class.

#ifndef __vtkImageHistogram_h
#define __vtkImageHistogram_h

#include "vtkImagingStatisticsModule.h" // For export macro
#include "vtkThreadedImageAlgorithm.h"

class vtkImageStencilData;
class vtkIdTypeArray;

class VTKIMAGINGSTATISTICS_EXPORT vtkImageHistogram : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageHistogram *New();
  vtkTypeMacro(vtkImageHistogram,vtkThreadedImageAlgorithm);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Scale types for the histogram image.
  enum {
    Linear = 0,
    Log = 1,
    Sqrt = 2
    };

  // Description:
  // Set the component for which to generate a histogram.  The default
  // value is -1, which produces a histogram that is the sum of the
  // histograms of the individual components.
  vtkSetMacro(ActiveComponent, int);
  vtkGetMacro(ActiveComponent, int);

  // Description:
  // If this is On, then the histogram binning will be done automatically.
  // For char and unsigned char data, there will be 256 bins with unit
  // spacing.  For data of type short and larger, there will be between
  // 256 and MaximumNumberOfBins, depending on the range of the data, and
  // the BinOrigin will be set to zero if no negative values are present,
  // or to the smallest negative value if negative values are present.
  // For float data, the MaximumNumberOfBins will always be used.
  // The BinOrigin and BinSpacing will be set so that they provide a mapping
  // from bin index to scalar value.
  vtkSetMacro(AutomaticBinning, int);
  vtkBooleanMacro(AutomaticBinning, int);
  vtkGetMacro(AutomaticBinning, int);

  // Description:
  // The maximum number of bins to use when AutomaticBinning is On.
  // When AutomaticBinning is On, the size of the output histogram
  // will be set to the full range of the input data values, unless
  // the full range is greater than this value.  By default, the max
  // value is 65536, which is large enough to capture the full range
  // of 16-bit integers.
  vtkSetMacro(MaximumNumberOfBins, int);
  vtkGetMacro(MaximumNumberOfBins, int);

  // Description:
  // The number of bins in histogram (default 256).  This is automatically
  // computed unless AutomaticBinning is Off.
  vtkSetMacro(NumberOfBins, int);
  vtkGetMacro(NumberOfBins, int);

  // Description:
  // The value for the center of the first bin (default 0).  This is
  // automatically computed unless AutomaticBinning is Off.
  vtkSetMacro(BinOrigin, double);
  vtkGetMacro(BinOrigin, double);

  // Description:
  // The bin spacing (default 1).  This is automatically computed unless
  // AutomaticBinning is Off.
  vtkSetMacro(BinSpacing, double);
  vtkGetMacro(BinSpacing, double);

  // Description:
  // Use a stencil to compute the histogram for just a part of the image.
  void SetStencilData(vtkImageStencilData *stencil);
  vtkImageStencilData *GetStencil();

  // Description:
  // Equivalent to SetInputConnection(1, algOutput).
  void SetStencilConnection(vtkAlgorithmOutput* algOutput);

  // Description:
  // If this is On, then a histogram image will be produced as the output.
  // Regardless of this setting, the histogram is always available as a
  // vtkIdTypeArray from the GetHistogram method.
  vtkSetMacro(GenerateHistogramImage, int);
  vtkBooleanMacro(GenerateHistogramImage, int);
  vtkGetMacro(GenerateHistogramImage, int);

  // Description:
  // Set the size of the histogram image that is produced as output.
  // The default is 256 by 256.
  vtkSetVector2Macro(HistogramImageSize, int);
  vtkGetVector2Macro(HistogramImageSize, int);

  // Description:
  // Set the scale to use for the histogram image.  The default is
  // a linear scale, but sqrt and log provide better visualization.
  vtkSetClampMacro(HistogramImageScale, int,
    vtkImageHistogram::Linear, vtkImageHistogram::Sqrt);
  void SetHistogramImageScaleToLinear() {
    this->SetHistogramImageScale(vtkImageHistogram::Linear); }
  void SetHistogramImageScaleToLog() {
    this->SetHistogramImageScale(vtkImageHistogram::Log); }
  void SetHistogramImageScaleToSqrt() {
    this->SetHistogramImageScale(vtkImageHistogram::Sqrt); }
  vtkGetMacro(HistogramImageScale, int);
  const char *GetHistogramImageScaleAsString();

  // Description:
  // Get the histogram as a vtkIdTypeArray.  You must call Update()
  // before calling this method.
  vtkIdTypeArray *GetHistogram();

  // Description:
  // Get the total count of the histogram.  This will be the number of
  // voxels times the number of components.
  vtkIdType GetTotal() { return this->Total; }

  // Description:
  // This is part of the executive, but is public so that it can be accessed
  // by non-member functions.
  virtual void ThreadedRequestData(vtkInformation *request,
                                   vtkInformationVector **inputVector,
                                   vtkInformationVector *outputVector,
                                   vtkImageData ***inData,
                                   vtkImageData **outData, int ext[6], int id);
protected:
  vtkImageHistogram();
  ~vtkImageHistogram();

  virtual int RequestUpdateExtent(vtkInformation *vtkNotUsed(request),
                                 vtkInformationVector **inInfo,
                                 vtkInformationVector *vtkNotUsed(outInfo));
  virtual int RequestInformation(vtkInformation *vtkNotUsed(request),
                                 vtkInformationVector **inInfo,
                                 vtkInformationVector *vtkNotUsed(outInfo));
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  virtual int FillOutputPortInformation(int port, vtkInformation *info);

  // Description:
  // Compute the range of the data.  The GetScalarRange() function of
  // vtkImageData only computes the range of the first component, but
  // this filter requires the range for all components.
  void ComputeImageScalarRange(vtkImageData *data, double range[2]);

  int ActiveComponent;
  int AutomaticBinning;
  int MaximumNumberOfBins;

  int HistogramImageSize[2];
  int HistogramImageScale;
  int GenerateHistogramImage;

  int NumberOfBins;
  double BinOrigin;
  double BinSpacing;

  vtkIdTypeArray *Histogram;
  vtkIdType Total;

  vtkIdType *ThreadOutput[VTK_MAX_THREADS];
  int ThreadBinRange[VTK_MAX_THREADS][2];

private:
  vtkImageHistogram(const vtkImageHistogram&);  // Not implemented.
  void operator=(const vtkImageHistogram&);  // Not implemented.
};

#endif
