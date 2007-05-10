/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTextRepresentation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkTextRepresentation.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkFreeTypeUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkCommand.h"

class vtkTextRepresentationObserver : public vtkCommand
{
public:
  static vtkTextRepresentationObserver* New()
    { return new vtkTextRepresentationObserver; }
  
  void SetTarget(vtkTextRepresentation* t)
    {
    this->Target = t;
    }
  virtual void Execute(vtkObject* o, unsigned long event, void *p)
    {
    if (this->Target)
      {
        if(o && vtkTextActor::SafeDownCast(o))
          {
          this->Target->ExecuteTextActorModifiedEvent(o, event, p);
          }
        else if( o && vtkTextProperty::SafeDownCast(o))
          {
          this->Target->ExecuteTextPropertyModifiedEvent(o, event, p);
          }
      }
    }
protected:
  vtkTextRepresentationObserver() { this->Target = 0; }
  vtkTextRepresentation* Target;
    
};

vtkCxxRevisionMacro(vtkTextRepresentation, "1.7");
vtkStandardNewMacro(vtkTextRepresentation);

//-------------------------------------------------------------------------
vtkTextRepresentation::vtkTextRepresentation()
{
  this->Observer = vtkTextRepresentationObserver::New();
  this->Observer->SetTarget(this);

  this->TextActor = vtkTextActor::New();
  this->InitializeTextActor();
 
  this->ShowBorder = vtkBorderRepresentation::BORDER_ACTIVE;
  this->BWActor->VisibilityOff();
}

