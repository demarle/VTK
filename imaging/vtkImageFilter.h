/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkImageFilter - Generic filter that has one input..
// .SECTION Description
// vtkImageFilter is a filter class that hides some of the pipeline 
// complexity.  This super class will loop over extra dimensions so the
// subclass can deal with simple low dimensional regions.
// "ComputeRequiredInputUpdateExtent(vtkImageCache *out, *in)" must set the
// extent of in required to compute out. 
// The advantage of using the execute method is that this super class
// will automatically break the execution into pieces if the 
// InputMemoryLimit is violated.
// This creates streaming where the pipeline processes images
// in dynamically sized pieces.



#ifndef __vtkImageFilter_h
#define __vtkImageFilter_h


#include "vtkImageSource.h"
#include "vtkStructuredPoints.h"
#include "vtkStructuredPointsToImage.h"
#include "vtkImageCache.h"
#include "vtkMultiThreader.h"

class VTK_EXPORT vtkImageFilter : public vtkImageSource
{
public:
  vtkImageFilter();
  ~vtkImageFilter();
  static vtkImageFilter *New() {return new vtkImageFilter;};
  const char *GetClassName() {return "vtkImageFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetInput(vtkImageCache *input);
  void SetInput(vtkStructuredPoints *spts)
    {this->SetInput(spts->GetStructuredPointsToImage()->GetOutput());}
  
  virtual void InternalUpdate(vtkImageData *outData);
  virtual void UpdateImageInformation();
  unsigned long int GetPipelineMTime();
  
  // Description:
  // Get input to this filter.
  vtkGetObjectMacro(Input,vtkImageCache);

  // Description:
  // Turning bypass on will cause the filter to turn off and
  // simply pass the data through.  This main purpose for this functionality
  // is support for vtkImageDecomposedFilter.  InputMemoryLimit is ignored
  // when Bypass in on.
  vtkSetMacro(Bypass,int);
  vtkGetMacro(Bypass,int);
  vtkBooleanMacro(Bypass,int);

  // Description:
  // Get/Set the number of threads to create when rendering
  vtkSetClampMacro( NumberOfThreads, int, 1, VTK_MAX_THREADS );
  vtkGetMacro( NumberOfThreads, int );

  // Description:
  // subclasses should define this function
  virtual void ThreadedExecute(vtkImageData *inData, 
			       vtkImageData *outData,
			       int extent[6], int threadId);

  // Description:
  // subclass can over ride this method to do custom streaming and
  // splitting for multiprocessing.
  virtual int SplitExtent(int splitExt[6], int startExt[6], 
			  int num, int total);
  
protected:
  vtkImageCache *Input;     
  vtkMultiThreader *Threader;
  int Bypass;
  int Updating;
  int NumberOfThreads;
  
  virtual void ExecuteImageInformation();
  virtual void ComputeRequiredInputUpdateExtent(int inExt[6],int outExt[6]);

  virtual void RecursiveStreamUpdate(vtkImageData *outData);
  virtual void Execute(vtkImageData *inData, vtkImageData *outData);
};

#endif







