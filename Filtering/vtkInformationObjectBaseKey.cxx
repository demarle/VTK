/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInformationObjectBaseKey.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInformationObjectBaseKey.h"

#include "vtkGarbageCollector.h"
#include "vtkInformation.h" // For vtkErrorWithObjectMacro

vtkCxxRevisionMacro(vtkInformationObjectBaseKey, "1.1.2.1");

//----------------------------------------------------------------------------
vtkInformationObjectBaseKey
::vtkInformationObjectBaseKey(const char* name, const char* location,
                              const char* requiredClass):
  vtkInformationKey(name, location), RequiredClass(requiredClass)
{
}

//----------------------------------------------------------------------------
vtkInformationObjectBaseKey::~vtkInformationObjectBaseKey()
{
}

//----------------------------------------------------------------------------
void vtkInformationObjectBaseKey::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkInformationObjectBaseKey::Set(vtkInformation* info,
                                      vtkObjectBase* value)
{
  if(value && this->RequiredClass && !value->IsA(this->RequiredClass))
    {
    vtkErrorWithObjectMacro(
      info,
      "Cannot store object of type " << value->GetClassName()
      << " with key " << this->Location << "::" << this->Name
      << " which requires objects of type "
      << this->RequiredClass << ".  Removing the key instead.");
    this->SetAsObjectBase(info, 0);
    return;
    }
  this->SetAsObjectBase(info, value);
}

//----------------------------------------------------------------------------
vtkObjectBase* vtkInformationObjectBaseKey::Get(vtkInformation* info)
{
  return this->GetAsObjectBase(info);
}

//----------------------------------------------------------------------------
int vtkInformationObjectBaseKey::Has(vtkInformation* info)
{
  return this->GetAsObjectBase(info)?1:0;
}

//----------------------------------------------------------------------------
void vtkInformationObjectBaseKey::Copy(vtkInformation* from,
                                       vtkInformation* to)
{
  this->Set(to, this->Get(from));
}

//----------------------------------------------------------------------------
void vtkInformationObjectBaseKey::Report(vtkInformation* info,
                                         vtkGarbageCollector* collector)
{
  collector->ReportReference(this->Get(info), this->GetName());
}
