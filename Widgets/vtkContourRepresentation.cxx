/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkContourRepresentation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkContourRepresentation.h"
#include "vtkHandleRepresentation.h"
#include "vtkCoordinate.h"
#include "vtkRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkBox.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkPointPlacer.h"
#include "vtkContourLineInterpolator.h"
#include "vtkLine.h"
#include "vtkCamera.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkMassProperties.h"
#include "vtkTriangleFilter.h"

#include <vtkstd/vector>
#include <vtkstd/set>
#include <vtkstd/algorithm>
#include <vtkstd/iterator>

vtkCxxRevisionMacro(vtkContourRepresentation, "1.3");
vtkCxxSetObjectMacro(vtkContourRepresentation, PointPlacer, vtkPointPlacer);
vtkCxxSetObjectMacro(vtkContourRepresentation, LineInterpolator, vtkContourLineInterpolator);

class vtkContourRepresentationPoint
{
public:
  double        WorldPosition[3];
};


class vtkContourRepresentationNode
{
public:
  double        WorldPosition[3];
  double        WorldOrientation[9];
  vtkstd::vector<vtkContourRepresentationPoint*> Points;
};

class vtkContourRepresentationInternals
{
public:
  vtkstd::vector<vtkContourRepresentationNode*> Nodes;
};

// This is a helper class. It computes statistics on the contour.
// Typical statistics may be area, perimeter, min-max values 
// within the contour etc..
//
// This class is used internally by the contour representation. If the user
// sets vtkContourRepresentation::ComputeContourStatisticsOn(), the contour
// statistics will be calculated at the end of every interaction, provided 
// it is closed.
//
class vtkContourStatistics
{
public:
  vtkContourStatistics() 
    {
    this->MassProperties = vtkMassProperties::New();
    this->TriangleFilter = vtkTriangleFilter::New();
    this->PolyData = vtkPolyData::New();
    this->TriangleFilter->SetInput( this->PolyData );
    this->MassProperties->SetInput( this->TriangleFilter->GetOutput() );
    this->Perimeter = 0.0;
    this->LastBuildTime = 0;
    this->LastPerimeterComputedTime = 0;
    }
   
  ~vtkContourStatistics()
    {
    this->MassProperties = NULL;
    this->TriangleFilter = NULL;
    this->PolyData       = NULL;
    }

  // Description: 
  // Set/Get methods for the Polydata
  vtkPolyData * GetPolyData() { return this->PolyData ; }
  
  void SetPolyData(vtkPolyData *pd)
    {
    if( !pd || (pd == this->PolyData) ) 
      {
      return;
      }
    this->PolyData = NULL;
    this->PolyData = pd;
    }

  // Description: 
  // Compute the perimeter of the contour. It is assumed that the polydata has 
  // been set.
  double ComputePerimeter() 
    {
    double perimeter=0.0;
    vtkPoints *points   = this->PolyData->GetPoints();
    vtkCellArray *lines = this->PolyData->GetPolys();
    const vtkIdType ncells = this->PolyData->GetNumberOfCells(); 
                        // = 1 for the one and only contour
    vtkIdType npts;
    vtkIdType *pts;
    lines->InitTraversal();
    for ( int i=0; i< ncells; i++)
      {
      lines->GetNextCell(npts,pts); 
      
      // Get the points in each line segment of the cell 
      // (polyline in the case of a contour)
      
      double p1[3], p2[3], p3[3];
      this->PolyData->GetPoint( pts[0], p3 );
      for ( int j=1; j < npts; j++ )
        {
        this->PolyData->GetPoint( pts[j-1], p1 );
        this->PolyData->GetPoint( pts[j], p2 );
        perimeter += sqrt((p2[0]-p1[0])*(p2[0]-p1[0]) 
                        + (p2[1]-p1[1])*(p2[1]-p1[1])
                        + (p2[2]-p1[2])*(p2[2]-p1[2]));
        }
      perimeter += sqrt((p2[0]-p3[0])*(p2[0]-p3[0]) 
                      + (p2[1]-p3[1])*(p2[1]-p3[1])
                      + (p2[2]-p3[2])*(p2[2]-p3[2]));
      
      }
    this->Perimeter = perimeter;
    this->LastPerimeterComputedTime = this->LastBuildTime;
    }

