/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkContourRepresentation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkContourRepresentation - represent the vtkContourWidget
// .SECTION Description
// The vtkContourRepresentation is a superclass for various types of
// representations for the vtkContourWidget.

// .SECTION See Also
// vtkContourWidget vtkHandleRepresentation 


#ifndef __vtkContourRepresentation_h
#define __vtkContourRepresentation_h

#include "vtkWidgetRepresentation.h"

class vtkHandleRepresentation;

class vtkContourRepresentationInternals;
class vtkPointPlacer;
class vtkContourLineInterpolator;
class vtkPolyData;

class VTK_WIDGETS_EXPORT vtkContourRepresentation : public vtkWidgetRepresentation
{
public:
  // Description:
  // Standard VTK methods.
  vtkTypeRevisionMacro(vtkContourRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add a node at a specific world position. Returns 0 if the
  // node could not be added, 1 otherwise.
  virtual int AddNodeAtWorldPosition( double worldPos[3] );
  virtual int AddNodeAtWorldPosition( double worldPos[3],
                                      double worldOrient[9] );
  
  // Description:
  // Add a node at a specific display position. This will be
  // converted into a world position according to the current
  // constrains of the point placer. Return 0 if a point could
  // not be added, 1 otherwise.
  virtual int AddNodeAtDisplayPosition( double displayPos[2] );
  virtual int AddNodeAtDisplayPosition( int displayPos[2] );
  virtual int AddNodeAtDisplayPosition( int X, int Y );

  // Description:
  // Given a display position, activate a node. The closest
  // node within tolerance will be activated. If a node is
  // activated, 1 will be returned, otherwise 0 will be
  // returned.
  virtual int ActivateNode( double displayPos[2] );
  virtual int ActivateNode( int displayPos[2] );
  virtual int ActivateNode( int X, int Y );
  
  // Descirption:
  // Move the active node to a specified world position.
  // Will return 0 if there is no active node or the node
  // could not be moved to that position. 1 will be returned
  // on success.
  virtual int SetActiveNodeToWorldPosition( double pos[3] );
  virtual int SetActiveNodeToWorldPosition( double pos[3],
                                            double orient[9] );
  
  // Description:
  // Move the active node based on a specified display position.
  // The display position will be converted into a world
  // position. If the new position is not valid or there is
  // no active node, a 0 will be returned. Otherwise, on
  // success a 1 will be returned.
  virtual int SetActiveNodeToDisplayPosition( double pos[2] );
  virtual int SetActiveNodeToDisplayPosition( int pos[2] );
  virtual int SetActiveNodeToDisplayPosition( int X, int Y );
  
  // Description:
  // Get the world position of the active node. Will return
  // 0 if there is no active node, or 1 otherwise.
  virtual int GetActiveNodeWorldPosition( double pos[3] );
  
  // Description:
  // Get the world orientation of the active node. Will return
  // 0 if there is no active node, or 1 otherwise.  
  virtual int GetActiveNodeWorldOrientation( double orient[9] );
  
  // Description:
  // Get the display position of the active node. Will return
  // 0 if there is no active node, or 1 otherwise.
  virtual int GetActiveNodeDisplayPosition( double pos[2] );

  // Description:
  // Get the number of nodes.
  virtual int GetNumberOfNodes();
  
  // Description:
  // Get the nth node's display position. Will return
  // 1 on success, or 0 if there are not at least 
  // (n+1) nodes (0 based counting).
  virtual int GetNthNodeDisplayPosition( int n, double pos[2] );
  
  // Description:
  // Get the nth node's world position. Will return
  // 1 on success, or 0 if there are not at least 
  // (n+1) nodes (0 based counting).
  virtual int GetNthNodeWorldPosition( int n, double pos[3] );
  
  // Description:
  // Get the nth node's world orientation. Will return
  // 1 on success, or 0 if there are not at least 
  // (n+1) nodes (0 based counting).
  virtual int GetNthNodeWorldOrientation( int n, double orient[9] );
  
  // Description:
  // Set the nth node's display position. Display position
  // will be converted into world position according to the
  // constraints of the point placer. Will return
  // 1 on success, or 0 if there are not at least 
  // (n+1) nodes (0 based counting) or the world position
  // is not valid.
  virtual int SetNthNodeDisplayPosition( int n, int X, int Y );
  virtual int SetNthNodeDisplayPosition( int n, int pos[2] );
  virtual int SetNthNodeDisplayPosition( int n, double pos[2] );
  
  // Description:
  // Set the nth node's world position. Will return
  // 1 on success, or 0 if there are not at least 
  // (n+1) nodes (0 based counting) or the world
  // position is not valid according to the point
  // placer.
  virtual int SetNthNodeWorldPosition( int n, double pos[3] );
  virtual int SetNthNodeWorldPosition( int n, double pos[3],
                                       double orient[9] );
  
  // Description:
  // Get the nth node's slope. Will return
  // 1 on success, or 0 if there are not at least 
  // (n+1) nodes (0 based counting).
  virtual int  GetNthNodeSlope( int idx, double slope[3] );
  
  // Descirption:
  // For a given node n, get the number of intermediate
  // points between this node and the node at
  // (n+1). If n is the last node and the loop is
  // closed, this is the number of intermediate points
  // between node n and node 0. 0 is returned if n is
  // out of range.
  virtual int GetNumberOfIntermediatePoints( int n );
  
  // Description:
  // Get the world position of the intermediate point at
  // index idx between nodes n and (n+1) (or n and 0 if
  // n is the last node and the loop is closed). Returns
  // 1 on success or 0 if n or idx are out of range.
  virtual int GetIntermediatePointWorldPosition( int n, 
                                                 int idx, double point[3] );
  
  // Description:
  // Add an intermediate point between node n and n+1
  // (or n and 0 if n is the last node and the loop is closed).
  // Returns 1 on success or 0 if n is out of range.
  virtual int AddIntermediatePointWorldPosition( int n, 
                                                 double point[3] );

  // Description:
  // Delete the last node. Returns 1 on success or 0 if 
  // there were not any nodes.
  virtual int DeleteLastNode();
  
  // Description:
  // Delete the active node. Returns 1 on success or 0 if
  // the active node did not indicate a valid node.
  virtual int DeleteActiveNode();
  
  // Description:
  // Delete the nth node. Return 1 on success or 0 if n
  // is out of range.
  virtual int DeleteNthNode( int n );

  // Description:
  // Given a specific X, Y pixel location, add a new node 
  // on the contour at this location. 
  virtual int AddNodeOnContour( int X, int Y );
  
  // Description:
  // The tolerance to use when calculations are performed in 
  // display coordinates
  vtkSetClampMacro(PixelTolerance,int,1,100);
  vtkGetMacro(PixelTolerance,int);

  // Description:
  // The tolerance to use when calculations are performed in
  // world coordinates
  vtkSetClampMacro(WorldTolerance, double, 0.0, VTK_FLOAT_MAX);
  vtkGetMacro(WorldTolerance, double);

//BTX -- used to communicate about the state of the representation
  enum {
    Outside=0,
    Nearby
  };
  
  enum {
    Inactive = 0,
    Translate
  };
//ETX

  // Description:
  // Set / get the current operation. The widget is either
  // inactive, or it is being translated.
  vtkGetMacro( CurrentOperation, int );
  vtkSetClampMacro( CurrentOperation, int, 
                    vtkContourRepresentation::Inactive,
                    vtkContourRepresentation::Translate );
  void SetCurrentOperationToInactive()
    { this->SetCurrentOperation( vtkContourRepresentation::Inactive ); }
  void SetCurrentOperationToTranslate()
    { this->SetCurrentOperation( vtkContourRepresentation::Translate ); }

  // Descirption:
  // Set / get the Point Placer. The point placer is
  // responsible for converting display coordinates into
  // world coordinates according to some constraints, and
  // for validating world positions.
  void SetPointPlacer( vtkPointPlacer * );
  vtkGetObjectMacro( PointPlacer, vtkPointPlacer );
  
  // Description:
  // Set / Get the Line Interpolator. The line interpolator
  // is repsonsible for generating the line segments connecting
  // nodes.
  void SetLineInterpolator( vtkContourLineInterpolator *);
  vtkGetObjectMacro( LineInterpolator, vtkContourLineInterpolator );
  
  // Description:
  // These are methods that satisfy vtkWidgetRepresentation's API.
  virtual void BuildRepresentation()=0;
  virtual int ComputeInteractionState(int X, int Y, int modified=0)=0;
  virtual void StartWidgetInteraction(double e[2])=0;
  virtual void WidgetInteraction(double e[2])=0;

  // Description:
  // Methods required by vtkProp superclass.
  virtual void ReleaseGraphicsResources(vtkWindow *w)=0;
  virtual int RenderOverlay(vtkViewport *viewport)=0;
  virtual int RenderOpaqueGeometry(vtkViewport *viewport)=0;
  virtual int RenderTranslucentGeometry(vtkViewport *viewport)=0;

  // Description:
  // Set / Get the ClosedLoop value. This ivar indicates whether the contour
  // forms a closed loop. 
  void SetClosedLoop( int val );
  vtkGetMacro( ClosedLoop, int );
  vtkBooleanMacro( ClosedLoop, int );
  
  // Description:
  // Get the points in this contour as a vtkPolyData. 
//BTX
  virtual const vtkPolyData * GetContourRepresentationAsPolyData() const = 0;
//ETX

protected:
  vtkContourRepresentation();
  ~vtkContourRepresentation();

//BTX  
  friend class vtkContourWidget;
//ETX
  
  // Selection tolerance for the handles
  int    PixelTolerance;
  double WorldTolerance;

  vtkPointPlacer             *PointPlacer;
  vtkContourLineInterpolator *LineInterpolator;
  
  int ActiveNode;
  
  int CurrentOperation;
  int ClosedLoop;
  
  vtkContourRepresentationInternals *Internal;

  void AddNodeAtWorldPositionInternal( double worldPos[3],
                                       double worldOrient[9] );
  void SetNthNodeWorldPositionInternal( int n, double worldPos[3],
                                        double worldOrient[9] );

  void UpdateLines( int index );
  void UpdateLine( int idx1, int idx2 );

  int FindClosestPointOnContour( int X, int Y, 
                                 double worldPos[3],
                                 int *idx );
  
  virtual void BuildLines()=0;

  // This method is called when something changes in the point
  // placer. It will cause all points to
  // be updates, and all lines to be regenerated.
  // Should be extended to detect changes in the line interpolator
  // too.
  virtual int  UpdateContour();
  vtkTimeStamp ContourBuildTime;
  
  void ComputeMidpoint( double p1[3], double p2[3], double mid[3] )
    {
      mid[0] = (p1[0] + p2[0])/2;
      mid[1] = (p1[1] + p2[1])/2;
      mid[2] = (p1[2] + p2[2])/2;
    }
  
private:
  vtkContourRepresentation(const vtkContourRepresentation&);  //Not implemented
  void operator=(const vtkContourRepresentation&);  //Not implemented
};

#endif
