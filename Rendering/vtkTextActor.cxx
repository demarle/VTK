/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTextActor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkTextActor.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"
#include "vtkWindow.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkFreeTypeUtilities.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkTexture.h"
#include "vtkMath.h"
#include "vtkTexture.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkTextActor, "1.41");
vtkStandardNewMacro(vtkTextActor);
vtkCxxSetObjectMacro(vtkTextActor,Texture,vtkTexture);

// ----------------------------------------------------------------------------
vtkTextActor::vtkTextActor()
{
  // To remain compatible with code using vtkActor2D, we must set
  // position coord to Viewport, not Normalized Viewport
  // so...compute equivalent coords for initial position
  this->PositionCoordinate->SetCoordinateSystemToViewport();
    
  // This intializes the rectangle structure.
  // It will be used to display the text image as a texture map.
  this->Rectangle = vtkPolyData::New();
  this->RectanglePoints = vtkPoints::New();
  // The actual corner points of the rectangle will be computed later.
  this->Rectangle->SetPoints(this->RectanglePoints);
  vtkCellArray* polys = vtkCellArray::New();
  polys->InsertNextCell(4);
  polys->InsertCellPoint(0);
  polys->InsertCellPoint(1);
  polys->InsertCellPoint(2);
  polys->InsertCellPoint(3);
  this->Rectangle->SetPolys(polys);
  polys->Delete();
  vtkFloatArray* tc = vtkFloatArray::New();
  tc->SetNumberOfComponents(2);
  tc->SetNumberOfTuples(4);
  tc->InsertComponent(0,0, 0.0);  tc->InsertComponent(0,1, 0.0);
  tc->InsertComponent(1,0, 0.0);  tc->InsertComponent(1,1, 1.0);
  tc->InsertComponent(2,0, 1.0);  tc->InsertComponent(2,1, 1.0);
  tc->InsertComponent(3,0, 1.0);  tc->InsertComponent(3,1, 0.0);  
  this->Rectangle->GetPointData()->SetTCoords(tc);
  tc->Delete();  
  
  this->ImageData = vtkImageData::New();
  this->Texture = NULL;
  vtkTexture* texture = vtkTexture::New();
  texture->SetInput(this->ImageData);
  this->SetTexture(texture);
  texture->Delete();


  vtkPolyDataMapper2D *mapper = vtkPolyDataMapper2D::New();
  this->PDMapper = 0;
  this->SetMapper(mapper);
  mapper->Delete();
  // Done already in SetMapper.
  //this->PDMapper->SetInput(this->Rectangle);

  this->TextProperty = vtkTextProperty::New();
  this->Transform = vtkTransform::New();

  this->LastOrigin[0]     = 0;
  this->LastOrigin[1]     = 0;

  this->LastSize[0]       = 0;
  this->LastSize[1]       = 0;

  this->MinimumSize[0]    = 10;
  this->MinimumSize[1]    = 10;

  this->MaximumLineHeight = 1.0;
  this->ScaledText        = 0;
  this->Orientation       = 0.0;
  this->UseBorderAlign    = 0;

  this->FontScaleExponent = 1;
  this->FontScaleTarget   = 10;

  this->Input = 0;
  this->InputRendered = false;

  this->FormerOrientation = 0.0;

  this->FreeTypeUtilities = vtkFreeTypeUtilities::GetInstance();
  if (!this->FreeTypeUtilities)
    {
    vtkErrorMacro(<<"Failed getting the FreeType utilities instance");
    }
}

// ----------------------------------------------------------------------------
vtkTextActor::~vtkTextActor()
{
  this->ImageData->Delete();
  this->Transform->Delete();
  this->SetTextProperty(NULL);
  if(this->Input)
    {
    delete [] this->Input;
    }
  this->Rectangle->Delete();
  this->Rectangle = 0;
  this->RectanglePoints->Delete();
  this->RectanglePoints = 0;
  this->SetTexture(0);
}

