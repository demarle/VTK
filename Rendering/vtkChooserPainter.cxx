/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkChooserPainter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkChooserPainter.h"

#include "vtkCommand.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkInstantiator.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"
#include "vtkStandardPolyDataPainter.h"

vtkCxxRevisionMacro(vtkChooserPainter, "1.1.2.1");
vtkStandardNewMacro(vtkChooserPainter);

vtkCxxSetObjectMacro(vtkChooserPainter, VertPainter, vtkPolyDataPainter);
vtkCxxSetObjectMacro(vtkChooserPainter, LinePainter, vtkPolyDataPainter);
vtkCxxSetObjectMacro(vtkChooserPainter, PolyPainter, vtkPolyDataPainter);
vtkCxxSetObjectMacro(vtkChooserPainter, StripPainter, vtkPolyDataPainter);

//-----------------------------------------------------------------------------
vtkChooserPainter::vtkChooserPainter()
{
  this->VertPainter = NULL;
  this->LinePainter = NULL;
  this->PolyPainter = NULL;
  this->StripPainter = NULL;

  this->LastRenderer = NULL;
}

//-----------------------------------------------------------------------------
vtkChooserPainter::~vtkChooserPainter()
{

  if (this->VertPainter) this->VertPainter->Delete();
  if (this->LinePainter) this->LinePainter->Delete();
  if (this->PolyPainter) this->PolyPainter->Delete();
  if (this->StripPainter) this->StripPainter->Delete();
}

/*
//-----------------------------------------------------------------------------
void vtkChooserPainter::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->VertPainter)
    {
    this->VertPainter->ReleaseGraphicsResources(w);
    }
  if (this->LinePainter)
    {
    this->LinePainter->ReleaseGraphicsResources(w);
    }
  if (this->PolyPainter)
    {
    this->PolyPainter->ReleaseGraphicsResources(w);
    }
  if (this->StripPainter)
    {
    this->StripPainter->ReleaseGraphicsResources(w);
    }
  this->Superclass::ReleaseGraphicsResources(w);
}
*/
//-----------------------------------------------------------------------------
void vtkChooserPainter::ReportReferences(vtkGarbageCollector *collector)
{
  this->Superclass::ReportReferences(collector);

  vtkGarbageCollectorReport(collector, this->VertPainter, "Vert Painter");
  vtkGarbageCollectorReport(collector, this->LinePainter, "Line Painter");
  vtkGarbageCollectorReport(collector, this->PolyPainter, "Poly Painter");
  vtkGarbageCollectorReport(collector, this->StripPainter, "Strip Painter");
}

//-----------------------------------------------------------------------------
void vtkChooserPainter::PrepareForRendering(vtkRenderer* ren, vtkActor* actor)
{
  // Ensure that the renderer chain is up-to-date.
  if (this->PaintersChoiceTime < this->MTime ||
    this->PaintersChoiceTime < this->Information->GetMTime() || 
    this->LastRenderer != ren || 
    this->PaintersChoiceTime < this->PolyData->GetMTime())
    {
    this->LastRenderer = ren;
    // Choose the painters.
    this->ChoosePainters(ren);
    // Pass them the information and poly data we have.
    this->UpdateChoosenPainters();
    this->PaintersChoiceTime.Modified();
    }
  this->Superclass::PrepareForRendering(ren, actor);
}

//-----------------------------------------------------------------------------
void vtkChooserPainter::UpdateChoosenPainters()
{
  if (this->VertPainter)
    {
    this->PassInformation(this->VertPainter);
    }
  if (this->LinePainter)
    {
    this->PassInformation(this->LinePainter);
    }
  if (this->PolyPainter)
    {
    this->PassInformation(this->PolyPainter);
    }
  if (this->StripPainter)
    {
    this->PassInformation(this->StripPainter);
    }
}

//-----------------------------------------------------------------------------
void vtkChooserPainter::ChoosePainters(vtkRenderer *renderer)
{
  const char *vertpaintertype;
  const char *linepaintertype;
  const char *polypaintertype;
  const char *strippaintertype;

  vtkPolyDataPainter* painter;
  
  this->SelectPainters(renderer, vertpaintertype, linepaintertype,
                       polypaintertype, strippaintertype);
  vtkDebugMacro(<< "Selected " << vertpaintertype << ", "
                << linepaintertype << ", " << polypaintertype << ", "
                << strippaintertype);

  if (!this->VertPainter || !this->VertPainter->IsA(vertpaintertype))
    {
    painter = this->CreatePainter(vertpaintertype);
    this->SetVertPainter(painter);
    painter->Delete();
    vtkStandardPolyDataPainter* sp = vtkStandardPolyDataPainter::New();
    painter->SetDelegatePainter(sp);
    sp->Delete();
    }

  if (!this->LinePainter || !this->LinePainter->IsA(linepaintertype))
    {
    if (strcmp(vertpaintertype, linepaintertype) == 0)
      {
      this->SetLinePainter(this->VertPainter);
      }
    else
      {
      painter = this->CreatePainter(linepaintertype);
      this->SetLinePainter(painter);
      painter->Delete();
      vtkStandardPolyDataPainter* sp = vtkStandardPolyDataPainter::New();
      painter->SetDelegatePainter(sp);
      sp->Delete();
      }
    }

  if (!this->PolyPainter || !this->PolyPainter->IsA(polypaintertype))
    {
    if (strcmp(vertpaintertype, polypaintertype) == 0)
      {
      this->SetPolyPainter(this->VertPainter);
      }
    else if (strcmp(linepaintertype, polypaintertype) == 0)
      {
      this->SetPolyPainter(this->LinePainter);
      }
    else
      {
      painter = this->CreatePainter(polypaintertype);
      this->SetPolyPainter(painter);
      painter->Delete();
      vtkStandardPolyDataPainter* sp = vtkStandardPolyDataPainter::New();
      painter->SetDelegatePainter(sp);
      sp->Delete();
      }
    }

  if (!this->StripPainter || !this->StripPainter->IsA(strippaintertype))
    {
    if (strcmp(vertpaintertype, strippaintertype) == 0)
      {
      this->SetStripPainter(this->VertPainter);
      }
    else if (strcmp(linepaintertype, strippaintertype) == 0)
      {
      this->SetStripPainter(this->LinePainter);
      }
    else if (strcmp(polypaintertype, strippaintertype) == 0)
      {
      this->SetStripPainter(this->PolyPainter);
      }
    else
      {
      painter = this->CreatePainter(strippaintertype);
      this->SetStripPainter(painter);
      painter->Delete();
      vtkStandardPolyDataPainter* sp = vtkStandardPolyDataPainter::New();
      painter->SetDelegatePainter(sp);
      sp->Delete();
      }
    }
}

