/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageIterator.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// Include blockers needed since vtkImageIterator.h includes this file
// when VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION is defined.
#ifndef __vtkImageIterator_txx
#define __vtkImageIterator_txx

#include "vtkImageIterator.h"
#include "vtkImageData.h"

template <class DType>
vtkImageIterator<DType>::vtkImageIterator(vtkImageData *id, int *ext)
{
  this->Pointer = (DType *)id->GetScalarPointerForExtent(ext);
  id->GetIncrements(this->Increments[0], this->Increments[1], 
                    this->Increments[2]);
  id->GetContinuousIncrements(ext,this->ContinuousIncrements[0], 
                              this->ContinuousIncrements[1], 
                              this->ContinuousIncrements[2]);
  this->EndPointer = 
    (DType *)id->GetScalarPointer(ext[1],ext[3],ext[5]) +this->Increments[0];

  this->SpanEndPointer = 
    this->Pointer + this->Increments[0]*(ext[1] - ext[0] + 1);
  this->SliceEndPointer = 
    this->Pointer + this->Increments[1]*(ext[3] - ext[2] + 1);
}
  
  
template <class DType>
void vtkImageIterator<DType>::NextSpan()
{
  this->Pointer += this->Increments[1];
  this->SpanEndPointer += this->Increments[1];
  if (this->Pointer >= this->SliceEndPointer)
    {
    this->Pointer += this->ContinuousIncrements[2];
    this->SpanEndPointer += this->ContinuousIncrements[2];
    this->SliceEndPointer += this->Increments[2];
    }
}

#endif
