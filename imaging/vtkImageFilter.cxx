/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageFilter.cxx
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
#include "vtkImageFilter.h"

//----------------------------------------------------------------------------
vtkImageFilter::vtkImageFilter()
{
  this->Input = NULL;
  this->Bypass = 0;
  this->Updating = 0;
  this->Threader = vtkMultiThreader::New();
  this->NumberOfThreads = this->Threader->GetNumberOfThreads();
}

//----------------------------------------------------------------------------
vtkImageFilter::~vtkImageFilter()
{
  this->Threader->Delete();
}

//----------------------------------------------------------------------------
void vtkImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "Bypass: " << this->Bypass << "\n";
  os << indent << "Input: (" << this->Input << ").\n";
  os << indent << "NumberOfThreads: " << this->NumberOfThreads << "\n";

  vtkImageSource::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
// Description:
// This Method returns the MTime of the pipeline upto and including this filter
// Note: current implementation may create a cascade of GetPipelineMTime calls.
// Each GetPipelineMTime call propagates the call all the way to the original
// source.  
unsigned long int vtkImageFilter::GetPipelineMTime()
{
  unsigned long int time1, time2;

  // This objects MTime
  // (Super class considers cache in case cache did not originate message)
  time1 = this->vtkImageSource::GetPipelineMTime();
  if ( ! this->Input)
    {
    vtkWarningMacro(<< "GetPipelineMTime: Input not set.");
    return time1;
    }
  
  // Pipeline mtime 
  time2 = this->Input->GetPipelineMTime();
  
  // Return the larger of the two 
  if (time2 > time1)
    time1 = time2;

  return time1;
}


//----------------------------------------------------------------------------
// Description:
// Set the Input of a filter. 
void vtkImageFilter::SetInput(vtkImageCache *input)
{
  vtkDebugMacro(<< "SetInput: input = " << input->GetClassName()
		<< " (" << input << ")");

  // does this change anything?
  if (input == this->Input)
    {
    return;
    }
  
  this->Input = input;
  this->Modified();
}


//----------------------------------------------------------------------------
// Description:
// This method is called by the cache.  It eventually calls the
// Execute(vtkImageData *, vtkImageData *) method.
// ImageInformation has already been updated by this point, 
// and outRegion is in local coordinates.
// This method will stream to get the input, and loops over extra axes.
// Only the UpdateExtent from output will get updated.
void vtkImageFilter::InternalUpdate(vtkImageData *outData)
{
  // Make sure the Input has been set.
  if ( ! this->Input)
    {
    vtkErrorMacro(<< "Input is not set.");
    return;
    }

  // prevent infinite update loops.
  if (this->Updating)
    {
    return;
    }
  this->Updating = 1;
  
  // Make sure there is an output.
  this->CheckCache();

  // In case this update is called directly.
  this->UpdateImageInformation();
  this->Output->ClipUpdateExtentWithWholeExtent();

  // Handle bypass condition.
  if (this->Bypass)
    {
    vtkImageData *inData;

    this->Input->SetUpdateExtent(this->Output->GetUpdateExtent());
    inData = this->Input->UpdateAndReturnData();
    if (!inData)
      {
      vtkWarningMacro("No input data provided!");
      }
    else
      {
      outData->GetPointData()->PassData(inData->GetPointData());
      }

    // release input data
    if (this->Input->ShouldIReleaseData())
      {
      this->Input->ReleaseData();
      }
    this->Updating = 0;
    return;
    }
  
  this->RecursiveStreamUpdate(outData,2);

  this->Updating = 0;
}



  
//----------------------------------------------------------------------------
// Description:
// This method can be called recursively for streaming.
// The extent of the outRegion changes, dim remains the same.
void vtkImageFilter::RecursiveStreamUpdate(vtkImageData *outData,
					   int splitAxis)
{
  int memory;
  vtkImageData *inData;
  
  // Compute the required input region extent.
  // Copy to fill in extent of extra dimensions.
  this->ComputeRequiredInputUpdateExtent(this->Input->GetUpdateExtent(),
					 this->Output->GetUpdateExtent());
    
  // determine the amount of memory that will be used by the input region.
  memory = this->Input->GetUpdateExtentMemorySize();
  
  // Split the inRegion if we are streaming.
  if ((memory > this->Input->GetMemoryLimit()))
    {
    int min, max, mid;
    this->Output->GetAxisUpdateExtent(splitAxis,min,max);
    while ( (min == max) && splitAxis > 0)
      {
      splitAxis--;
      this->Output->GetAxisUpdateExtent(splitAxis,min,max);
      }
    // Make sure we can actually split the axis
    if (min < max)
      {
      // Set the first half to update
      mid = (min + max) / 2;
      vtkDebugMacro(<< "RecursiveStreamUpdate: Splitting " 
      << splitAxis << " : memory = " << memory <<
      ", extent = " << min << "->" << mid << " | " << mid+1 << "->" << max);
      this->Output->SetAxisUpdateExtent(splitAxis, min, mid);
      this->RecursiveStreamUpdate(outData, splitAxis);
      // Set the second half to update
      this->Output->SetAxisUpdateExtent(splitAxis, mid+1, max);
      this->RecursiveStreamUpdate(outData, splitAxis);
      // Restore the original extent
      this->Output->SetAxisUpdateExtent(splitAxis, min, max);
      return;
      }
    else
      {
      // Cannot split any more.  Ignore memory limit and continue.
      vtkWarningMacro(<< "RecursiveStreamUpdate: Cannot split. memory = "
        << memory << ", " << splitAxis << " : " << min << "->" << max);
      }
    }

  // No Streaming required.
  // Get the input region (Update extent was set at start of this method).
  inData = this->Input->UpdateAndReturnData();

  // The StartMethod call is placed here to be after updating the input.
  if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
  // fill the output region 
  this->Execute(inData, outData);
  if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
  
  // Like the graphics pipeline this source releases inputs data.
  if (this->Input->ShouldIReleaseData())
    {
    this->Input->ReleaseData();
    }
}