//-----------------------------------------------------------------------------
void vtkChooserPainter::SelectPainters(vtkRenderer *vtkNotUsed(renderer),
                                       const char *&vertptype,
                                       const char *&lineptype,
                                       const char *&polyptype,
                                       const char *&stripptype)
{
  vertptype = "vtkPointsPainter";
  lineptype = "vtkLinesPainter";
  polyptype = "vtkPolygonsPainter";
  stripptype = "vtkTStripsPainter";
  // No elaborate selection as yet. 
  // Merely create the pipeline as the vtkOpenGLPolyDataMapper.
}

//-----------------------------------------------------------------------------
vtkPolyDataPainter* vtkChooserPainter::CreatePainter(const char *paintertype)
{
  vtkObject* o = vtkInstantiator::CreateInstance(paintertype);
  vtkPolyDataPainter* p = vtkPolyDataPainter::SafeDownCast(o);
  if (!p && o)
    {
    o->Delete();
    }
  this->ObserverPainterProgress(p);
  return p;
}

//-----------------------------------------------------------------------------
void vtkChooserPainter::RenderInternal(vtkRenderer* renderer, vtkActor* actor, 
    unsigned long typeflags)
{
  vtkIdType numVerts = this->PolyData->GetNumberOfVerts();
  vtkIdType numLines = this->PolyData->GetNumberOfLines();
  vtkIdType numPolys = this->PolyData->GetNumberOfPolys();
  vtkIdType numStrips = this->PolyData->GetNumberOfStrips();
  
  vtkIdType total_cells = (typeflags & vtkPainter::VERTS)? 
    this->PolyData->GetNumberOfVerts() : 0;
  total_cells += (typeflags & vtkPainter::LINES)? 
    this->PolyData->GetNumberOfLines() : 0;
  total_cells += (typeflags & vtkPainter::POLYS)? 
    this->PolyData->GetNumberOfPolys() : 0;
  total_cells += (typeflags & vtkPainter::STRIPS)? 
    this->PolyData->GetNumberOfStrips() : 0;
  
  if (total_cells == 0)
    {
    // nothing to render.
    return;
    }

  this->ProgressOffset = 0.0;
  this->TimeToDraw = 0.0;
  if (typeflags & vtkPainter::VERTS)
    {
    //cout << this << "Verts" << endl;
    this->ProgressScaleFactor = static_cast<double>(numVerts)/total_cells;
    this->VertPainter->Render(renderer, actor, vtkPainter::VERTS);
    this->TimeToDraw += this->VertPainter->GetTimeToDraw();
    this->ProgressOffset += this->ProgressScaleFactor;
    }
  
  if (typeflags & vtkPainter::LINES)
    {
    //cout << this << "Lines" << endl;
    this->ProgressScaleFactor = static_cast<double>(numLines)/total_cells;
    this->LinePainter->Render(renderer, actor, vtkPainter::LINES);
    this->TimeToDraw += this->LinePainter->GetTimeToDraw();
    this->ProgressOffset += this->ProgressScaleFactor;
    }
  
  
  if (typeflags & vtkPainter::POLYS)
    {
    //cout << this << "Polys" << endl;
    this->ProgressScaleFactor = static_cast<double>(numPolys)/total_cells;
    this->PolyPainter->Render(renderer, actor, vtkPainter::POLYS);
    this->TimeToDraw += this->PolyPainter->GetTimeToDraw();
    this->ProgressOffset += this->ProgressScaleFactor;
    }

  if (typeflags & vtkPainter::STRIPS)
    {
    //cout << this << "Strips" << endl;
    this->ProgressScaleFactor = static_cast<double>(numStrips)/total_cells;
    this->StripPainter->Render(renderer, actor, vtkPainter::STRIPS);
    this->TimeToDraw += this->StripPainter->GetTimeToDraw();
    }
  this->Superclass::RenderInternal(renderer, actor, typeflags);
}

//-----------------------------------------------------------------------------
void vtkChooserPainter::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "VertPainter: " << this->VertPainter << endl;
  os << indent << "LinePainter: " << this->LinePainter << endl;
  os << indent << "PolyPainter: " << this->PolyPainter << endl;
  os << indent << "StripPainter: " << this->StripPainter << endl;
}

