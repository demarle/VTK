/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWorldPointPicker.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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
// .NAME vtkWorldPointPicker - find world x,y,z corresponding to display x,y,z
// .SECTION Description
// vtkWorldPointPicker is used to find the x,y,z world coordinate of a
// screen x,y,z. This picker cannot pick actors and/or mappers, it
// simply determines an x-y-z coordinate in world space. (It will always
// return a x-y-z, even if the selection point is not over a prop/actor.)

// .SECTION Caveats 
// The PickMethod() is not invoked, but StartPickMethod() and EndPickMethod()
// are.

// .SECTION See Also 
// vtkPropPicker vtkPicker vtkCellPicker vtkPointPicker 

#ifndef __vtkWorldPointPicker_h
#define __vtkWorldPointPicker_h

#include "vtkAbstractPicker.h"

class VTK_EXPORT vtkWorldPointPicker : public vtkAbstractPicker
{
public:
  static vtkWorldPointPicker *New();
  vtkTypeMacro(vtkWorldPointPicker,vtkAbstractPicker);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Perform the pick. (This method overload's the superclass.)
  int Pick(float selectionX, float selectionY, float selectionZ, 
           vtkRenderer *renderer);  
  int Pick(float selectionPt[3], vtkRenderer *renderer)
    { return this->vtkAbstractPicker::Pick( selectionPt, renderer); };  

protected:
  vtkWorldPointPicker ();
  ~vtkWorldPointPicker() {};
  vtkWorldPointPicker(const vtkWorldPointPicker&) {};
  void operator=(const vtkWorldPointPicker&) {};

};

#endif


