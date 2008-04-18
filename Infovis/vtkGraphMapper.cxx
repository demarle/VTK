/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGraphMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
#include "vtkGraphMapper.h"

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDirectedGraph.h"
#include "vtkExecutive.h"
#include "vtkFollower.h"
#include "vtkGarbageCollector.h"
#include "vtkGlyph3D.h"
#include "vtkGraphToPolyData.h"
#include "vtkIconGlyphFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneSource.h"
#include "vtkPNGReader.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkTexture.h"
#include "vtkTransformCoordinateSystems.h"
#include "vtkUndirectedGraph.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkViewTheme.h"

vtkCxxRevisionMacro(vtkGraphMapper, "1.13");
vtkStandardNewMacro(vtkGraphMapper);

#define VTK_CREATE(type,name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
vtkGraphMapper::vtkGraphMapper()
{
  this->GraphToPoly       = vtkSmartPointer<vtkGraphToPolyData>::New();
  this->VertexGlyph       = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->ScaledGlyph       = vtkSmartPointer<vtkGlyph3D>::New();
  this->OutlineGlyph      = vtkSmartPointer<vtkGlyph3D>::New();
  this->IconGlyph         = vtkSmartPointer<vtkIconGlyphFilter>::New();
  this->IconTransform     = vtkSmartPointer<vtkTransformCoordinateSystems>::New();
  this->EdgeMapper        = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->VertexMapper      = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->OutlineMapper     = vtkSmartPointer<vtkPolyDataMapper>::New(); 
  this->IconMapper        = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->EdgeActor         = vtkSmartPointer<vtkActor>::New();
  this->VertexActor       = vtkSmartPointer<vtkActor>::New();
  this->OutlineActor      = vtkSmartPointer<vtkActor>::New();
  this->IconActor         = vtkSmartPointer<vtkActor2D>::New();
  this->VertexLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  this->EdgeLookupTable   = vtkSmartPointer<vtkLookupTable>::New();
  this->VertexColorArrayNameInternal = 0;
  this->EdgeColorArrayNameInternal = 0;
  this->VertexPointSize = 5;
  this->EdgeLineWidth = 1;
  this->VertexGlyphs = true;
  this->GlyphScaling = false;
  this->ScalingArrayName = 0;
  this->SetScalingArrayName("leaf_count");
  
  this->VertexMapper->SetScalarModeToUsePointData();
  this->VertexMapper->SetLookupTable(this->VertexLookupTable);
  this->VertexMapper->SetScalarVisibility(false);
  this->VertexActor->PickableOff();
  this->VertexActor->GetProperty()->SetPointSize(this->GetVertexPointSize());
  this->OutlineActor->PickableOff();
  this->OutlineActor->GetProperty()->SetPointSize(this->GetVertexPointSize()+2);
  this->OutlineActor->SetPosition(0, 0, -0.001);
  this->OutlineMapper->SetScalarVisibility(false);
  this->EdgeMapper->SetScalarModeToUseCellData();
  this->EdgeMapper->SetLookupTable(this->EdgeLookupTable);
  this->EdgeMapper->SetScalarVisibility(false);
  this->EdgeActor->SetPosition(0, 0, -0.003);
  this->EdgeActor->GetProperty()->SetLineWidth(this->GetEdgeLineWidth());
  
  this->IconTransform->SetInputCoordinateSystemToWorld();
  this->IconTransform->SetOutputCoordinateSystemToDisplay();
  this->IconTransform->SetInputConnection(this->GraphToPoly->GetOutputPort());

  this->IconGlyph->SetInputConnection(this->IconTransform->GetOutputPort());
  this->IconGlyph->SetUseIconSize(true);
  this->IconMapper->SetInputConnection(this->IconGlyph->GetOutputPort());
  this->IconMapper->ScalarVisibilityOff();

  this->IconActor->SetMapper(this->IconMapper);
  this->IconArrayNameInternal = 0;

  this->VertexGlyph->SetInputConnection(this->GraphToPoly->GetOutputPort());
  this->VertexMapper->SetInputConnection(this->VertexGlyph->GetOutputPort());
  this->OutlineMapper->SetInputConnection(this->VertexGlyph->GetOutputPort());
  
  this->VertexActor->SetMapper(this->VertexMapper);
  this->OutlineActor->SetMapper(this->OutlineMapper);
  this->EdgeMapper->SetInputConnection(this->GraphToPoly->GetOutputPort());
  this->EdgeActor->SetMapper(this->EdgeMapper);
  
  // Set default parameters
  this->SetVertexColorArrayName("VertexDegree");
  this->ColorVerticesOff();
  this->SetEdgeColorArrayName("weight");
  this->ColorEdgesOff();
  this->IconVisibilityOff();
}