// ----------------------------------------------------------------------------
void vtkTextActor::SetNonLinearFontScale(double exp, int tgt)
{
  if (this->FontScaleExponent == exp && this->FontScaleTarget == tgt)
    {
    return;
    }
  this->FontScaleExponent = exp;
  this->FontScaleTarget = tgt;
  this->Modified();
}

// ----------------------------------------------------------------------------
void vtkTextActor::SetMapper(vtkPolyDataMapper2D *mapper)
{
  // I will not reference count this because the superclass does.
  this->PDMapper = mapper; // So what is the point of have the ivar PDMapper?
  this->vtkActor2D::SetMapper( mapper );

  if (mapper)
    {
    mapper->SetInput(this->Rectangle);
    }
}

// ----------------------------------------------------------------------------
void vtkTextActor::SetMapper(vtkMapper2D *mapper)
{
  if (mapper && mapper->IsA("vtkPolyDataMapper2D"))
    {
    this->SetMapper( (vtkPolyDataMapper2D *)mapper );
    }
  else
    {
    vtkErrorMacro(<<"Must use a vtkPolyDataMapper2D with this class");
    }
}

// ----------------------------------------------------------------------------
void vtkTextActor::SetInput(const char* str)
{
  if(!str)
    {
      vtkErrorMacro(
        <<"vtkTextActor::SetInput was passed an uninitialized string");
    return;
    }
  if(this->Input)
    {
    if(strcmp(this->Input, str) == 0)
      {
      return;
      }
    delete [] this->Input;
    }
  this->Input = new char[strlen(str)+1];
  strcpy(this->Input, str);
  this->InputRendered = false;
  this->Modified();
}

// ----------------------------------------------------------------------------
char* vtkTextActor::GetInput()
{
  return this->Input;
}

// ----------------------------------------------------------------------------
void vtkTextActor::SetTextProperty(vtkTextProperty *p)
{
  if ( this->TextProperty == p )
    {
    return;
    }
  if ( this->TextProperty )
    {
    this->TextProperty->UnRegister( this );
    this->TextProperty = NULL; 
    }
  this->TextProperty = p;
  if (this->TextProperty)
    {
    this->TextProperty->Register(this);
    }
  this->Modified();
}

// ----------------------------------------------------------------------------
void vtkTextActor::ShallowCopy(vtkProp *prop)
{
  vtkTextActor *a = vtkTextActor::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->SetPosition2(a->GetPosition2());
    this->SetMinimumSize(a->GetMinimumSize());
    this->SetMaximumLineHeight(a->GetMaximumLineHeight());
    this->SetScaledText(a->GetScaledText());
    this->SetTextProperty(a->GetTextProperty());
    }
  // Now do superclass (mapper is handled by it as well).
  this->vtkActor2D::ShallowCopy(prop);
}

// ----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkTextActor::ReleaseGraphicsResources(vtkWindow *win)
{
  this->vtkActor2D::ReleaseGraphicsResources(win);
}

// ----------------------------------------------------------------------------
int vtkTextActor::RenderOverlay(vtkViewport *viewport)
{
  // render the texture 
  if (this->Texture && this->Input)
    {
    vtkRenderer* ren = vtkRenderer::SafeDownCast(viewport);
    if (ren)
      {
      this->Texture->Render(ren);
      }
    }
  
  // Everything is built in RenderOpaqueGeometry, just have to render
  return this->vtkActor2D::RenderOverlay(viewport);
}

