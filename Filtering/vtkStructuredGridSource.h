/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStructuredGridSource.h
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
// .NAME vtkStructuredGridSource - Abstract class whose subclasses generates structured grid data
// .SECTION Description
// vtkStructuredGridSource is an abstract class whose subclasses generate
// structured grid data.

// .SECTION See Also
// vtkStructuredGridReader vtkPLOT3DReader

#ifndef __vtkStructuredGridSource_h
#define __vtkStructuredGridSource_h

#include "vtkSource.h"
#include "vtkStructuredGrid.h"

class VTK_FILTERING_EXPORT vtkStructuredGridSource : public vtkSource
{
public:
  static vtkStructuredGridSource *New();
  vtkTypeMacro(vtkStructuredGridSource,vtkSource);

  // Description:
  // Get the output of this source.
  vtkStructuredGrid *GetOutput();
  vtkStructuredGrid *GetOutput(int idx)
    {return (vtkStructuredGrid *) this->vtkSource::GetOutput(idx); };
  void SetOutput(vtkStructuredGrid *output);  

protected:
  vtkStructuredGridSource();
  ~vtkStructuredGridSource() {};

private:
  vtkStructuredGridSource(const vtkStructuredGridSource&);  // Not implemented.
  void operator=(const vtkStructuredGridSource&);  // Not implemented.
};

#endif


