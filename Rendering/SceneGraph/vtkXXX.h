/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXXX.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkXXX -
// .SECTION Description
//

#ifndef vtkXXX_h
#define vtkXXX_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkObject.h"


class VTKRENDERINGSCENEGRAPH_EXPORT vtkXXX :
  public vtkObject
{
public:
  static vtkXXX* New();
  vtkTypeMacro(vtkXXX, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkXXX();
  ~vtkXXX();

private:
  vtkXXX(const vtkXXX&); // Not implemented.
  void operator=(const vtkXXX&); // Not implemented.

  class vtkInternals;
  vtkInternals *Internals;
};

#endif
