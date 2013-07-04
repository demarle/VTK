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
// .NAME vtkFunctorInitializable - !!!!
// .SECTION Description
// !!!!

#ifndef __vtkFunctorInitializable_h__
#define __vtkFunctorInitializable_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkFunctor.h"
#include <vector> //TODO PIMPLE

class VTKPARALLELSMP_EXPORT vtkFunctorInitializable : public vtkFunctor
{
public:
  vtkTypeMacro(vtkFunctorInitializable,vtkFunctor);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Init ( ) const = 0;
  bool ShouldInitialize( ) const;

protected:
  vtkFunctorInitializable();
  ~vtkFunctorInitializable();

  void Initialized( ) const;
  mutable vtkstd::vector<vtkIdType> IsInitialized;

private:
  vtkFunctorInitializable(const vtkFunctorInitializable&);  // Not implemented.
  void operator=(const vtkFunctorInitializable&);  // Not implemented.
};

#endif