//-------------------------------------------------------------------------
vtkTextRepresentation::~vtkTextRepresentation()
{
  this->TextActor->GetTextProperty()->RemoveObserver(this->Observer);
  this->TextActor->RemoveObserver(this->Observer);
  this->Observer->SetTarget(0);
  this->Observer->Delete();

  this->SetTextActor(0);
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::SetTextActor(vtkTextActor *textActor)
{
  if ( textActor != this->TextActor )
    {
    if ( this->TextActor )
      {
      this->TextActor->GetTextProperty()->RemoveObserver(this->Observer);
      this->TextActor->RemoveObserver(this->Observer);
      this->TextActor->Delete();
      }
    this->TextActor = textActor;
    if(this->TextActor)
      {
      this->TextActor->Register(this);
      }

    this->InitializeTextActor();
    this->Modified();
    }
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::SetText(const char* text)
{
  if (this->TextActor)
    {
    this->TextActor->SetInput(text);
    }
  else
    {
    vtkErrorMacro("No Text Actor present. Cannot set text.");
    }
}

//-------------------------------------------------------------------------
const char* vtkTextRepresentation::GetText()
{
  if (this->TextActor)
    {
    return this->TextActor->GetInput();
    }
  vtkErrorMacro("No text actor present. No showing any text.");
  return 0;
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::BuildRepresentation()
{
  // Ask the superclass the size and set the text
  int *pos1 = this->PositionCoordinate->GetComputedDisplayValue(this->Renderer);
  int *pos2 = this->Position2Coordinate->GetComputedDisplayValue(this->Renderer);

  if ( this->TextActor )
    {
    this->TextActor->GetPositionCoordinate()->SetValue(pos1[0],pos1[1]);
    this->TextActor->GetPosition2Coordinate()->SetValue(pos2[0],pos2[1]);
    }

  // Note that the transform is updated by the superclass
  this->Superclass::BuildRepresentation();
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::GetActors2D(vtkPropCollection *pc)
{
  pc->AddItem(this->TextActor);
  this->Superclass::GetActors2D(pc);
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  this->TextActor->ReleaseGraphicsResources(w);
  this->Superclass::ReleaseGraphicsResources(w);
}

//-------------------------------------------------------------------------
int vtkTextRepresentation::RenderOverlay(vtkViewport *w)
{
  int count = this->Superclass::RenderOverlay(w);
  count += this->TextActor->RenderOverlay(w);
  return count;
}

//-------------------------------------------------------------------------
int vtkTextRepresentation::RenderOpaqueGeometry(vtkViewport *w)
{
  int count = this->Superclass::RenderOpaqueGeometry(w);
  count += this->TextActor->RenderOpaqueGeometry(w);
  return count;
}

//-------------------------------------------------------------------------
int vtkTextRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *w)
{
  int count = this->Superclass::RenderTranslucentPolygonalGeometry(w);
  count += this->TextActor->RenderTranslucentPolygonalGeometry(w);
  return count;
}

//-------------------------------------------------------------------------
int vtkTextRepresentation::HasTranslucentPolygonalGeometry()
{
  int result = this->Superclass::HasTranslucentPolygonalGeometry();
  result |= this->TextActor->HasTranslucentPolygonalGeometry();
  return result;
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::InitializeTextActor()
{
  if ( this->TextActor )
    {
    this->TextActor->ScaledTextOn();
    this->TextActor->SetMinimumSize(1,1);
    this->TextActor->SetMaximumLineHeight(1.0);
    this->TextActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    this->TextActor->GetPosition2Coordinate()->SetCoordinateSystemToDisplay();
    this->TextActor->GetPosition2Coordinate()->SetReferenceCoordinate(0);
    this->TextActor->GetTextProperty()->SetJustificationToCentered();
    this->TextActor->GetTextProperty()->SetVerticalJustificationToCentered();
    
    this->TextActor->UseBorderAlignOn();

    this->TextProperty = this->TextActor->GetTextProperty();

    this->TextActor->GetTextProperty()->AddObserver(
      vtkCommand::ModifiedEvent, this->Observer);
    this->TextActor->AddObserver(
      vtkCommand::ModifiedEvent, this->Observer);
    }
}  

//----------------------------------------------------------------------------
void vtkTextRepresentation::ExecuteTextPropertyModifiedEvent(vtkObject* object, 
  unsigned long enumEvent, void*)
{
  if(!object || enumEvent != vtkCommand::ModifiedEvent)
    {
    return;
    }
  vtkTextProperty* tp = vtkTextProperty::SafeDownCast(object);
  if(!tp)
    {
    return;
    }

  this->CheckTextBoundary();
}

//----------------------------------------------------------------------------
void vtkTextRepresentation::ExecuteTextActorModifiedEvent(vtkObject* object, 
  unsigned long enumEvent, void*)
{
  if(!object || enumEvent != vtkCommand::ModifiedEvent)
    {
    return;
    }
  vtkTextActor* ta = vtkTextActor::SafeDownCast(object);
  if(!ta || ta != this->TextActor)
    {
    return;
    }

  if(this->TextProperty != this->TextActor->GetTextProperty())
    {
    this->TextActor->GetTextProperty()->AddObserver(
      vtkCommand::ModifiedEvent, this->Observer);
    this->TextProperty = this->TextActor->GetTextProperty();
    }

  this->CheckTextBoundary();
}

//----------------------------------------------------------------------------
void vtkTextRepresentation::CheckTextBoundary()
{
  if(!this->TextActor->GetScaledText())
  {
    vtkFreeTypeUtilities* ftu = vtkFreeTypeUtilities::GetInstance();
    if (!ftu)
      {
      vtkErrorMacro(<<"Failed getting the FreeType utilities instance");
      return;
      }

    int text_bbox[4];
    ftu->GetBoundingBox(this->TextActor->GetTextProperty(), 
      this->GetText(), text_bbox);
    if (!ftu->IsBoundingBoxValid(text_bbox))
      {
      return;
      }

    // The bounding box was the area that is going to be filled with pixels
    // given a text origin of (0, 0). Now get the real size we need, i.e.
    // the full extent from the origin to the bounding box.

    double text_size[2];
    text_size[0] = (text_bbox[1] - text_bbox[0] + 1);
    text_size[1] = (text_bbox[3] - text_bbox[2] + 1);

    this->GetRenderer()->DisplayToNormalizedDisplay (text_size[0], text_size[1]);
    this->GetRenderer()->NormalizedDisplayToViewport (text_size[0], text_size[1]);
    this->GetRenderer()->ViewportToNormalizedViewport (text_size[0], text_size[1]);

   // update the PositionCoordinate
    
    double* pos2 = this->Position2Coordinate->GetValue();
    if(pos2[0] != text_size[0] || pos2[1] != text_size[1])
      {
      this->Position2Coordinate->SetValue(text_size[0], text_size[1], 0);
      this->Modified();
      }
    }
}

//-------------------------------------------------------------------------
void vtkTextRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Text Actor: " << this->TextActor << "\n";
  
}
