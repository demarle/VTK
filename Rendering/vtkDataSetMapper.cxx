/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataSetMapper.cxx
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
#include "vtkDataSetMapper.h"

#include "vtkPolyDataMapper.h"
#include "vtkObjectFactory.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkDataSet.h"

vtkCxxRevisionMacro(vtkDataSetMapper, "1.63");
vtkStandardNewMacro(vtkDataSetMapper);

vtkDataSetMapper::vtkDataSetMapper()
{
  this->GeometryExtractor = NULL;
  this->PolyDataMapper = NULL;
}

vtkDataSetMapper::~vtkDataSetMapper()
{
  // delete internally created objects.
  if ( this->GeometryExtractor )
    {
    this->GeometryExtractor->Delete();
    }
  if ( this->PolyDataMapper )
    {
    this->PolyDataMapper->Delete();
    }
}

void vtkDataSetMapper::SetInput(vtkDataSet *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

vtkDataSet *vtkDataSetMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Inputs[0]);
}

void vtkDataSetMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  if (this->PolyDataMapper)
    {
    this->PolyDataMapper->ReleaseGraphicsResources( renWin );
    }
}

// Receives from Actor -> maps data to primitives
//
void vtkDataSetMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  // make sure that we've been properly initialized
  //
  if ( !this->GetInput() )
    {
    vtkErrorMacro(<< "No input!\n");
    return;
    } 

  // Need a lookup table
  //
  if ( this->LookupTable == NULL )
    {
    this->CreateDefaultLookupTable();
    }
  this->LookupTable->Build();

  // Now can create appropriate mapper
  //
  if ( this->PolyDataMapper == NULL ) 
    {
    vtkDataSetSurfaceFilter *gf = vtkDataSetSurfaceFilter::New();
    vtkPolyDataMapper *pm = vtkPolyDataMapper::New();
    pm->SetInput(gf->GetOutput());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    }

  // share clipping planes with the PolyDataMapper
  //
  if (this->ClippingPlanes != this->PolyDataMapper->GetClippingPlanes()) 
    {
    this->PolyDataMapper->SetClippingPlanes(this->ClippingPlanes);
    }

  // For efficiency: if input type is vtkPolyData, there's no need to 
  // pass it thru the geometry filter.
  //
  if ( this->GetInput()->GetDataObjectType() == VTK_POLY_DATA )
    {
    this->PolyDataMapper->SetInput((vtkPolyData *)(this->GetInput()));
    }
  else
    {
    this->GeometryExtractor->SetInput(this->GetInput());
    this->PolyDataMapper->SetInput(this->GeometryExtractor->GetOutput());
    }
  
  // update ourselves in case something has changed
  this->PolyDataMapper->SetLookupTable(this->GetLookupTable());
  this->PolyDataMapper->SetScalarVisibility(this->GetScalarVisibility());
  this->PolyDataMapper->SetUseLookupTableScalarRange(
    this->GetUseLookupTableScalarRange());
  this->PolyDataMapper->SetScalarRange(this->GetScalarRange());
  this->PolyDataMapper->SetImmediateModeRendering(
    this->GetImmediateModeRendering());
  this->PolyDataMapper->SetColorMode(this->GetColorMode());
  this->PolyDataMapper->SetScalarMode(this->GetScalarMode());
  if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
       this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
    {
    if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
      {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayId,ArrayComponent);
      }
    else
      {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayName,ArrayComponent);
      }
    }
  
  this->PolyDataMapper->Render(ren,act);
  this->TimeToDraw = this->PolyDataMapper->GetTimeToDraw();
}

void vtkDataSetMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->PolyDataMapper )
    {
    os << indent << "Poly Mapper: (" << this->PolyDataMapper << ")\n";
    }
  else
    {
    os << indent << "Poly Mapper: (none)\n";
    }

  if ( this->GeometryExtractor )
    {
    os << indent << "Geometry Extractor: (" << this->GeometryExtractor << ")\n";
    }
  else
    {
    os << indent << "Geometry Extractor: (none)\n";
    }
}

unsigned long vtkDataSetMapper::GetMTime()
{
  unsigned long mTime=this->vtkMapper::GetMTime();
  unsigned long time;

  if ( this->LookupTable != NULL )
    {
    time = this->LookupTable->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}
