/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCursor3D.cxx
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
#include "vtkCursor3D.h"
#include "vtkObjectFactory.h"

#include <math.h>

vtkCxxRevisionMacro(vtkCursor3D, "1.37");
vtkStandardNewMacro(vtkCursor3D);

// Construct with model bounds = (-1,1,-1,1,-1,1), focal point = (0,0,0),
// all parts of cursor visible, and wrapping off.
vtkCursor3D::vtkCursor3D()
{
  vtkPoints *pts;

  this->Focus = vtkPolyData::New();
  pts = vtkPoints::New();
  pts->Allocate(1);
  pts->vtkPoints::InsertPoint(0, 0.0, 0.0, 0.0);
  this->Focus->SetPoints(pts);
  pts->Delete();

  this->ModelBounds[0] = -1.0;
  this->ModelBounds[1] = 1.0;
  this->ModelBounds[2] = -1.0;
  this->ModelBounds[3] = 1.0;
  this->ModelBounds[4] = -1.0;
  this->ModelBounds[5] = 1.0;

  this->FocalPoint[0] = 0.0;  
  this->FocalPoint[1] = 0.0;  
  this->FocalPoint[2] = 0.0;

  this->Outline = 1;
  this->Axes = 1;
  this->XShadows = 1;
  this->YShadows = 1;
  this->ZShadows = 1;
  this->Wrap = 0;
}

vtkCursor3D::~vtkCursor3D()
{
  this->Focus->Delete();
}

void vtkCursor3D::Execute()
{
  int i;
  int numPts=0, numLines=0;
  vtkPoints *newPts;
  vtkCellArray *newLines;
  float x[3];
  vtkIdType ptIds[2];
  vtkPolyData *output = this->GetOutput();
  
  vtkDebugMacro(<<"Generating cursor");

  // Check bounding box and origin
  //
  if ( this->Wrap ) 
    {
    for (i=0; i<3; i++)
      {
      this->FocalPoint[i] = this->ModelBounds[2*i] + 
             fmod((double)(this->FocalPoint[i]-this->ModelBounds[2*i]), 
                  (double)(this->ModelBounds[2*i+1]-this->ModelBounds[2*i]));
      }
    } 
  else 
    {
    for (i=0; i<3; i++)
      {
      if ( this->FocalPoint[i] < this->ModelBounds[2*i] )
        {
        this->FocalPoint[i] = this->ModelBounds[2*i];
        }
      if ( this->FocalPoint[i] > this->ModelBounds[2*i+1] )
        {
        this->FocalPoint[i] = this->ModelBounds[2*i+1];
        }
      }
    }

  // Allocate storage
  //
  if (this->Axes) 
    {
    numPts += 6;
    numLines += 3;
    }

  if (this->Outline) 
    {
    numPts += 8;
    numLines += 12;
    }

  if (this->XShadows) 
    {
    numPts += 8;
    numLines += 4;
    }

  if (this->YShadows) 
    {
    numPts += 8;
    numLines += 4;
    }

  if (this->ZShadows) 
    {
    numPts += 8;
    numLines += 4;
    }

  if ( numPts ) 
    {
    newPts = vtkPoints::New();
    newPts->Allocate(numPts);
    newLines = vtkCellArray::New();
    newLines->Allocate(newLines->EstimateSize(numLines,2));
    }
  else
    {
    return;
    }

  // Create axes
  //
  if ( this->Axes ) 
    {
    x[0] = this->ModelBounds[0]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->FocalPoint[2];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->FocalPoint[2];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->FocalPoint[2];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->FocalPoint[2];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->ModelBounds[4]; 
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);
    }

  // Create outline
  //
  if ( this->Outline ) 
    {
    // First triad
    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[4];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    // Second triad
    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[5];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    // Fill in remaining lines
    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[4];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);


    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);
    }

  // Create x-shadows
  //
  if ( this->XShadows ) 
    {
    for (i=0; i<2; i++) 
      {
      x[0] = this->ModelBounds[i]; 
      x[1] = this->ModelBounds[2]; 
      x[2] = this->FocalPoint[2];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[i]; 
      x[1] = this->ModelBounds[3]; 
      x[2] = this->FocalPoint[2];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);

      x[0] = this->ModelBounds[i]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[4];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[i]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[5];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);
      }
    }

  //  Create y-shadows
  //
  if ( this->YShadows ) 
    {
    for (i=0; i<2; i++) 
      {
      x[0] = this->ModelBounds[0]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->FocalPoint[2];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[1]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->FocalPoint[2];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->ModelBounds[4];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->ModelBounds[5];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);
      }
    }

  //  Create z-shadows
  //
  if ( this->ZShadows ) 
    {
    for (i=0; i<2; i++) 
      {
      x[0] = this->ModelBounds[0]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[i+4]; 
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[1]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[i+4]; 
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[2]; 
      x[2] = this->ModelBounds[i+4]; 
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[3]; 
      x[2] = this->ModelBounds[i+4];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);
      }
    }

  // Update ourselves and release memory
  //
  this->Focus->GetPoints()->SetPoint(0,this->FocalPoint);

  output->SetPoints(newPts);
  newPts->Delete();

  output->SetLines(newLines);
  newLines->Delete();
}

