/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMeasureRepresentation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMeasureRepresentation.h"
#include "vtkHandleRepresentation.h"
#include "vtkCoordinate.h"
#include "vtkRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkBox.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"

vtkCxxRevisionMacro(vtkMeasureRepresentation, "1.2");

vtkCxxSetObjectMacro(vtkMeasureRepresentation,HandleRepresentation,vtkHandleRepresentation);


//----------------------------------------------------------------------
vtkMeasureRepresentation::vtkMeasureRepresentation()
{
  this->HandleRepresentation  = NULL;
  this->Point1Representation = NULL;
  this->Point2Representation = NULL;

  this->Tolerance = 5;
  this->Placed = 0;
}

//----------------------------------------------------------------------
vtkMeasureRepresentation::~vtkMeasureRepresentation()
{
  if ( this->HandleRepresentation )
    {
    this->HandleRepresentation->Delete();
    }
  if ( this->Point1Representation )
    {
    this->Point1Representation->Delete();
    }
  if ( this->Point2Representation )
    {
    this->Point2Representation->Delete();
    }
}

  
//----------------------------------------------------------------------
void vtkMeasureRepresentation::InstantiateHandleRepresentation()
{
  if ( ! this->Point1Representation )
    {
    this->Point1Representation = this->HandleRepresentation->NewInstance();
    this->Point1Representation->ShallowCopy(this->HandleRepresentation);
    }
  
  if ( ! this->Point2Representation )
    {
    this->Point2Representation = this->HandleRepresentation->NewInstance();
    this->Point2Representation->ShallowCopy(this->HandleRepresentation);
    }
}
  

//----------------------------------------------------------------------
void vtkMeasureRepresentation::GetPoint1WorldPosition(double pos[3])
{
  this->Point1Representation->GetWorldPosition(pos);
}

//----------------------------------------------------------------------
void vtkMeasureRepresentation::GetPoint2WorldPosition(double pos[3])
{
  this->Point2Representation->GetWorldPosition(pos);
}

//----------------------------------------------------------------------
int vtkMeasureRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
  // See if we are near one of the end points or outside
  double pos1[3], pos2[3];
  this->GetPoint1DisplayPosition(pos1);
  this->GetPoint2DisplayPosition(pos2);
  
  double p1[3], p2[3], xyz[3];
  xyz[0] = static_cast<double>(X);
  xyz[1] = static_cast<double>(Y);
  p1[0] = static_cast<double>(pos1[0]);
  p1[1] = static_cast<double>(pos1[1]);
  p2[0] = static_cast<double>(pos2[0]);
  p2[1] = static_cast<double>(pos2[1]);
  xyz[2] = p1[2] = p2[2] = 0.0;

  double tol2 = this->Tolerance*this->Tolerance;
  if ( vtkMath::Distance2BetweenPoints(xyz,p1) <= tol2 )
    {
    this->InteractionState = vtkMeasureRepresentation::NearP1;
    }
  else if ( vtkMath::Distance2BetweenPoints(xyz,p2) <= tol2 )
    {
    this->InteractionState = vtkMeasureRepresentation::NearP2;
    }
  else 
    {
    this->InteractionState = vtkMeasureRepresentation::Outside;
    }

  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkMeasureRepresentation::StartWidgetInteraction(double e[2])
{
  double pos[3];
  pos[0] = e[0];
  pos[1] = e[1];
  pos[2] = 0.0;
  this->SetPoint1DisplayPosition(pos);
  this->SetPoint2DisplayPosition(pos);
}

//----------------------------------------------------------------------
void vtkMeasureRepresentation::WidgetInteraction(double e[2])
{
  double pos[3];
  pos[0] = e[0];
  pos[1] = e[1];
  pos[2] = 0.0;
  this->SetPoint2DisplayPosition(pos);
}

//----------------------------------------------------------------------
void vtkMeasureRepresentation::BuildRepresentation()
{
  // Make sure the handles are up to date
  this->Point1Representation->BuildRepresentation();
  this->Point2Representation->BuildRepresentation();
}

//----------------------------------------------------------------------
void vtkMeasureRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Tolerance: " << this->Tolerance <<"\n";
}
