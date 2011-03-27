/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageProperty.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageProperty.h"

#include "vtkObjectFactory.h"
#include "vtkLookupTable.h"
#include "vtkColorTransferFunction.h"

vtkStandardNewMacro(vtkImageProperty);

vtkCxxSetObjectMacro(vtkImageProperty, LookupTable, vtkScalarsToColors);

//----------------------------------------------------------------------------
// Construct a new vtkImageProperty with default values
vtkImageProperty::vtkImageProperty()
{
  this->ColorWindow = 255.0;
  this->ColorLevel = 127.5;

  this->LookupTable = NULL;
  this->UseLookupTableScalarRange = 0;

  this->Opacity = 1.0;
  this->Ambient = 1.0;
  this->Diffuse = 0.0;

  this->InterpolationType = VTK_LINEAR_INTERPOLATION;

  this->Checkerboard = 0;
  this->CheckerboardSpacing[0] = 10.0;
  this->CheckerboardSpacing[1] = 10.0;
  this->CheckerboardOffset[0] = 0.0;
  this->CheckerboardOffset[1] = 0.0;
}

//----------------------------------------------------------------------------
// Destruct a vtkImageProperty
vtkImageProperty::~vtkImageProperty()
{
  if (this->LookupTable != NULL)
    {
    this->LookupTable->Delete();
    }
}

//----------------------------------------------------------------------------
const char *vtkImageProperty::GetInterpolationTypeAsString()
{
  switch (this->InterpolationType)
    {
    case VTK_NEAREST_INTERPOLATION:
      return "Nearest";
    case VTK_LINEAR_INTERPOLATION:
      return "Linear";
    case VTK_CUBIC_INTERPOLATION:
      return "Cubic";
    }
  return "";
}

//----------------------------------------------------------------------------
void vtkImageProperty::DeepCopy(vtkImageProperty *p)
{
  if (p != NULL)
    {
    this->SetColorWindow(p->GetColorWindow());
    this->SetColorLevel(p->GetColorLevel());
    vtkScalarsToColors *lut = p->GetLookupTable();
    if (lut == NULL)
      {
      this->SetLookupTable(NULL);
      }
    else
      {
      vtkLookupTable *olut = vtkLookupTable::SafeDownCast(lut);
      vtkColorTransferFunction *octf =
        vtkColorTransferFunction::SafeDownCast(lut);

      if (olut)
        {
        vtkLookupTable *nlut = olut->NewInstance();
        nlut->DeepCopy(olut);
        this->SetLookupTable(nlut);
        nlut->Delete();
        }
      else if (octf)
        {
        vtkColorTransferFunction *nctf = octf->NewInstance();
        nctf->DeepCopy(octf);
        this->SetLookupTable(nctf);
        nctf->Delete();
        }
      else
        {
        // The vtkScalarsToColors base class has no DeepCopy,
        // so fall back to shallow copy
        this->SetLookupTable(lut);
        }
      }
    this->SetUseLookupTableScalarRange(p->GetUseLookupTableScalarRange());
    this->SetOpacity(p->GetOpacity());
    this->SetAmbient(p->GetAmbient());
    this->SetDiffuse(p->GetDiffuse());
    this->SetInterpolationType(p->GetInterpolationType());
    this->SetCheckerboard(p->GetCheckerboard());
    this->SetCheckerboardSpacing(p->GetCheckerboardSpacing());
    this->SetCheckerboardOffset(p->GetCheckerboardOffset());
    }
}

//----------------------------------------------------------------------------
unsigned long int vtkImageProperty::GetMTime()
{
  unsigned long mTime = this->vtkObject::GetMTime();
  unsigned long time;

  if (this->LookupTable)
    {
    time = this->LookupTable->GetMTime();
    mTime = (mTime > time ? mTime : time);
    }

  return mTime;
}

//----------------------------------------------------------------------------
void vtkImageProperty::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ColorWindow: " << this->ColorWindow << "\n";
  os << indent << "ColorLevel: " << this->ColorLevel << "\n";
  os << indent << "UseLookupTableScalarRange: "
     << (this->UseLookupTableScalarRange ? "On\n" : "Off\n");
  os << indent << "LookupTable: " << this->LookupTable << "\n";
  os << indent << "Opacity: " << this->Opacity << "\n";
  os << indent << "Ambient: " << this->Ambient << "\n";
  os << indent << "Diffuse: " << this->Diffuse << "\n";
  os << indent << "InterpolationType: "
     << this->GetInterpolationTypeAsString() << "\n";
  os << indent << "Checkerboard: "
     << (this->Checkerboard ? "On\n" : "Off\n");
  os << indent << "CheckerboardSpacing: " << this->CheckerboardSpacing[0]
     << " " << this->CheckerboardSpacing[1] << "\n";
  os << indent << "CheckerboardOffset: " << this->CheckerboardOffset[0]
     << " " << this->CheckerboardOffset[1] << "\n";
}