// Set the boundary of the 3D cursor.
void vtkCursor3D::SetModelBounds(float xmin, float xmax, float ymin, float ymax,
                                float zmin, float zmax)
{
  if ( xmin != this->ModelBounds[0] || xmax != this->ModelBounds[1] ||
  ymin != this->ModelBounds[2] || ymax != this->ModelBounds[3] ||
  zmin != this->ModelBounds[4] || zmax != this->ModelBounds[5] )
    {
    this->Modified();

    this->ModelBounds[0] = xmin; this->ModelBounds[1] = xmax; 
    this->ModelBounds[2] = ymin; this->ModelBounds[3] = ymax; 
    this->ModelBounds[4] = zmin; this->ModelBounds[5] = zmax; 

    for (int i=0; i<3; i++)
      {
      if ( this->ModelBounds[2*i] > this->ModelBounds[2*i+1] )
        {
        this->ModelBounds[2*i] = this->ModelBounds[2*i+1];
        }
      }
    }
}

void vtkCursor3D::SetModelBounds(float bounds[6])
{
  this->SetModelBounds(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4],
                       bounds[5]);
}

// Turn every part of the 3D cursor on.
void vtkCursor3D::AllOn()
{
  this->OutlineOn();
  this->AxesOn();
  this->XShadowsOn();
  this->YShadowsOn();
  this->ZShadowsOn();
}

// Turn every part of the 3D cursor off.
void vtkCursor3D::AllOff()
{
  this->OutlineOff();
  this->AxesOff();
  this->XShadowsOff();
  this->YShadowsOff();
  this->ZShadowsOff();
}

void vtkCursor3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ModelBounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->ModelBounds[0] << ", " << this->ModelBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->ModelBounds[2] << ", " << this->ModelBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->ModelBounds[4] << ", " << this->ModelBounds[5] << ")\n";

  os << indent << "Focal Point: (" << this->FocalPoint[0] << ", "
               << this->FocalPoint[1] << ", "
               << this->FocalPoint[2] << ")\n";

  os << indent << "Outline: " << (this->Outline ? "On\n" : "Off\n");
  os << indent << "Axes: " << (this->Axes ? "On\n" : "Off\n");
  os << indent << "XShadows: " << (this->XShadows ? "On\n" : "Off\n");
  os << indent << "YShadows: " << (this->YShadows ? "On\n" : "Off\n");
  os << indent << "ZShadows: " << (this->ZShadows ? "On\n" : "Off\n");
  os << indent << "Wrap: " << (this->Wrap ? "On\n" : "Off\n");
}