  // Description: 
  // Get the perimeter of the contour. It is assumed that ComputePerimeter
  // has already been called.
  double GetPerimeter() 
    {
    if( this->LastBuildTime > this->LastPerimeterComputedTime )
      {
      this->ComputePerimeter();
      }
    return this->Perimeter;
    }
  
  
  // Description: 
  // Get the area of the contour. It is assumed that the polydata has been set.
  double GetArea()
    {
    return this->MassProperties->GetSurfaceArea();
    }
    
  // Description: 
  // Get the NSI of the contour. It is assumed that the polydata has been set.
  double GetNormalizedShapeIndex()
    {
    return this->MassProperties->GetNormalizedShapeIndex();
    }
  
  // Description: 
  // Store the build times of the last assigned polydata, to avoid 
  // recomputation.
  void SetLastBuildTime( unsigned long t ) { this->LastBuildTime = t; }
  unsigned long GetLastBuildTime() const { return this->LastBuildTime; }
  
 private:
  vtkMassProperties *MassProperties;
  vtkTriangleFilter *TriangleFilter;
  vtkPolyData       *PolyData;
  double            Perimeter;
  unsigned long     LastBuildTime;
  unsigned long     LastPerimeterComputedTime;
};


//----------------------------------------------------------------------
vtkContourRepresentation::vtkContourRepresentation()
{
  this->Internal = new vtkContourRepresentationInternals;
  
  this->PixelTolerance           = 7;
  this->WorldTolerance           = 0.001;
  this->PointPlacer              = NULL;
  this->LineInterpolator         = NULL;
  this->ActiveNode               = -1;
  this->NeedToRender             = 0;
  this->ClosedLoop               = 0;
  this->CurrentOperation         = vtkContourRepresentation::Inactive;
  this->ContourStatistics        = NULL;
  this->ComputeStatisticsOff();
}

//----------------------------------------------------------------------
vtkContourRepresentation::~vtkContourRepresentation()
{
  this->SetPointPlacer(NULL);
  this->SetLineInterpolator(NULL);
  
  for(unsigned int i=0;i<this->Internal->Nodes.size();i++)
    {
    for (unsigned int j=0;j<this->Internal->Nodes[i]->Points.size();j++)
      {
      delete this->Internal->Nodes[i]->Points[j];
      }
    this->Internal->Nodes[i]->Points.clear();
    delete this->Internal->Nodes[i];
    }
  this->Internal->Nodes.clear(); 
  delete this->Internal;

  if ( this->ContourStatistics ) 
    {
    delete this->ContourStatistics;
    }
}

  
//----------------------------------------------------------------------
void vtkContourRepresentation::AddNodeAtWorldPositionInternal( double worldPos[3],
                                                               double worldOrient[9] )
{
  // Add a new point at this position
  vtkContourRepresentationNode *node = new vtkContourRepresentationNode;
  node->WorldPosition[0] = worldPos[0];
  node->WorldPosition[1] = worldPos[1];
  node->WorldPosition[2] = worldPos[2];
  
  memcpy(node->WorldOrientation, worldOrient, 9*sizeof(double) );
  
  
  this->Internal->Nodes.push_back(node);
  
  this->UpdateLines( this->Internal->Nodes.size()-1);
  this->NeedToRender = 1;
}
//----------------------------------------------------------------------
int vtkContourRepresentation::AddNodeAtWorldPosition( double worldPos[3],
                                                      double worldOrient[9] )
{
  // Check if this is a valid location
  if ( !this->PointPlacer->ValidateWorldPosition( worldPos, worldOrient ) )
    {
    return 0;
    }

  this->AddNodeAtWorldPositionInternal( worldPos, worldOrient );
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::AddNodeAtWorldPosition( double worldPos[3] )
{
  // Check if this is a valid location
  if ( !this->PointPlacer->ValidateWorldPosition( worldPos ) )
    {
    return 0;
    }

  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};
  
  this->AddNodeAtWorldPositionInternal( worldPos, worldOrient );
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::AddNodeAtDisplayPosition(double displayPos[2])
{
  double worldPos[3];
  double worldOrient[9];
  
  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient) )
    {
    return 0;
    }
  
  this->AddNodeAtWorldPositionInternal( worldPos, worldOrient );  
  return 1;
}
//----------------------------------------------------------------------
int vtkContourRepresentation::AddNodeAtDisplayPosition(int displayPos[2])
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];
  return this->AddNodeAtDisplayPosition( doubleDisplayPos );
  
}
//----------------------------------------------------------------------
int vtkContourRepresentation::AddNodeAtDisplayPosition(int X, int Y)
{
  double displayPos[2];
  displayPos[0] = X;
  displayPos[1] = Y;
  return this->AddNodeAtDisplayPosition( displayPos );
  
}

