/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageSource.h
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
// .NAME vtkImageSource - Source of data for pipeline.
// .SECTION Description
// vtkImageSource is the supperclass for all sources and filters.
// The method Update, called by the cache, is the major interface
// to the source.

// .SECTION See Also
// vtkImageCache vtkImageRegion


#ifndef __vtkImageSource_h
#define __vtkImageSource_h

#include "vtkImageData.h"
class vtkImageCache;


class VTK_EXPORT vtkImageSource : public vtkObject
{
public:
  vtkImageSource();
  ~vtkImageSource();
  const char *GetClassName() {return "vtkImageSource";};
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void InterceptCacheUpdate();
  virtual void InternalUpdate(vtkImageData *data);
  virtual void Update();
  virtual void UpdateWholeExtent();
  virtual void UpdateImageInformation() = 0;

  virtual unsigned long GetPipelineMTime();
  vtkImageCache *GetOutput();

  virtual void SetCache(vtkImageCache *cache);
  vtkImageCache *GetCache();

  virtual void SetReleaseDataFlag(int value);
  int  GetReleaseDataFlag();
  vtkBooleanMacro(ReleaseDataFlag, int);
  
  // Description:
  // Set/Get the AbortExecute flag for the filter. It's up to the filter writer
  // to handle premature ending of a filter
  vtkSetMacro(AbortExecute,int);
  vtkGetMacro(AbortExecute,int);
  vtkBooleanMacro(AbortExecute,int);

  // Description:
  // Specify progress of a filter.
  vtkSetClampMacro(Progress,float,0.0,1.0);
  vtkGetMacro(Progress,float);

  virtual void SetStartMethod(void (*f)(void *), void *arg);
  virtual void SetProgressMethod(void (*f)(void *), void *arg);
  virtual void SetEndMethod(void (*f)(void *), void *arg);
  virtual void SetStartMethodArgDelete(void (*f)(void *));
  virtual void SetProgressMethodArgDelete(void (*f)(void *));
  virtual void SetEndMethodArgDelete(void (*f)(void *));
  
  // Description:
  // Update the progress of a filter. If a ProgressMEthod, exists, 
  // executes it. Then sets the Progress ivar to amount.
  void UpdateProgress(float amount);

  // Description:
  // subclass can over ride this method to do custom streaming and
  // splitting for multiprocessing.
  virtual int SplitExtent(int splitExt[6], int startExt[6], 
			  int num, int total);

protected:
  vtkImageCache *Output;

  void (*StartMethod)(void *);
  void (*StartMethodArgDelete)(void *);
  void *StartMethodArg;
  void (*ProgressMethod)(void *);
  void *ProgressMethodArg;
  void (*ProgressMethodArgDelete)(void *);
  void (*EndMethod)(void *);
  void (*EndMethodArgDelete)(void *);
  void *EndMethodArg;
  float Progress;
  int AbortExecute;
  
  virtual void Execute(vtkImageData *data); 
  virtual void CheckCache();
};


#endif


