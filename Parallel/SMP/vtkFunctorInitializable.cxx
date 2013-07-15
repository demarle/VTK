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

#include "vtkFunctorInitializable.h"

extern int vtkSMPInternalGetNumberOfThreads();
extern int vtkSMPInternalGetTid();

//======================================================================================
vtkFunctorInitializable::vtkFunctorInitializable() :
    vtkFunctor(), IsInitialized(vtkSMPInternalGetNumberOfThreads(), 0)
  {
  }

//--------------------------------------------------------------------------------
vtkFunctorInitializable::~vtkFunctorInitializable()
  {
  IsInitialized.clear();
  }

//--------------------------------------------------------------------------------
bool vtkFunctorInitializable::ShouldInitialize( ) const
  {
  return !IsInitialized[vtkSMPInternalGetTid()];
  }

//--------------------------------------------------------------------------------
void vtkFunctorInitializable::Initialized( ) const
  {
  IsInitialized[vtkSMPInternalGetTid()] = 1;
  }

//--------------------------------------------------------------------------------
void vtkFunctorInitializable::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Is initialized: " << endl;
  for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < IsInitialized.size(); ++i )
    {
    os << indent.GetNextIndent() << "Id " << i << ": ";
    if ( IsInitialized[i] )
      {
      os << "true";
      }
    else
      {
      os << "false";
      }
    os << endl;
    }
  }