//----------------------------------------------------------------------------
vtkGraphMapper::~vtkGraphMapper()
{
  // Delete internally created objects.
  // Note: All of the smartpointer objects 
  //       will be deleted for us
    
  this->SetVertexColorArrayNameInternal(0);
  this->SetEdgeColorArrayNameInternal(0);
  this->SetIconArrayNameInternal(0);
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetIconArrayName(const char* name)
{
  this->SetIconArrayNameInternal(name);
  this->IconGlyph->SetInputArrayToProcess(0,0,0,
          vtkDataObject::FIELD_ASSOCIATION_POINTS,name);
}

//----------------------------------------------------------------------------
const char* vtkGraphMapper::GetIconArrayName()
{
  return this->GetIconArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetVertexGlyphs(bool arg)
{
  if (!arg)
    {
    vtkPolyData *circle = this->CreateCircle(true);
    this->ScaledGlyph->SetSource(circle);
    circle->Delete();
    this->ScaledGlyph->SetInputConnection(this->GraphToPoly->GetOutputPort());
    this->ScaledGlyph->SetScaling(1);
    this->ScaledGlyph->SetInputArrayToProcess(0,0,0,
             vtkDataObject::FIELD_ASSOCIATION_POINTS, this->ScalingArrayName);
    this->VertexMapper->SetInputConnection(this->ScaledGlyph->GetOutputPort());
    
    vtkPolyData *outline = this->CreateCircle(false);
    this->OutlineGlyph->SetSource(outline);
    outline->Delete();
    this->OutlineGlyph->SetInputConnection(this->GraphToPoly->GetOutputPort());
    this->OutlineGlyph->SetScaling(1);
    this->OutlineGlyph->SetInputArrayToProcess(0,0,0,
             vtkDataObject::FIELD_ASSOCIATION_POINTS, this->ScalingArrayName);
    this->OutlineMapper->SetInputConnection(this->OutlineGlyph->GetOutputPort());
    this->OutlineActor->SetPosition(0, 0, 0.001);
    this->OutlineActor->GetProperty()->SetLineWidth(2);
    }
  else
    {
    this->VertexGlyph->SetInputConnection(this->GraphToPoly->GetOutputPort());
    this->VertexMapper->SetInputConnection(this->VertexGlyph->GetOutputPort());
    this->OutlineActor->SetPosition(0, 0, -0.001);
    this->OutlineMapper->SetInputConnection(this->VertexGlyph->GetOutputPort());
    }
}


//----------------------------------------------------------------------------
// Helper method
vtkPolyData* vtkGraphMapper::CreateCircle(bool filled)
{
  int circleRes = 16;
  vtkIdType ptIds[17];
  double x[3], theta;
  
  // Allocate storage
  vtkPolyData *poly = vtkPolyData::New();
  VTK_CREATE(vtkPoints,pts);
  VTK_CREATE(vtkCellArray, circle);
  VTK_CREATE(vtkCellArray, outline);

  // generate points around the circle
  x[2] = 0.0;
  theta = 2.0 * vtkMath::Pi() / circleRes;
  for (int i=0; i<circleRes; i++)
    {
    x[0] = 0.5 * cos((double)i*theta);
    x[1] = 0.5 * sin((double)i*theta);
    ptIds[i] = pts->InsertNextPoint(x);
    }
  circle->InsertNextCell(circleRes,ptIds);
  
  // Outline
  ptIds[circleRes] = ptIds[0];
  outline->InsertNextCell(circleRes+1,ptIds);
  
  // Set up polydata
  poly->SetPoints(pts);
  if (filled)
    {
    poly->SetPolys(circle);
    }
  else
    {
    poly->SetLines(outline); 
    }
  
  return poly;
}


//----------------------------------------------------------------------------
void vtkGraphMapper::SetVertexColorArrayName(const char* name)
{
  this->SetVertexColorArrayNameInternal(name);
  this->VertexMapper->SetScalarModeToUsePointFieldData();
  this->VertexMapper->SelectColorArray(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphMapper::GetVertexColorArrayName()
{
  return this->GetVertexColorArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetColorVertices(bool vis)
{
  this->VertexMapper->SetScalarVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphMapper::GetColorVertices()
{
  return this->VertexMapper->GetScalarVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkGraphMapper::ColorVerticesOn()
{
  this->VertexMapper->SetScalarVisibility(true);
}

//----------------------------------------------------------------------------
void vtkGraphMapper::ColorVerticesOff()
{
  this->VertexMapper->SetScalarVisibility(false);
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetIconVisibility(bool vis)
{
  this->IconActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphMapper::GetIconVisibility()
{
  return this->IconActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetEdgeColorArrayName(const char* name)
{
  this->SetEdgeColorArrayNameInternal(name);
  this->EdgeMapper->SetScalarModeToUseCellFieldData();
  this->EdgeMapper->SelectColorArray(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphMapper::GetEdgeColorArrayName()
{
  return this->GetEdgeColorArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetColorEdges(bool vis)
{
  this->EdgeMapper->SetScalarVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphMapper::GetColorEdges()
{
  return this->EdgeMapper->GetScalarVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkGraphMapper::ColorEdgesOn()
{
  this->EdgeMapper->SetScalarVisibility(true);
}

//----------------------------------------------------------------------------
void vtkGraphMapper::ColorEdgesOff()
{
  this->EdgeMapper->SetScalarVisibility(false);
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetVertexPointSize(float size)
{
  this->VertexPointSize = size;
  this->VertexActor->GetProperty()->SetPointSize(this->GetVertexPointSize());
  this->OutlineActor->GetProperty()->SetPointSize(this->GetVertexPointSize()+2);
}
  
//----------------------------------------------------------------------------
void vtkGraphMapper::SetEdgeLineWidth(float width)
{
  this->EdgeLineWidth = width;
  this->EdgeActor->GetProperty()->SetLineWidth(this->GetEdgeLineWidth());
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetEdgeVisibility(bool vis)
{
  this->EdgeActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphMapper::GetEdgeVisibility()
{
  return this->EdgeActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetIconSize(int *size)
{
  this->IconGlyph->SetIconSize(size);
}

//----------------------------------------------------------------------------
int *vtkGraphMapper::GetIconSize()
{
  return this->IconGlyph->GetIconSize();
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetIconTexture(vtkTexture *texture)
{
  this->IconActor->SetTexture(texture);
}

//----------------------------------------------------------------------------
vtkTexture *vtkGraphMapper::GetIconTexture()
{ 
  return this->IconActor->GetTexture();
}

//----------------------------------------------------------------------------
void vtkGraphMapper::SetInput(vtkGraph *input)
{
  if(input)
    {
    this->SetInputConnection(0, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0, 0);
    }
}

//----------------------------------------------------------------------------
vtkGraph *vtkGraphMapper::GetInput()
{
  vtkGraph *inputGraph =
  vtkGraph::SafeDownCast(this->Superclass::GetInputAsDataSet());
  return inputGraph;
}

//----------------------------------------------------------------------------
void vtkGraphMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  if (this->EdgeMapper)
    {
    this->EdgeMapper->ReleaseGraphicsResources( renWin );
    }
}

//----------------------------------------------------------------------------
// Receives from Actor -> maps data to primitives
//
void vtkGraphMapper::Render(vtkRenderer *ren, vtkActor * vtkNotUsed(act))
{
  // make sure that we've been properly initialized
  if ( !this->GetExecutive()->GetInputData(0, 0) )
    {
    vtkErrorMacro(<< "No input!\n");
    return;
    } 
    
  // Update the pipeline up until the graph to poly data
  vtkGraph *input = vtkGraph::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
  if (!input)
    {
    vtkErrorMacro(<< "Input is not a graph!\n");
    return;
    }
  vtkGraph *graph = 0;
  if (vtkDirectedGraph::SafeDownCast(input))
    {
    graph = vtkDirectedGraph::New();
    }
  else
    {
    graph = vtkUndirectedGraph::New();
    }
  graph->ShallowCopy(input);

  this->GraphToPoly->SetInput(graph);
  graph->Delete();
  this->GraphToPoly->Update();
  vtkPolyData* pd = this->GraphToPoly->GetOutput();
 
  // Try to find the range the user-specified color array.
  // If we cannot find that array, use the scalar range.
  double range[2];
  vtkDataArray* arr = 0; 
  if (this->GetColorEdges())
    {
    if (this->GetEdgeColorArrayName())
      {
      arr = pd->GetCellData()->GetArray(this->GetEdgeColorArrayName());
      }
    if (!arr)
      {
      arr = pd->GetCellData()->GetScalars();
      }
    if (arr)
      {
      arr->GetRange(range);    
      this->EdgeMapper->SetScalarRange(range[0], range[1]);
      }
    }

  // Do the same thing for the vertex array.
  arr = 0; 
  if (this->GetColorVertices())
    {
    if (this->GetVertexColorArrayName())
      {
      arr = pd->GetPointData()->GetArray(this->GetVertexColorArrayName());
      }
    if (!arr)
      {
      arr = pd->GetPointData()->GetScalars();
      }
    if (arr)
      {
      arr->GetRange(range);    
      this->VertexMapper->SetScalarRange(range[0], range[1]);
      }
    }

  if (this->IconActor->GetTexture() && 
      this->IconActor->GetTexture()->GetInput() && 
      this->IconActor->GetVisibility())
    {
    this->IconTransform->SetViewport(ren);
    this->IconActor->GetTexture()->MapColorScalarsThroughLookupTableOff();
    this->IconActor->GetTexture()->GetInput()->Update();
    int *dim = this->IconActor->GetTexture()->GetInput()->GetDimensions();
    this->IconGlyph->SetIconSheetSize(dim);

    }
  if (this->EdgeActor->GetVisibility())
    {
    this->EdgeActor->RenderOpaqueGeometry(ren);
    }
  this->VertexActor->RenderOpaqueGeometry(ren);
  this->OutlineActor->RenderOpaqueGeometry(ren);
  if (this->IconActor->GetVisibility())
    {
    this->IconActor->RenderOpaqueGeometry(ren);
    }
  
  if (this->EdgeActor->GetVisibility())
    {
    this->EdgeActor->RenderTranslucentPolygonalGeometry(ren);
    }
  this->VertexActor->RenderTranslucentPolygonalGeometry(ren);
  this->OutlineActor->RenderTranslucentPolygonalGeometry(ren);
  if (this->IconActor->GetVisibility())
    {
    this->IconActor->RenderTranslucentPolygonalGeometry(ren);
    }
  if(this->IconActor->GetVisibility())
    {
    this->IconActor->RenderOverlay(ren);
    }
  this->TimeToDraw = this->EdgeMapper->GetTimeToDraw() +
                     this->VertexMapper->GetTimeToDraw() +
                     this->OutlineMapper->GetTimeToDraw() +
                     this->IconMapper->GetTimeToDraw();
}

void vtkGraphMapper::ApplyViewTheme(vtkViewTheme* theme)
{
  this->VertexActor->GetProperty()->SetColor(theme->GetPointColor());
  this->VertexActor->GetProperty()->SetOpacity(theme->GetPointOpacity());
  this->OutlineActor->GetProperty()->SetColor(theme->GetOutlineColor());
  this->VertexLookupTable->SetHueRange(theme->GetPointHueRange()); 
  this->VertexLookupTable->SetSaturationRange(theme->GetPointSaturationRange()); 
  this->VertexLookupTable->SetValueRange(theme->GetPointValueRange()); 
  this->VertexLookupTable->SetAlphaRange(theme->GetPointAlphaRange()); 
  this->VertexLookupTable->Build();

  this->EdgeActor->GetProperty()->SetColor(theme->GetCellColor());
  this->EdgeActor->GetProperty()->SetOpacity(theme->GetCellOpacity());
  this->EdgeLookupTable->SetHueRange(theme->GetCellHueRange()); 
  this->EdgeLookupTable->SetSaturationRange(theme->GetCellSaturationRange()); 
  this->EdgeLookupTable->SetValueRange(theme->GetCellValueRange()); 
  this->EdgeLookupTable->SetAlphaRange(theme->GetCellAlphaRange()); 
  this->EdgeLookupTable->Build();
}

//----------------------------------------------------------------------------
void vtkGraphMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->EdgeMapper )
    {
    os << indent << "EdgeMapper: (" << this->EdgeMapper << ")\n";
    }
  else
    {
    os << indent << "EdgeMapper: (none)\n";
    }
 if ( this->VertexMapper )
    {
    os << indent << "VertexMapper: (" << this->VertexMapper << ")\n";
    }
  else
    {
    os << indent << "VertexMapper: (none)\n";
    }
 if ( this->OutlineMapper )
    {
    os << indent << "OutlineMapper: (" << this->OutlineMapper << ")\n";
    }
  else
    {
    os << indent << "OutlineMapper: (none)\n";
    }
  if ( this->EdgeActor )
    {
    os << indent << "EdgeActor: (" << this->EdgeActor << ")\n";
    }
  else
    {
    os << indent << "EdgeActor: (none)\n";
    }
 if ( this->VertexActor )
    {
    os << indent << "VertexActor: (" << this->VertexActor << ")\n";
    }
  else
    {
    os << indent << "VertexActor: (none)\n";
    }
 if ( this->OutlineActor )
    {
    os << indent << "OutlineActor: (" << this->OutlineActor << ")\n";
    }
  else
    {
    os << indent << "OutlineActor: (none)\n";
    }

  if ( this->GraphToPoly )
    {
    os << indent << "GraphToPoly: (" << this->GraphToPoly << ")\n";
    }
  else
    {
    os << indent << "GraphToPoly: (none)\n";
    }
  os << indent << "VertexPointSize: " << this->VertexPointSize << "\n";
  os << indent << "EdgeLineWidth: " << this->EdgeLineWidth << "\n";
}

//----------------------------------------------------------------------------
unsigned long vtkGraphMapper::GetMTime()
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

//----------------------------------------------------------------------------
int vtkGraphMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
  return 1;
}

//----------------------------------------------------------------------------
double *vtkGraphMapper::GetBounds()
{
  static double bounds[] = {-1.0,1.0, -1.0,1.0, -1.0,1.0};

  vtkGraph *graph = vtkGraph::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
  if (!graph) 
    {
    return bounds;
    }
  else
    {
    if (!this->Static)
      {
      this->Update();
      }
    graph->GetBounds(this->Bounds);
    return this->Bounds;
    }
}

#if 1
//----------------------------------------------------------------------------
void vtkGraphMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  //vtkGarbageCollectorReport(collector, this->GraphToPoly,
  //                          "GraphToPoly");
}

#endif