//----------------------------------------------------------------------------
// Description:
// This method sets the WholeExtent, Spacing and Origin of the output.
void vtkImageFilter::UpdateImageInformation()
{
  // Make sure the Input has been set.
  if ( ! this->Input)
    {
    vtkErrorMacro(<< "UpdateImageInformation: Input is not set.");
    return;
    }
  // make sure we have an output
  this->CheckCache();
  
  this->Input->UpdateImageInformation();
  // Set up the defaults
  this->Output->SetWholeExtent(this->Input->GetWholeExtent());
  this->Output->SetSpacing(this->Input->GetSpacing());
  this->Output->SetOrigin(this->Input->GetOrigin());
  this->Output->SetScalarType(this->Input->GetScalarType());
  this->Output->
    SetNumberOfScalarComponents(this->Input->GetNumberOfScalarComponents());

  if ( ! this->Bypass)
    {
    // Let the subclass modify the default.
    this->ExecuteImageInformation();
    }
}



//----------------------------------------------------------------------------
// Description:
// This method can be overriden in a subclass to compute the output
// ImageInformation: WholeExtent, Spacing and Origin.
void vtkImageFilter::ExecuteImageInformation()
{
}


//----------------------------------------------------------------------------
// Description:
// This method can be overriden in a subclass to compute the input
// UpdateExtent needed to generate the output UpdateExtent.
// By default the input is set to the same as the output before this
// method is called.
void vtkImageFilter::ComputeRequiredInputUpdateExtent(int inExt[6], int outExt[6])
{
  memcpy(inExt,outExt,sizeof(int)*6);
}

struct vtkImageThreadStruct
{
  vtkImageFilter *Filter;
  vtkImageData   *Input;
  vtkImageData   *Output;
};



// this mess is really a simple function. All it does is call
// the ThreadedExecute method after setting the correct
// extent for this thread. Its just a pain to calculate
// the correct extent.
VTK_THREAD_RETURN_TYPE vtkImageThreadedExecute( void *arg )
{
  vtkImageThreadStruct *str;
  int ext[6];
  int threadId, threadCount;
  int axis;
  
  threadId = ((ThreadInfoStruct *)(arg))->ThreadID;
  threadCount = ((ThreadInfoStruct *)(arg))->NumberOfThreads;

  str = (vtkImageThreadStruct *)(((ThreadInfoStruct *)(arg))->UserData);
  
  memcpy(ext,str->Filter->GetOutput()->GetUpdateExtent(),
	 sizeof(int)*6);

  // execute the actual method with appropriate extent
  // first find a non degenerate axis
  axis = 2;
  while ((axis > 0) && 
	 ((ext[axis*2+1] - ext[axis*2]) == 0)) axis--;
  
  // is there is no division then only execute for thread zero
  if (axis == 0)
    {
    if (threadId == 0)
      str->Filter->ThreadedExecute(str->Input, str->Output, ext, threadId);
    }
  else
    {
    // find correct extent
    int range = ext[axis*2+1] - ext[axis*2] + 1;
    int valuesPerThread = (int)ceil(range/(double)threadCount);
    int maxThreadIdUsed = (int)ceil(range/(double)valuesPerThread) - 1;
    if (threadId < maxThreadIdUsed)
      {
      ext[axis*2] = ext[axis*2] + threadId*valuesPerThread;
      ext[axis*2+1] = ext[axis*2] + valuesPerThread - 1;
      str->Filter->ThreadedExecute(str->Input, str->Output, ext, threadId);
      }
    if (threadId == maxThreadIdUsed)
      {
      ext[axis*2] = ext[axis*2] + threadId*valuesPerThread;
      str->Filter->ThreadedExecute(str->Input, str->Output, ext, threadId);
      }
    // otherwise don't use this thread. Sometimes the threads dont
    // break up very well and it is just as efficient to leave a 
    // few threads idle.
    }
  
  return VTK_THREAD_RETURN_VALUE;
}


void vtkImageFilter::Execute(vtkImageData *inData, 
			     vtkImageData *outData)
{
  vtkImageThreadStruct str;
  
  str.Filter = this;
  str.Input = inData;
  str.Output = outData;
  
  this->Threader->SetNumberOfThreads(1/*this->NumberOfThreads*/);
  
  // setup threading and the invoke threadedExecute
  this->Threader->SetSingleMethod(vtkImageThreadedExecute, &str);
  this->Threader->SingleMethodExecute();
}


//----------------------------------------------------------------------------
// Description:
// The execute method created by the subclass.
void vtkImageFilter::ThreadedExecute(vtkImageData *vtkNotUsed(inData), 
				     vtkImageData *vtkNotUsed(outData),
				     int extent[6], int threadId)
{
  vtkErrorMacro("subclase should override this method!!!");
}