//----------------------------------------------------------------------
int vtkContourRepresentation::ActivateNode( double displayPos[2] )
{
  // Find closest node to this display pos that
  // is within PixelTolerance
  
  int closestNode = -1;
  double closestDistance2 = VTK_FLOAT_MAX;
  
  unsigned int i;
  double limit = this->PixelTolerance * this->PixelTolerance;
  
  for ( i = 0; i < this->Internal->Nodes.size(); i++ )
    {
    double currDisplayPos[2];
    this->GetNthNodeDisplayPosition( i, currDisplayPos );
    
    double currDistance2 =
      (currDisplayPos[0] - displayPos[0])*
      (currDisplayPos[0] - displayPos[0]) +
      (currDisplayPos[1] - displayPos[1])*
      (currDisplayPos[1] - displayPos[1]);
    
    if ( currDistance2 < limit &&
         currDistance2 < closestDistance2 )
      {
      closestNode = i;
      closestDistance2 = currDistance2;
      }
    }

  if ( closestNode != this->ActiveNode )
    {
    this->ActiveNode = closestNode;
    this->NeedToRender = 1;
    }

  return ( this->ActiveNode >= 0 );
}
//----------------------------------------------------------------------
int vtkContourRepresentation::ActivateNode( int displayPos[2] )
{
  double doubleDisplayPos[2];
  
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];
  return this->ActivateNode( doubleDisplayPos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::ActivateNode( int X, int Y )
{
  double doubleDisplayPos[2];
  
  doubleDisplayPos[0] = X;
  doubleDisplayPos[1] = Y;
  return this->ActivateNode( doubleDisplayPos );
}


//----------------------------------------------------------------------
int vtkContourRepresentation::SetActiveNodeToWorldPosition( double worldPos[3],
                                                            double worldOrient[9] )
{
  if ( this->ActiveNode < 0 ||
       static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  // Check if this is a valid location
  if ( !this->PointPlacer->ValidateWorldPosition( worldPos, worldOrient ) )
    {
    return 0;
    }
  
  this->SetNthNodeWorldPositionInternal( this->ActiveNode, 
                                         worldPos, 
                                         worldOrient );
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetActiveNodeToWorldPosition( double worldPos[3] )
{
  if ( this->ActiveNode < 0 ||
       static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  // Check if this is a valid location
  if ( !this->PointPlacer->ValidateWorldPosition( worldPos ) )
    {
    return 0;
    }
  
  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};
  
  this->SetNthNodeWorldPositionInternal( this->ActiveNode, 
                                         worldPos,
                                         worldOrient );
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetActiveNodeToDisplayPosition( double displayPos[2] )
{
  if ( this->ActiveNode < 0 ||
       static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  double worldPos[3];
  double worldOrient[9];
  
  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient ) )
    {
    return 0;
    }
  
  this->SetNthNodeWorldPositionInternal( this->ActiveNode, 
                                         worldPos,
                                         worldOrient );
  return 1;
}
//----------------------------------------------------------------------
int vtkContourRepresentation::SetActiveNodeToDisplayPosition( int displayPos[2] )
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];
  return this->SetActiveNodeToDisplayPosition( doubleDisplayPos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetActiveNodeToDisplayPosition( int X, int Y )
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = X;
  doubleDisplayPos[1] = Y;
  return this->SetActiveNodeToDisplayPosition( doubleDisplayPos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetActiveNodeWorldPosition( double pos[3] )
{
  return this->GetNthNodeWorldPosition( this->ActiveNode, pos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetActiveNodeWorldOrientation( double orient[9] )
{
  return this->GetNthNodeWorldOrientation( this->ActiveNode, orient );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetActiveNodeDisplayPosition( double pos[2] )
{
  return this->GetNthNodeDisplayPosition( this->ActiveNode, pos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetNumberOfNodes()
{
  return this->Internal->Nodes.size();
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetNumberOfIntermediatePoints(int n)
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  return this->Internal->Nodes[n]->Points.size();
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetIntermediatePointWorldPosition(int n, 
                                                                int idx, 
                                                                double point[3])
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  if ( idx < 0 ||
       static_cast<unsigned int>(idx) >= this->Internal->Nodes[n]->Points.size() )
    {
    return 0;
    }
  
  point[0] = this->Internal->Nodes[n]->Points[idx]->WorldPosition[0];
  point[1] = this->Internal->Nodes[n]->Points[idx]->WorldPosition[1];
  point[2] = this->Internal->Nodes[n]->Points[idx]->WorldPosition[2];
  
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetNthNodeDisplayPosition( int n, double displayPos[2] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  double pos[4];
  pos[0] = this->Internal->Nodes[n]->WorldPosition[0];
  pos[1] = this->Internal->Nodes[n]->WorldPosition[1];
  pos[2] = this->Internal->Nodes[n]->WorldPosition[2];
  pos[3] = 1.0;
  
  this->Renderer->SetWorldPoint( pos );
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint( pos );
  
  displayPos[0] = pos[0];
  displayPos[1] = pos[1];
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetNthNodeWorldPosition( int n, double worldPos[3] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  worldPos[0] = this->Internal->Nodes[n]->WorldPosition[0];
  worldPos[1] = this->Internal->Nodes[n]->WorldPosition[1];
  worldPos[2] = this->Internal->Nodes[n]->WorldPosition[2];
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetNthNodeWorldOrientation( int n, double worldOrient[9] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
 
  memcpy( worldOrient, this->Internal->Nodes[n]->WorldOrientation, 9*sizeof(double) );
  return 1;
}

//----------------------------------------------------------------------
void vtkContourRepresentation::SetNthNodeWorldPositionInternal( int n, double worldPos[3],
                                                                double worldOrient[9] )
{
  this->Internal->Nodes[n]->WorldPosition[0] = worldPos[0];
  this->Internal->Nodes[n]->WorldPosition[1] = worldPos[1];
  this->Internal->Nodes[n]->WorldPosition[2] = worldPos[2];
  
  memcpy(this->Internal->Nodes[n]->WorldOrientation, worldOrient, 9*sizeof(double) );

  this->UpdateLines( n );
  this->NeedToRender = 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetNthNodeWorldPosition( int n, double worldPos[3],
                                                       double worldOrient[9] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  // Check if this is a valid location
  if ( !this->PointPlacer->ValidateWorldPosition( worldPos, worldOrient ) )
    {
    return 0;
    }

  this->SetNthNodeWorldPositionInternal( n, worldPos, worldOrient );
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetNthNodeWorldPosition( int n, double worldPos[3] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  // Check if this is a valid location
  if ( !this->PointPlacer->ValidateWorldPosition( worldPos ) )
    {
    return 0;
    }
  
  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};

  this->SetNthNodeWorldPositionInternal( n, worldPos, worldOrient );
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetNthNodeDisplayPosition( int n, double displayPos[2] )
{
  double worldPos[3];
  double worldOrient[9];
  
  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient) )
    {
    return 0;
    }
  
  return this->SetNthNodeWorldPosition( n, worldPos, worldOrient );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetNthNodeDisplayPosition( int n, int displayPos[2] )
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = displayPos[0];
  doubleDisplayPos[1] = displayPos[1];
  return this->SetNthNodeDisplayPosition( n, doubleDisplayPos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::SetNthNodeDisplayPosition( int n, int X, int Y )
{
  double doubleDisplayPos[2];
  doubleDisplayPos[0] = X;
  doubleDisplayPos[1] = Y;
  return this->SetNthNodeDisplayPosition( n, doubleDisplayPos );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::FindClosestPointOnContour( int X, int Y, 
                                                         double closestWorldPos[3], 
                                                         int *idx )
{
  // Make a line out of this viewing ray
  double p1[4], p2[4], *p3=NULL, *p4=NULL;
  
  double tmp1[4], tmp2[4];
  tmp1[0] = X;
  tmp1[1] = Y;
  tmp1[2] = 0.0;
  this->Renderer->SetDisplayPoint( tmp1 );
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(p1);
  
  tmp1[2] = 1.0;
  this->Renderer->SetDisplayPoint( tmp1 );
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(p2);

  double closestDistance2 = VTK_FLOAT_MAX;
  int closestNode=0;

  // compute a world tolerance based on pixel
  // tolerance on the focal plane
  double fp[4];
  this->Renderer->GetActiveCamera()->GetFocalPoint(fp);
  fp[3] = 1.0;
  this->Renderer->SetWorldPoint(fp);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(tmp1);
  
  tmp1[0] = 0;
  tmp1[1] = 0;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(tmp2);
  
  tmp1[0] = this->PixelTolerance;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(tmp1);
  
  double wt2 = vtkMath::Distance2BetweenPoints(tmp1, tmp2);
  
  // Now loop through all lines and look for closest one within tolerance  
  for(unsigned int i=0;i<this->Internal->Nodes.size();i++)
    {
    for (unsigned int j=0;j<=this->Internal->Nodes[i]->Points.size();j++)
      {
      if ( j == 0 )
        {
        p3 = this->Internal->Nodes[i]->WorldPosition;
        if ( this->Internal->Nodes[i]->Points.size() )
          {
          p4 = this->Internal->Nodes[i]->Points[j]->WorldPosition;
          }
        else
          {
          if ( i < this->Internal->Nodes.size() - 1 )
            {
            p4 = this->Internal->Nodes[i+1]->WorldPosition;
            }
          else if ( this->ClosedLoop )
            {
            p4 = this->Internal->Nodes[0]->WorldPosition;         
            }
          }
        }
      else if ( j == this->Internal->Nodes[i]->Points.size() )
        {
        p3 = this->Internal->Nodes[i]->Points[j-1]->WorldPosition;
        if ( i < this->Internal->Nodes.size() - 1 )
          {
          p4 = this->Internal->Nodes[i+1]->WorldPosition;
          }
        else if ( this->ClosedLoop )
          {
          p4 = this->Internal->Nodes[0]->WorldPosition;         
          }
        else
          {
          // Shouldn't be able to get here (only if we don't have
          // a closed loop but we do have intermediate points after
          // the last node - contradictary conditions)
          continue;
          }
        }
      else
        {
        p3 = this->Internal->Nodes[i]->Points[j-1]->WorldPosition;
        p4 = this->Internal->Nodes[i]->Points[j]->WorldPosition;
        }
      
      // Now we have the four points - check closest intersection
      double u, v;
      
      if ( vtkLine::Intersection( p1, p2, p3, p4, u, v ) )
        {
        double p5[3], p6[3];
        p5[0] = p1[0] + u*(p2[0]-p1[0]);
        p5[1] = p1[1] + u*(p2[1]-p1[1]);
        p5[2] = p1[2] + u*(p2[2]-p1[2]);

        p6[0] = p3[0] + v*(p4[0]-p3[0]);
        p6[1] = p3[1] + v*(p4[1]-p3[1]);
        p6[2] = p3[2] + v*(p4[2]-p3[2]);
        
        double d = vtkMath::Distance2BetweenPoints(p5, p6);
        
        if ( d < wt2 && d < closestDistance2 )
          {
          closestWorldPos[0] = p6[0];
          closestWorldPos[1] = p6[1];
          closestWorldPos[2] = p6[2];
          closestDistance2 = d;
          closestNode = (int)i;
          }
        }
      else
        {
        double d = vtkLine::DistanceToLine( p3, p1, p2 );
        if ( d < wt2 && d < closestDistance2 )
          {
          closestWorldPos[0] = p3[0];
          closestWorldPos[1] = p3[1];
          closestWorldPos[2] = p3[2];
          closestDistance2 = d;
          closestNode = (int)i;
          }
        
        d = vtkLine::DistanceToLine( p4, p1, p2 );
        if ( d < wt2 && d < closestDistance2 )
          {
          closestWorldPos[0] = p4[0];
          closestWorldPos[1] = p4[1];
          closestWorldPos[2] = p4[2];
          closestDistance2 = d;
          closestNode = (int)i;
          }        
        }
      }
    }

  if ( closestDistance2 < VTK_FLOAT_MAX )
    {
    if ( closestNode < this->GetNumberOfNodes() -1 )
      {
      *idx = closestNode+1;
      return 1;
      }
    else if ( this->ClosedLoop )
      {
      *idx = 0;
      return 1;
      }
    }
  
  return 0;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::AddNodeOnContour( int X, int Y )
{
  int idx;
  
  double worldPos[3];
  double worldOrient[9];
  
  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  double displayPos[2];
  displayPos[0] = X;
  displayPos[1] = Y;
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient) )
    {
    return 0;
    }
  
  double pos[3];
  if ( !this->FindClosestPointOnContour( X, Y, pos, &idx ) )
    {
    return 0;
    }
  
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, 
                                                 pos,
                                                 worldPos,
                                                 worldOrient) )
    {
    return 0;
    }
  
  // Add a new point at this position
  vtkContourRepresentationNode *node = new vtkContourRepresentationNode;
  node->WorldPosition[0] = worldPos[0];
  node->WorldPosition[1] = worldPos[1];
  node->WorldPosition[2] = worldPos[2];
  
  memcpy(node->WorldOrientation, worldOrient, 9*sizeof(double) );
  
  this->Internal->Nodes.insert(this->Internal->Nodes.begin() + idx, node);
  
  this->UpdateLines( idx );
  this->NeedToRender = 1;

  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::DeleteNthNode( int n )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  for (unsigned int j=0;j<this->Internal->Nodes[n]->Points.size();j++)
    {
    delete this->Internal->Nodes[n]->Points[j];
    }
  this->Internal->Nodes[n]->Points.clear();
  delete this->Internal->Nodes[n];
  this->Internal->Nodes.erase( this->Internal->Nodes.begin() + n );
  if ( n )
    {
    this->UpdateLines(n-1);
    }
  else
    {
    this->UpdateLines(this->GetNumberOfNodes()-1);
    }
  
  this->NeedToRender = 1;
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::DeleteActiveNode()
{
  return this->DeleteNthNode( this->ActiveNode );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::DeleteLastNode()
{
  return this->DeleteNthNode( this->Internal->Nodes.size() - 1 );
}

//----------------------------------------------------------------------
void vtkContourRepresentation::SetClosedLoop( int val )
{
  if ( this->ClosedLoop != val )
    {
    this->ClosedLoop = val;
    this->UpdateLines(this->GetNumberOfNodes()-1);
    this->Modified();
    }
}

//----------------------------------------------------------------------
void vtkContourRepresentation::UpdateLines( int index )
{
  int start = index - 2;
  int end = index - 1;

  for ( int i = 0; i < 4; i++ )
    {
    int p1 = start;
    int p2 = end;
    
    start++;
    end++;
    
    if ( this->ClosedLoop )
      {
      if ( p1 < 0 )
        {
        p1 += this->GetNumberOfNodes();
        }
      if ( p2 < 0 )
        {
        p2 += this->GetNumberOfNodes();
        }
      if ( p1 >= this->GetNumberOfNodes() )
        {
        p1 -= this->GetNumberOfNodes();
        }
      if ( p2 >= this->GetNumberOfNodes() )
        {
        p2 -= this->GetNumberOfNodes();        
        }
      }
    
    if ( p1 >= 0 && p1 < this->GetNumberOfNodes() &&
         p2 >= 0 && p2 < this->GetNumberOfNodes() )
      {
      this->UpdateLine(p1,p2);
      }
    }
  
  // A check to make sure that we have no line segments in
  // the last node if the loop is not closed
  if ( !this->ClosedLoop && this->GetNumberOfNodes() > 0 )
    {
    int idx = this->Internal->Nodes.size() -1;
    for (unsigned int j=0;j<this->Internal->Nodes[idx]->Points.size();j++)
      {
      delete this->Internal->Nodes[idx]->Points[j];
      }
    this->Internal->Nodes[idx]->Points.clear();
    }
  
  this->BuildLines();
}

//----------------------------------------------------------------------
int vtkContourRepresentation::AddIntermediatePointWorldPosition( int n,
                                                                 double pos[3] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }
  
  vtkContourRepresentationPoint *point = new vtkContourRepresentationPoint;
  point->WorldPosition[0] = pos[0];
  point->WorldPosition[1] = pos[1];
  point->WorldPosition[2] = pos[2];
  this->Internal->Nodes[n]->Points.push_back(point);
  return 1;
}

//----------------------------------------------------------------------
int vtkContourRepresentation::GetNthNodeSlope( int n, double slope[3] )
{
  if ( n < 0 ||
       static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    return 0;
    }

  int idx1, idx2;
  
  if ( n == 0 && !this->ClosedLoop )
    {
    idx1 = 0;
    idx2 = 1;
    }
  else if ( n == this->GetNumberOfNodes()-1 && !this->ClosedLoop )
    {
    idx1 = this->GetNumberOfNodes()-2;
    idx2 = idx1+1;
    }
  else
    {
    idx1 = n - 1;
    idx2 = n + 1;
    
    if ( idx1 < 0 )
      {
      idx1 += this->GetNumberOfNodes();
      }
    if ( idx2 >= this->GetNumberOfNodes() )
      {
      idx2 -= this->GetNumberOfNodes();
      }
    }
  
  slope[0] = 
    this->Internal->Nodes[idx2]->WorldPosition[0] - 
    this->Internal->Nodes[idx1]->WorldPosition[0];
  slope[1] = 
    this->Internal->Nodes[idx2]->WorldPosition[1] - 
    this->Internal->Nodes[idx1]->WorldPosition[1];
  slope[2] = 
    this->Internal->Nodes[idx2]->WorldPosition[2] - 
    this->Internal->Nodes[idx1]->WorldPosition[2];

  vtkMath::Normalize( slope );
  return 1;
}

//----------------------------------------------------------------------
void vtkContourRepresentation::UpdateLine( int idx1, int idx2 )
{
  if ( !this->LineInterpolator )
    {
    return;
    }
  
  // Clear all the points at idx1
  for (unsigned int j=0;j<this->Internal->Nodes[idx1]->Points.size();j++)
    {
    delete this->Internal->Nodes[idx1]->Points[j];
    }
  this->Internal->Nodes[idx1]->Points.clear();
  
  this->LineInterpolator->InterpolateLine( this->Renderer, 
                                           this,
                                           idx1, idx2 );
}

//----------------------------------------------------------------------
int vtkContourRepresentation::ComputeInteractionState(int vtkNotUsed(X), int vtkNotUsed(Y), int vtkNotUsed(modified))
{
  return this->InteractionState;
}

//---------------------------------------------------------------------
void vtkContourRepresentation::ComputeStatisticsOn()
{
  this->SetComputeStatistics( 1 );
}

//---------------------------------------------------------------------
void vtkContourRepresentation::ComputeStatisticsOff()
{
  this->SetComputeStatistics( 0 );
}

//---------------------------------------------------------------------
void vtkContourRepresentation::SetComputeStatistics( int i )
{
  if( i )
    {
    this->ComputeStatistics = 1;
    if (!this->ContourStatistics)
      {
      // Create object
      this->ContourStatistics = new vtkContourStatistics(); 
      }
    }
  else
    {
    this->ComputeStatistics = 0;
    }
      
  // Delete ContourStatistics if no statistics are going to be computed
  if ( !this->ComputeStatistics )
    {
    delete this->ContourStatistics;
    }
    
}

//----------------------------------------------------------------------
double vtkContourRepresentation::GetArea() 
{
  if( this->ComputeStatistics && this->ContourStatistics && this->ClosedLoop )
    {
    this->AssignPolyDataToStatisticsCalculator();
    return this->ContourStatistics->GetArea();
    }
  
  return 0.; // not a closed loop
}
  
//----------------------------------------------------------------------
double vtkContourRepresentation::GetNormalizedShapeIndex() 
{
  if( this->ComputeStatistics && this->ContourStatistics && this->ClosedLoop )
    {
    this->AssignPolyDataToStatisticsCalculator();
    return this->ContourStatistics->GetNormalizedShapeIndex();
    }
  
  return 0.;
}

//----------------------------------------------------------------------
double vtkContourRepresentation::GetPerimeter() 
{
  if( this->ComputeStatistics && this->ContourStatistics && this->ClosedLoop )
    {
    this->AssignPolyDataToStatisticsCalculator();
    return this->ContourStatistics->GetPerimeter();
    }

  return 0.;
}

//----------------------------------------------------------------------
void vtkContourRepresentation::AssignPolyDataToStatisticsCalculator() 
{
  // Rebuild only if the polydata has changed.
  if (const_cast< vtkPolyData * >(
        this->GetContourRepresentationAsPolyData())->GetMTime() > 
        this->ContourStatistics->GetLastBuildTime() ) 
    {
    vtkPolyData *pd = this->ContourStatistics->GetPolyData();
    pd->DeepCopy( const_cast< vtkPolyData * >(this->GetContourRepresentationAsPolyData()));
    pd->SetPolys( pd->GetLines() );
    pd->SetLines( NULL );
    this->ContourStatistics->SetLastBuildTime( 
        this->ContourStatistics->GetPolyData()->GetMTime() );
    }
}

//----------------------------------------------------------------------
void vtkContourRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Pixel Tolerance: " << this->PixelTolerance <<"\n";
  os << indent << "World Tolerance: " << this->WorldTolerance <<"\n";
}
