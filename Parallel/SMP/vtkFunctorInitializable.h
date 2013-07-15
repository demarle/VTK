/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkFunctorInitializable.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFunctorInitializable - abstract base class to derive from if
// you need extra initialization in a vtkFunctor
// .SECTION Description
// ?

#ifndef __vtkFunctorInitializable_h__
#define __vtkFunctorInitializable_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkFunctor.h"
#include <vector> //No STL in API in core

class VTKPARALLELSMP_EXPORT vtkFunctorInitializable : public vtkFunctor
{
public:
  vtkTypeMacro(vtkFunctorInitializable,vtkFunctor);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //?
  virtual void Init ( ) const = 0;

  //Description:
  //?
  bool ShouldInitialize( ) const;

protected:
  vtkFunctorInitializable();
  ~vtkFunctorInitializable();

  //Description:
  //?
  void Initialized( ) const;

  //Description:
  //?
  mutable vtkstd::vector<vtkIdType> IsInitialized;

private:
  vtkFunctorInitializable(const vtkFunctorInitializable&);  // Not implemented.
  void operator=(const vtkFunctorInitializable&);  // Not implemented.
};

#endif
