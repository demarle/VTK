/*=========================================================================
  
  Program:   Visualization Toolkit
  Module:    vtkTreeComposite.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  
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

// This software and ancillary information known as vtk_ext (and
// herein called "SOFTWARE") is made available under the terms
// described below.  The SOFTWARE has been approved for release with
// associated LA_CC Number 99-44, granted by Los Alamos National
// Laboratory in July 1999.
//
// Unless otherwise indicated, this SOFTWARE has been authored by an
// employee or employees of the University of California, operator of
// the Los Alamos National Laboratory under Contract No. W-7405-ENG-36
// with the United States Department of Energy.
//
// The United States Government has rights to use, reproduce, and
// distribute this SOFTWARE.  The public may copy, distribute, prepare
// derivative works and publicly display this SOFTWARE without charge,
// provided that this Notice and any statement of authorship are
// reproduced on all copies.
//
// Neither the U. S. Government, the University of California, nor the
// Advanced Computing Laboratory makes any warranty, either express or
// implied, nor assumes any liability or responsibility for the use of
// this SOFTWARE.
//
// If SOFTWARE is modified to produce derivative works, such modified
// SOFTWARE should be clearly marked, so as not to confuse it with the
// version available from Los Alamos National Laboratory.


// .NAME vtkTreeComposite - An object to composite multiple render windows.
// .SECTION Description
// vtkTreeComposite operates in multiple processes.  Each compositer has 
// a render window.  They use vtkMultiProcessControllers to comunicate 
// the color and depth buffer to process 0's render window.
// It will not handle transparency well.
// .SECTION note
// You should set up the renders and render window interactor before setting
// the compositers render window.  We set up observers on the renderer,
// An have no easy way of knowing when the renderers change.  We could 
// create AddRenderer and RemoveRenderer events ...
// .SECTION see also
// vtkMultiProcessController vtkRenderWindow.

#ifndef __vtkTreeComposite_h
#define __vtkTreeComposite_h

#include "vtkObject.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkMultiProcessController.h"

class VTK_EXPORT vtkTreeComposite : public vtkObject
{
public:
  static vtkTreeComposite *New();
  vtkTypeMacro(vtkTreeComposite,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the RenderWindow to use for compositing.
  // We add a start and end observer to the window.
  vtkGetObjectMacro(RenderWindow, vtkRenderWindow);
  void SetRenderWindow(vtkRenderWindow *renWin);

  // Description:
  // This method sets the piece and number of pieces for each
  // actor with a polydata mapper. My other option is to 
  // do it every render, but that would force a partioning scheme.
  void InitializePieces();
  
  // Description:
  // turn off screen rendering on for the
  void InitializeOffScreen();
  
  // Description:
  // Callbacks that initialize and finish the compositing.
  void StartInteractor();
  void ExitInteractor();
  void StartRender();
  void EndRender();
  void RenderRMI();
  void ResetCamera(vtkRenderer *ren);
  void ResetCameraClippingRange(vtkRenderer *ren);
  void ComputeVisiblePropBoundsRMI();
  
protected:
  vtkTreeComposite();
  ~vtkTreeComposite();
  vtkTreeComposite(const vtkTreeComposite&) {};
  void operator=(const vtkTreeComposite&) {};
  
  vtkRenderWindow* RenderWindow;
  vtkRenderWindowInteractor* RenderWindowInteractor;
  vtkMultiProcessController* Controller;

  unsigned long StartInteractorTag;
  unsigned long EndInteractorTag;
  unsigned long StartTag;
  unsigned long EndTag;
  unsigned long ResetCameraTag;
  unsigned long ResetCameraClippingRangeTag;
  
  void Composite(int flag);

  // Convenience method used internally. It set up the start observer
  // and allows the render window's interactor to be set before or after
  // the compositer's render window (not exactly true).
  void SetRenderWindowInteractor(vtkRenderWindowInteractor *iren);

  void ComputeVisiblePropBounds(vtkRenderer *ren, float bounds[6]);

  // Arrays for compositing.
  float *PData;
  float *ZData;
  int WindowSize[2];
  
  // This flag stops nested RMIs from occuring.  Some rmis send and receive information.
  // Nesting them can lock up the processes.
  int Lock;
  
  
  void SetWindowSize(int x, int y);
};


#endif