// ----------------------------------------------------------------------------
int vtkTextActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  //Make sure we have a string to render
  if(!this->Input)
    {
    return 0;
    }

  int *vSize = viewport->GetSize();
  //vSize == (0,0) means that we're not ready to render yet
  if(vSize[0] == 0 && vSize[1] == 0)
    {
    return 0;
    }
  //not sure what vSize == 1 means, but it can cause divide-by-zero errors
  //in some of the coordinate conversion methods used below
  if(vSize[0] == 1 || vSize[1] == 1)
    {
    return 0;
    }
  
  //Scaled text case.  We need to be sure that our text will fit
  //inside the specified boundaries
  if(this->ScaledText)
    {
    int size[2], *point1, *point2;
    point1 = this->PositionCoordinate->GetComputedViewportValue(viewport);
    point2 = this->Position2Coordinate->GetComputedViewportValue(viewport);
    size[0] = point2[0] - point1[0];
    size[1] = point2[1] - point1[1];

    // Check to see whether we have to rebuild everything
    int positionsHaveChanged = 0;
    int orientationHasChanged = 0;
    if (viewport->GetMTime() > this->BuildTime ||
        (viewport->GetVTKWindow() &&
         viewport->GetVTKWindow()->GetMTime() > this->BuildTime))
      {
      // if the viewport has changed we may - or may not need
      // to rebuild, it depends on if the projected coords change
      if (this->LastSize[0]   != size[0]   || this->LastSize[1]   != size[1] ||
          this->LastOrigin[0] != point1[0] || this->LastOrigin[1] != point1[1])
        {
        positionsHaveChanged = 1;
        }
      }

    // If the orientation has changed then we'll probably need to change our
    // constrained font size as well
    if(this->FormerOrientation != this->Orientation)
      {
      this->Transform->Identity();
      this->Transform->RotateZ(this->Orientation);
      this->FormerOrientation = this->Orientation;
      orientationHasChanged = 1;
      }
    
    // Check to see whether we have to rebuild everything
    if (positionsHaveChanged || orientationHasChanged ||
        this->GetMTime() > this->BuildTime ||
        (this->Mapper && this->Mapper->GetMTime() > this->BuildTime) ||
        (this->TextProperty && this->TextProperty->GetMTime() > this->BuildTime))
      {
      vtkDebugMacro(<<"Rebuilding text");

      this->LastOrigin[0] = point1[0];
      this->LastOrigin[1] = point1[1];

      //  Lets try to minimize the number of times we change the font size.
      //  If the width of the font box has not changed by more than a pixel
      // (numerical issues) do not recompute font size.
      if ((this->Mapper && this->Mapper->GetMTime() > this->BuildTime) ||
          (this->TextProperty && this->TextProperty->GetMTime() > this->BuildTime) ||
          this->LastSize[0] < size[0] - 1 || this->LastSize[1] < size[1] - 1 ||
          this->LastSize[0] > size[0] + 1 || this->LastSize[1] > size[1] + 1 ||
          orientationHasChanged)
        {
        this->LastSize[0] = size[0];
        this->LastSize[1] = size[1];
      
        // limit by minimum size
        if (this->MinimumSize[0] > size[0])
          {
          size[0] = this->MinimumSize[0];
          }
        if (this->MinimumSize[1] > size[1])
          {
          size[1] = this->MinimumSize[1];
          }    
        int max_height = (int)(this->MaximumLineHeight * (float)size[1]);

        int fsize = this->FreeTypeUtilities->GetConstrainedFontSize(
          this->Input, this->TextProperty, this->Orientation, size[0],
          (size[1] < max_height ? size[1] : max_height));
          
        // apply non-linear scaling
        fsize =static_cast<int>(pow(static_cast<double>(fsize),this->FontScaleExponent)*
          pow(this->FontScaleTarget, 1.0 - this->FontScaleExponent));
        // and set the new font size
        this->TextProperty->SetFontSize(fsize);
        }
      }
    }
    
  //check if we need to render the string
  if(this->TextProperty->GetMTime() > this->BuildTime ||
    !this->InputRendered || this->GetMTime() > this->BuildTime)
    {
    if(!this->FreeTypeUtilities->RenderString(this->TextProperty,
                                              this->Input,
                                              this->ImageData))
      {
      vtkErrorMacro(<<"Failed rendering text to buffer");
      return 0;
      }

    // Check if we need to create a new rectangle.  
    // Need to check if angle has changed.
    //justification and line offset are handled in ComputeRectangle
    this->ComputeRectangle(viewport);

    this->ImageData->Modified();
    this->Texture->SetInput(this->ImageData);
    this->InputRendered = true;
    this->BuildTime.Modified();
    }    

  // Everything is built, just have to render
  return this->vtkActor2D::RenderOpaqueGeometry(viewport);
}

