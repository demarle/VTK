/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImagePolyDataClippingExtents.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to David G. Gobbi who developed this class.

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkImagePolyDataClippingExtents - helper class for clipping images
// .SECTION Description
// vtkImagePolyDataClippingExtents is a class that can be used with 
// vtkImageStencil to perform stencil operations on an image using
// a closed vtkPolyData surface.  If the vtkPolyData surface is not
// closed, then the results are undefined.  To use this class,
// call the SetClippingObject() method with a vtkPolyData object,
// and then connect this class to vtkImageStencil using the
// SetClippingExtents method of vtkImageStencil.
// .SECTION see also
// vtkImageStencil vtkPolyData

#ifndef __vtkImagePolyDataClippingExtents_h
#define __vtkImagePolyDataClippingExtents_h


#include "vtkImageClippingExtents.h"
#include "vtkImageData.h"
#include "vtkOBBTree.h"

class VTK_EXPORT vtkImagePolyDataClippingExtents : public vtkImageClippingExtents
{
public:
  static vtkImagePolyDataClippingExtents *New();
  vtkTypeMacro(vtkImagePolyDataClippingExtents, vtkImageClippingExtents);

  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkImagePolyDataClippingExtents();
  ~vtkImagePolyDataClippingExtents();
  vtkImagePolyDataClippingExtents(const vtkImagePolyDataClippingExtents&) {};
  void operator=(const vtkImagePolyDataClippingExtents&) {};

  // Description:
  // This method is called prior to ThreadedBuildExtents, i.e. before
  // the execution threads have been split off.  It is used to do
  // any preparatory work that is necessary before ThreadedBuildExtents().
  void PrepareForThreadedBuildExtents();

  // Description:
  // Override this method to support clipping with different kinds
  // of objects.  Eventually the extent could be split up and handled
  // by multiple threads, but it isn't for now.  But please ensure
  // that all code inside this method is thread-safe.
  void ThreadedBuildExtents(int extent[6], int threadId);

  vtkOBBTree *OBBTree;
};

#endif
