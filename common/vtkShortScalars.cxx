/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkShortScalars.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkShortScalars.h"

vtkShortScalars::vtkShortScalars()
{
  this->S = vtkShortArray::New();
}

vtkShortScalars::vtkShortScalars(const vtkShortScalars& ss)
{
  this->S = vtkShortArray::New();
  *(this->S) = *(ss.S);
}

vtkShortScalars::vtkShortScalars(const int sz, const int ext)
{
  this->S = vtkShortArray::New();
  this->S->Allocate(sz,ext);
}

vtkShortScalars::~vtkShortScalars()
{
  this->S->Delete();
}



vtkScalars *vtkShortScalars::MakeObject(int sze, int ext)
{
  return new vtkShortScalars(sze,ext);
}

// Description:
// Deep copy of scalars.
vtkShortScalars& vtkShortScalars::operator=(const vtkShortScalars& ss)
{
  *(this->S) = *(ss.S);
  return *this;
}

void vtkShortScalars::GetScalars(vtkIdList& ptId, vtkFloatScalars& fs)
{
  int NumberOfIds = ptId.GetNumberOfIds();
  float *value = fs.WritePointer (0, NumberOfIds);

  for (int i=0; i<NumberOfIds; i++)
    {
    *value++ = this->S->GetValue(ptId.GetId(i));
    }
}

void vtkShortScalars::GetScalars(int p1, int p2, vtkFloatScalars& fs)
{
  float *fp=fs.GetPointer(0);
  short *sp=this->S->GetPointer(p1);
  
  for (int id=p1; id <= p2; id++)
    {
    *fp++ = (float)*sp++;
    }
}
