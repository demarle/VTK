/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAbstractPropPicker.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAbstractPropPicker.h"

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkAssembly.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkObjectFactory.h"
#include "vtkPropAssembly.h"
#include "vtkVolume.h"

vtkCxxRevisionMacro(vtkAbstractPropPicker, "1.8");

vtkCxxSetObjectMacro(vtkAbstractPropPicker,Path,vtkAssemblyPath);

vtkAbstractPropPicker::vtkAbstractPropPicker()
{
  this->Path = NULL;
}

vtkAbstractPropPicker::~vtkAbstractPropPicker()
{
  if ( this->Path )
    {
    this->Path->Delete();
    }
}

// set up for a pick
void vtkAbstractPropPicker::Initialize()
{
  this->vtkAbstractPicker::Initialize();
  if ( this->Path )
    {
    this->Path->Delete();
    this->Path = NULL;
    }
}

//----------------------------------------------------------------------------
#ifdef VTK_WORKAROUND_WINDOWS_MANGLE
# undef GetProp
// Define possible mangled names.
vtkProp* vtkAbstractPropPicker::GetPropA()
{
  return this->GetPropInternal();
}
vtkProp* vtkAbstractPropPicker::GetPropW()
{
  return this->GetPropInternal();
}
#endif
vtkProp* vtkAbstractPropPicker::GetProp()
{
  return this->GetPropInternal();
}

//----------------------------------------------------------------------------
vtkProp* vtkAbstractPropPicker::GetPropInternal()
{
  if ( this->Path != NULL )
    {
    return this->Path->GetFirstNode()->GetProp();
    }
  else
    {
    return NULL;
    }
}

vtkProp3D *vtkAbstractPropPicker::GetProp3D()
{
  if ( this->Path != NULL )
    {
    vtkProp *prop = this->Path->GetFirstNode()->GetProp();
    return vtkProp3D::SafeDownCast(prop);
    }
  else
    {
    return NULL;
    }
}

vtkActor *vtkAbstractPropPicker::GetActor()
{
  if ( this->Path != NULL )
    {
    vtkProp *prop = this->Path->GetFirstNode()->GetProp();
    return vtkActor::SafeDownCast(prop);
    }
  else
    {
    return NULL;
    }
}

vtkActor2D *vtkAbstractPropPicker::GetActor2D()
{
  if ( this->Path != NULL )
    {
    vtkProp *prop = this->Path->GetFirstNode()->GetProp();
    return vtkActor2D::SafeDownCast(prop);
    }
  else
    {
    return NULL;
    }
}

vtkVolume *vtkAbstractPropPicker::GetVolume()
{
  if ( this->Path != NULL )
    {
    vtkProp *prop = this->Path->GetFirstNode()->GetProp();
    return vtkVolume::SafeDownCast(prop);
    }
  else
    {
    return NULL;
    }
}

vtkAssembly *vtkAbstractPropPicker::GetAssembly()
{
  if ( this->Path != NULL )
    {
    vtkProp *prop = this->Path->GetFirstNode()->GetProp();
    return vtkAssembly::SafeDownCast(prop);
    }
  else
    {
    return NULL;
    }
}

vtkPropAssembly *vtkAbstractPropPicker::GetPropAssembly()
{
  if ( this->Path != NULL )
    {
    vtkProp *prop = this->Path->GetFirstNode()->GetProp();
    return vtkPropAssembly::SafeDownCast(prop);
    }
  else
    {
    return NULL;
    }
}
  
void vtkAbstractPropPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  if ( this->Path )
    {
    os << indent << "Path: " << this->Path << endl;
    }
  else
    {
    os << indent << "Path: (none)" << endl;
    }
}
