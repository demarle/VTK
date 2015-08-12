/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkViewNodeFactory.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkViewNodeFactory -
// .SECTION Description
//

#ifndef vtkViewNodeFactory_h
#define vtkViewNodeFactory_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkObject.h"

class vtkViewNode;

class VTKRENDERINGSCENEGRAPH_EXPORT vtkViewNodeFactory :
  public vtkObject
{
public:
  static vtkViewNodeFactory* New();
  vtkTypeMacro(vtkViewNodeFactory, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //this...
  vtkViewNode *CreateNode(vtkObject *);

  //Description:
  //this...
  vtkViewNode *CreateNode(const char *);

  //Description:
  //this...
  void RegisterOverride(const char *name, vtkViewNode *(*func)());

protected:
  vtkViewNodeFactory();
  ~vtkViewNodeFactory();

private:
  vtkViewNodeFactory(const vtkViewNodeFactory&); // Not implemented.
  void operator=(const vtkViewNodeFactory&); // Not implemented.

  class vtkInternals;
  vtkInternals *Internals;
};

#endif