//-----------------------------------------------------------------------------
// Description:
// Does this prop have some translucent polygonal geometry?
int vtkTextActor::HasTranslucentPolygonalGeometry()
{
  return 0;
}

// ----------------------------------------------------------------------------
void vtkTextActor::SetOrientation(float orientation) 
{
  if (this->Orientation == orientation)
    {
    return;
    }
  this->Modified();
  this->Orientation = orientation;
}


// ----------------------------------------------------------------------------
int vtkTextActor::GetAlignmentPoint() 
{
  int alignmentCode = 0;
  
  if ( ! this->TextProperty)
    {
    return 0;
    }
  switch (this->TextProperty->GetJustification())
    {
    case VTK_TEXT_LEFT:
      alignmentCode = 0;
      break;
    case VTK_TEXT_CENTERED:
      alignmentCode = 1;
      break;
    case VTK_TEXT_RIGHT:
      alignmentCode = 2;
      break;
    default:
      vtkErrorMacro(<<"Unknown justifaction code.");
    }
   switch (this->TextProperty->GetVerticalJustification())
    {
    case VTK_TEXT_BOTTOM:
      alignmentCode += 0;
      break;
    case VTK_TEXT_CENTERED:
      alignmentCode += 3;
      break;
    case VTK_TEXT_TOP: 
      alignmentCode += 6;
      break;
    default:
      vtkErrorMacro(<<"Unknown justifaction code.");
    }
  return alignmentCode;
}     
    
// ----------------------------------------------------------------------------
void vtkTextActor::SetAlignmentPoint(int val) 
{
  vtkWarningMacro(<< "Alignment point is being depricated.  You should use "
                  << "SetJustification and SetVerticalJustification in the text property.");

  switch (val)
    {
    case 0:
      this->TextProperty->SetJustificationToLeft();
      this->TextProperty->SetVerticalJustificationToBottom();
      break;
    case 1:
      this->TextProperty->SetJustificationToCentered();
      this->TextProperty->SetVerticalJustificationToBottom();
      break;
    case 2:
      this->TextProperty->SetJustificationToRight();
      this->TextProperty->SetVerticalJustificationToBottom();
      break;
    case 3:
      this->TextProperty->SetJustificationToLeft();
      this->TextProperty->SetVerticalJustificationToCentered();
      break;
    case 4:
      this->TextProperty->SetJustificationToCentered();
      this->TextProperty->SetVerticalJustificationToCentered();
      break;
    case 5:
      this->TextProperty->SetJustificationToRight();
      this->TextProperty->SetVerticalJustificationToCentered();
      break;
    case 6:
      this->TextProperty->SetJustificationToLeft();
      this->TextProperty->SetVerticalJustificationToTop();
      break;
    case 7:
      this->TextProperty->SetJustificationToCentered();
      this->TextProperty->SetVerticalJustificationToTop();
      break;
    case 8:
      this->TextProperty->SetJustificationToRight();
      this->TextProperty->SetVerticalJustificationToTop();
      break;
    }
}
  
