/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInformationInformationVectorKey.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInformationInformationVectorKey.h"

#include "vtkInformationVector.h"
#include "vtkInformation.h"

vtkCxxRevisionMacro(vtkInformationInformationVectorKey, "1.5");

//----------------------------------------------------------------------------
vtkInformationInformationVectorKey::vtkInformationInformationVectorKey(const char* name, const char* location):
  vtkInformationKey(name, location)
{
  vtkFilteringInformationKeyManager::Register(this);
}

//----------------------------------------------------------------------------
vtkInformationInformationVectorKey::~vtkInformationInformationVectorKey()
{
}

//----------------------------------------------------------------------------
void vtkInformationInformationVectorKey::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkInformationInformationVectorKey::Set(vtkInformation* info,
                                             vtkInformationVector* value)
{
  this->SetAsObjectBase(info, value);
}

//----------------------------------------------------------------------------
vtkInformationVector*
vtkInformationInformationVectorKey::Get(vtkInformation* info)
{
  return vtkInformationVector::SafeDownCast(this->GetAsObjectBase(info));
}

//----------------------------------------------------------------------------
int vtkInformationInformationVectorKey::Has(vtkInformation* info)
{
  return vtkInformationVector::SafeDownCast(this->GetAsObjectBase(info))?1:0;
}

//----------------------------------------------------------------------------
void vtkInformationInformationVectorKey::ShallowCopy(vtkInformation* from,
                                              vtkInformation* to)
{
  this->Set(to, this->Get(from));
}

//----------------------------------------------------------------------------
void vtkInformationInformationVectorKey::DeepCopy(vtkInformation* from,
                                              vtkInformation* to)
{
  vtkInformationVector *fromVector = this->Get(from);
  vtkInformationVector *toVector = vtkInformationVector::New();
  vtkInformation *toInfo;
  int i;

  for (i = 0; i < fromVector->GetNumberOfInformationObjects(); i++)
    {
    toInfo = vtkInformation::New();
    toInfo->Copy(fromVector->GetInformationObject(i), 1);
    toVector->Append(toInfo);
    toInfo->Delete();
    }
  this->Set(to, toVector);
  toVector->Delete();
}

//----------------------------------------------------------------------------
void vtkInformationInformationVectorKey::Report(vtkInformation* info,
                                                vtkGarbageCollector* collector)
{
  this->ReportAsObjectBase(info, collector);
}
