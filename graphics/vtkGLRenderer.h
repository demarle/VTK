/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGLRenderer.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkGLRenderer - SGI gl renderer
// .SECTION Description
// vtkGLRenderer is a concrete implementation of the abstract class
// vtkRenderer. vtkGLRenderer interfaces to the Silicon Graphics gl
// graphics library.

#ifndef __vtkGLRenderer_h
#define __vtkGLRenderer_h

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "vtkRenderer.h"
#include "gl.h"

class VTK_EXPORT vtkGLRenderer : public vtkRenderer
{
 protected:
  int NumberOfLightsBound;

 public:
  vtkGLRenderer();

  void Render(void); // overides base 
  static vtkGLRenderer *New() {return new vtkGLRenderer;};
  const char *GetClassName() {return "vtkGLRenderer";};
  void PrintSelf(ostream& os, vtkIndent indent);

  void ClearLights(void);

  int UpdateActors(void);
  int UpdateVolumes(void);
  int UpdateCameras(void);
  int UpdateLights(void);

  // stereo related stuff
  virtual float *GetCenter();
  virtual void DisplayToView(); 
  virtual void ViewToDisplay(); 
  virtual int  IsInViewport(int x,int y); 
};

#endif