// ----------------------------------------------------------------------------
void vtkTextActor::ComputeRectangle(vtkViewport *viewport) 
{
  int dims[3];
  this->RectanglePoints->Reset();
  if (this->ImageData)
    {
    this->ImageData->GetDimensions(dims);
    }
  else
    {
    dims[0] = dims[1] = 0;
    }
    
  // I could do this with a transform, but it is simple enough
  // to rotate the four corners in 2D ...
  double radians = this->Orientation * vtkMath::DegreesToRadians();
  double c = cos(radians);      
  double s = sin(radians);
  double xo, yo;
  double x, y;
  double maxWidth, maxHeight;
  xo = yo = 0.0;
  maxWidth = maxHeight = 0;
  // In ScaledText mode we justify text based on the rectangle formed by
  // Position & Position2 coordinates
  if(this->ScaledText || this->UseBorderAlign)
    {
    double position1[3], position2[3];
    this->PositionCoordinate->GetValue(position1);
    this->Position2Coordinate->GetValue(position2);
    this->SpecifiedToDisplay(position1, viewport,
                             this->PositionCoordinate->GetCoordinateSystem());
    this->SpecifiedToDisplay(position2, viewport,
                             this->Position2Coordinate->GetCoordinateSystem());
    maxWidth = position2[0] - position1[0];
    maxHeight = position2[1] - position1[1];
    // I could get rid of "GetAlignmentPoint" and use justification directly.
    switch(this->GetAlignmentPoint())
      {
      case 0:
        break;
      case 1:
        xo = (double)(maxWidth - dims[0]) * 0.5;
        break;
      case 2:
        xo = (double)(maxWidth - dims[0]);
        break;
      case 3:
        yo = (double)(maxHeight - dims[1]) * 0.5;
        break;
      case 4:
        xo = (double)(maxWidth - dims[0]) * 0.5;
        yo = (double)(maxHeight - dims[1]) * 0.5;
        break;
      case 5:
        xo = (double)(maxWidth - dims[0]);
        yo = (double)(maxHeight - dims[1]) * 0.5;
        break;
      case 6:
        yo = (double)(maxHeight - dims[1]);
        break;
      case 7:
        xo = (double)(maxWidth - dims[0]) * 0.5;
        yo = (double)(maxHeight - dims[1]);
        break;
      case 8:
        xo = (double)(maxWidth - dims[0]);
        yo = (double)(maxHeight - dims[1]);
        break;
      default:
        vtkErrorMacro(<< "Bad alignment point value.");
      }
    //handle line offset.  make sure we stay within the bounds defined by
    //position1 & position2
    double offset = this->TextProperty->GetLineOffset();
    if( (yo + offset + dims[1]) > maxHeight)
      {
      yo = maxHeight - dims[1];
      }
    else if( (yo + offset ) < 0)
      {
      yo = 0;
      } 
    else
      {
      yo += offset;
      }
    }
  else
    {
    // I could get rid of "GetAlignmentPoint" and use justification directly.
    switch (this->GetAlignmentPoint())
      {
      case 0:
        break;
      case 1:
        xo = -(double)(dims[0]) * 0.5;
        break;
      case 2:
        xo = -(double)(dims[0]);
        break;
      case 3:
        yo = -(double)(dims[1]) * 0.5;
        break;
      case 4:
        yo = -(double)(dims[1]) * 0.5;
        xo = -(double)(dims[0]) * 0.5;
        break;
      case 5:
        yo = -(double)(dims[1]) * 0.5;
        xo = -(double)(dims[0]);
        break;
      case 6:
        yo = -(double)(dims[1]);
        break;
      case 7:
        yo = -(double)(dims[1]);
        xo = -(double)(dims[0]) * 0.5;
        break;
      case 8:
        yo = -(double)(dims[1]);
        xo = -(double)(dims[0]);
        break;
      default:
        vtkErrorMacro(<< "Bad alignment point value.");
      }
    // handle line offset
    yo += this->TextProperty->GetLineOffset();
    } //end unscaled text case  

  x = xo; y = yo;      
  this->RectanglePoints->InsertNextPoint(c*x-s*y,s*x+c*y,0.0);
  x = xo; y = yo + (double)(dims[1]);      
  this->RectanglePoints->InsertNextPoint(c*x-s*y,s*x+c*y,0.0);
  x = xo + (double)(dims[0]); y = yo + (double)(dims[1]);      
  this->RectanglePoints->InsertNextPoint(c*x-s*y,s*x+c*y,0.0);
  x = xo + (double)(dims[0]); y = yo;
  this->RectanglePoints->InsertNextPoint(c*x-s*y,s*x+c*y,0.0);
}


