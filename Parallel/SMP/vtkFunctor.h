/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkFunctor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFunctor - !!!!
// .SECTION Description
// !!!!

#ifndef __vtkFunctor_h__
#define __vtkFunctor_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"

class VTKPARALLELSMP_EXPORT vtkFunctor : public vtkObject
{
public:
  vtkTypeMacro(vtkFunctor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void operator () ( vtkIdType ) const = 0;

protected:
  vtkFunctor();
  ~vtkFunctor();

private:
  vtkFunctor(const vtkFunctor&);  // Not implemented.
  void operator=(const vtkFunctor&);  // Not implemented.

};

#endif
