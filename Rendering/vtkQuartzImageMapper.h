/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQuartzImageMapper.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkQuartzImageMapper - 2D image display support for Quartz windows
// .SECTION Description
// vtkQuartzImageMapper is a concrete subclass of vtkImageMapper that
// renders images under Quartz.

// .SECTION See Also
// vtkImageMapper

#ifndef __vtkQuartzImageMapper_h
#define __vtkQuartzImageMapper_h


#include "vtkImageMapper.h"
class vtkImageActor2D;


class VTK_EXPORT vtkQuartzImageMapper : public vtkImageMapper
{
public:
  static vtkQuartzImageMapper *New();
  vtkTypeMacro(vtkQuartzImageMapper,vtkImageMapper);
  
  // Description:
  // Handle the render method.
  void RenderOverlay(vtkViewport* viewport, vtkActor2D* actor) {
    this->RenderStart(viewport,actor);}

  // Description:
  // Called by the Render function in vtkImageMapper.  Actually draws
  // the image to the screen.
  void RenderData(vtkViewport* viewport, vtkImageData* data, 
		  vtkActor2D* actor);

  unsigned char *DataOut;	// the data in the DIBSection
  void *HBitmap;			// our handle to the DIBSection

protected:
  vtkQuartzImageMapper();
  ~vtkQuartzImageMapper();
  vtkQuartzImageMapper(const vtkQuartzImageMapper&) {};
  void operator=(const vtkQuartzImageMapper&) {};

};


#endif









