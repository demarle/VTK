/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGeneralTransformInverse.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to David G. Gobbi who developed this class.

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

#include "vtkGeneralTransformInverse.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkGeneralTransformInverse* vtkGeneralTransformInverse::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGeneralTransformInverse");
  if(ret)
    {
    return (vtkGeneralTransformInverse*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkGeneralTransformInverse;
}

//----------------------------------------------------------------------------
vtkGeneralTransformInverse::vtkGeneralTransformInverse()
{
  this->Transform = NULL;
  this->UpdateRequired = 0;
  this->UpdateMutex = vtkMutexLock::New();
}

//----------------------------------------------------------------------------
vtkGeneralTransformInverse::~vtkGeneralTransformInverse()
{
  if (this->Transform)
    {
    this->Transform->Delete();
    }
  if (this->UpdateMutex)
    {
    this->UpdateMutex->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkGeneralTransform::PrintSelf(os,indent);

  os << indent << "Transform: " << this->Transform << "\n";
  if (this->Transform)
    {
    this->Transform->PrintSelf(os,indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::SetInverse(vtkGeneralTransform *trans)
{
  if (this == trans)
    {
    vtkErrorMacro(<<"SetInverse: A transform cannot be its own inverse!");
    return;
    }
  if (this->MyInverse == trans)
    {
    return;
    }
  if (this->MyInverse)
    {
    this->MyInverse->Delete();
    this->Transform->Delete();
    }
  this->MyInverse = trans;
  trans->Register(this);
  this->Transform = trans->MakeTransform();

  this->UpdateRequired = 1;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::InternalTransformPoint(const float in[3], 
							float out[3])
{
  this->Transform->InternalTransformPoint(in,out);
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::InternalTransformDerivative(
						      const float in[3],
						      float out[3],
						      float derivative[3][3])
{
  this->Transform->InternalTransformDerivative(in,out,derivative);
}

//----------------------------------------------------------------------------
vtkGeneralTransform *vtkGeneralTransformInverse::GetInverse()
{
  return this->MyInverse;
}

//----------------------------------------------------------------------------
vtkGeneralTransform *vtkGeneralTransformInverse::GetTransform()
{
  return this->Transform;
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::Identity()
{
  if (this->MyInverse == NULL)
    {
    vtkErrorMacro(<< "Identity: Inverse has not been set");
    return;
    }
  this->MyInverse->Identity();
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::Inverse()
{
  if (this->MyInverse == NULL)
    {
    vtkErrorMacro(<< "Inverse: Inverse has not been set");
    return;
    }
  this->MyInverse->Inverse();
}

//----------------------------------------------------------------------------
vtkGeneralTransform *vtkGeneralTransformInverse::MakeTransform()
{
  if (this->MyInverse == NULL)
    {
    vtkErrorMacro(<< "MakeTransform: Inverse has not been set");
    return NULL;
    }
  return this->MyInverse->MakeTransform();
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::DeepCopy(vtkGeneralTransform *transform)
{
  if (this->MyInverse == NULL)
    {
    vtkErrorMacro(<< "DeepCopy: Inverse has not been set");
    return;
    }
  this->MyInverse->DeepCopy(transform);
  this->MyInverse->Inverse();
}

//----------------------------------------------------------------------------
void vtkGeneralTransformInverse::Update()
{
  // lock the update just in case multiple threads update simultaneously
  this->UpdateMutex->Lock();

  if (this->MyInverse->GetMTime() > 
      this->Transform->GetMTime() || this->UpdateRequired)
    {
    this->Transform->DeepCopy(this->MyInverse);
    this->Transform->Inverse();
    this->UpdateRequired = 0;
    }
  this->Transform->Update();

  this->UpdateMutex->Unlock();
}

//----------------------------------------------------------------------------
unsigned long vtkGeneralTransformInverse::GetMTime()
{
  unsigned long result = this->vtkGeneralTransform::GetMTime();

  if (this->MyInverse)
    {
    unsigned long mtime = this->MyInverse->GetMTime();
    if (mtime > result)
      {
      result = mtime;
      }
    }
  return result;
}