// ----------------------------------------------------------------------------
void vtkTextActor::SpecifiedToDisplay(double *pos, vtkViewport *vport,
                                      int specified) 
{
  switch(specified)
  {    
  case VTK_WORLD:
    vport->WorldToView(pos[0], pos[1], pos[2]);
  case VTK_VIEW:
    vport->ViewToNormalizedViewport(pos[0], pos[1], pos[2]);
  case VTK_NORMALIZED_VIEWPORT:
    vport->NormalizedViewportToViewport(pos[0], pos[1]);
  case VTK_VIEWPORT:
    vport->ViewportToNormalizedDisplay(pos[0], pos[1]);
  case VTK_NORMALIZED_DISPLAY:
    vport->NormalizedDisplayToDisplay(pos[0], pos[1]);
  case VTK_DISPLAY:
    break;
  }
}

// ----------------------------------------------------------------------------
void vtkTextActor::DisplayToSpecified(double *pos, vtkViewport *vport,
                                      int specified)
{
  switch(specified)
    {
    case VTK_WORLD:
      vport->DisplayToNormalizedDisplay(pos[0], pos[1]);
      vport->NormalizedDisplayToViewport(pos[0], pos[1]);
      vport->ViewportToNormalizedViewport(pos[0], pos[1]);
      vport->NormalizedViewportToView(pos[0], pos[1], pos[2]);
      vport->ViewToWorld(pos[0], pos[1], pos[2]);
      break;
    case VTK_VIEW:
      vport->DisplayToNormalizedDisplay(pos[0], pos[1]);
      vport->NormalizedDisplayToViewport(pos[0], pos[1]);
      vport->ViewportToNormalizedViewport(pos[0], pos[1]);
      vport->NormalizedViewportToView(pos[0], pos[1], pos[2]);
      break;
    case VTK_NORMALIZED_VIEWPORT:
      vport->DisplayToNormalizedDisplay(pos[0], pos[1]);
      vport->NormalizedDisplayToViewport(pos[0], pos[1]);
      vport->ViewportToNormalizedViewport(pos[0], pos[1]);
      break;
    case VTK_VIEWPORT:
      vport->DisplayToNormalizedDisplay(pos[0], pos[1]);
      vport->NormalizedDisplayToViewport(pos[0], pos[1]);
      break;
    case VTK_NORMALIZED_DISPLAY:
      vport->DisplayToNormalizedDisplay(pos[0], pos[1]);
      break;
    case VTK_DISPLAY:
      break;
    }
}


// ----------------------------------------------------------------------------
void vtkTextActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->TextProperty)
    {
    os << indent << "Text Property:\n";
    this->TextProperty->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Text Property: (none)\n";
    }

  os << indent << "MaximumLineHeight: " << this->MaximumLineHeight << endl;
  os << indent << "MinimumSize: " << this->MinimumSize[0] << " " << this->MinimumSize[1] << endl;
  os << indent << "ScaledText: " << this->ScaledText << endl;
  os << indent << "Orientation: " << this->Orientation << endl;
  os << indent << "FontScaleExponent: " << this->FontScaleExponent << endl;
  os << indent << "FontScaleTarget: " << this->FontScaleTarget << endl;
  os << indent << "Texture: " << this->Texture << "\n";
  os << indent << "UseBorderAlign: " << this->UseBorderAlign << "\n";
  if (this->Texture)
    {
    this->Texture->PrintSelf(os, indent.GetNextIndent());
    }
}
