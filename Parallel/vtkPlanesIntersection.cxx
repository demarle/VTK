/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPlanesIntersection.cxx

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

#include "vtkMath.h"
#include "vtkPlanesIntersection.h"
#include "vtkPointsProjectedHull.h"
#include "vtkFloatArray.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkCell.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkPlanesIntersection, "1.3");
vtkStandardNewMacro(vtkPlanesIntersection);

// Experiment shows that we get plane equation values on the
//  order of 10e-6 when the point is actually on the plane

#define VTK_SMALL_DOUBLE (10e-5)

const int Inside   = 0;
const int Outside  = 1;        
const int Straddle = 2;

const int Xdim=0;  // don't change these three values
const int Ydim=1;
const int Zdim=2;

vtkPlanesIntersection::vtkPlanesIntersection()
{
  this->Plane   = NULL;
  this->regionPts = NULL;
}
vtkPlanesIntersection::~vtkPlanesIntersection()
{
  if (this->regionPts)
    {
    this->regionPts->Delete();
    this->regionPts = NULL;
    }
  if (this->Plane)
    {
    delete [] this->Plane;
    this->Plane = NULL;
    }
}
void vtkPlanesIntersection::SetRegionVertices(vtkPoints *v)
{
  int i;
  if (this->regionPts) this->regionPts->Delete();
  this->regionPts = vtkPointsProjectedHull::New();

  if (v->GetDataType() == VTK_DOUBLE)
    {
    this->regionPts->DeepCopy(v);
    }
  else
    {
    this->regionPts->SetDataTypeToDouble();

    int npts = v->GetNumberOfPoints();
    this->regionPts->SetNumberOfPoints(npts);

    double *pt;
    for (i=0; i<npts; i++)
      {
      pt = v->GetPoint(i);
      regionPts->SetPoint(i, pt[0], pt[1], pt[2]);
      }
    }
}
void vtkPlanesIntersection::SetRegionVertices(double *v, int nvertices)
{
  int i;
  if (this->regionPts) this->regionPts->Delete();
  this->regionPts = vtkPointsProjectedHull::New();
  
  this->regionPts->SetDataTypeToDouble();
  this->regionPts->SetNumberOfPoints(nvertices);
  
  for (i=0; i<nvertices; i++)
    {
    this->regionPts->SetPoint(i, v + (i*3));
    }
}
int vtkPlanesIntersection::GetRegionVertices(double *v, int nvertices)
{
  int i;
  if (this->regionPts == NULL) this->ComputeRegionVertices();

  int npts = this->regionPts->GetNumberOfPoints();

  if (npts > nvertices) npts = nvertices;

  for (i=0; i<npts; i++)
    {
    this->regionPts->GetPoint(i, v + i*3);
    }

  return npts;
}
int vtkPlanesIntersection::GetNumRegionVertices()
{
  if (this->regionPts == NULL) this->ComputeRegionVertices();

  return this->regionPts->GetNumberOfPoints();
}

//---------------------------------------------------------------------
// Determine whether the axis aligned box provided intersects
// the convex region bounded by the planes.
//---------------------------------------------------------------------

int vtkPlanesIntersection::IntersectsRegion(vtkPoints *R)
{
  int plane;
  int allInside=0;
  int nplanes = this->GetNumberOfPlanes();

  if (nplanes < 4)
    {
    vtkErrorMacro("invalid region - less than 4 planes");
    return 0;
    }

  if (this->regionPts == NULL)
    {
    this->ComputeRegionVertices();
    if (this->regionPts->GetNumberOfPoints() < 4)
      {
      vtkErrorMacro("Invalid region: zero-volume intersection");
      return 0;
      }
    }

  if (R->GetNumberOfPoints() < 8)
    {
    vtkErrorMacro("invalid box");
    return 0;
    }

  int *where = new int[nplanes];

  int intersects = -1;

//  Here's the algorithm from Graphics Gems IV, page 81,
//            
//  R is an axis aligned box (could represent a region in a spatial
//    spatial partitioning of a volume of data).
//            
//  P is a set of planes defining a convex region in space (could be
//    a view frustum).
//            
//  The question is does P intersect R.  We expect to be doing the 
//    calculation for one P and many Rs.

//    You may wonder why we don't do what vtkClipPolyData does, which
//    computes the following on every point of it's PolyData input:
//
//      for each point in the input
//        for each plane defining the convex region
//          evaluate plane eq to determine if outside, inside or on plane
//
//     For each cell, if some points are inside and some outside, then
//     vtkClipPolyData decides it straddles the region and clips it.  If
//     every point is inside, it tosses it.
//
//     The reason is that the Graphics Gems algorithm is faster in some
//     cases (we may only need to evaluate one vertex of the box).  And
//     also because if the convex region passes through the box without
//     including any vertices of the box, all box vertices will be
//     "outside" and the algorithm will fail.  vtkClipPolyData assumes
//     cells are very small relative to the clip region.  In general
//     the axis-aligned box may be a large portion of world coordinate
//     space, and the convex region a view frustum representing a
//     small portion of the screen.


//  1.  If R does not intersect P's bounding box, return 0.

  if (this->IntersectsBoundingBox(R) == 0)
    {
    intersects = 0;
    }

//  2.  If P's bounding box is entirely inside R, return 1.

  else if (this->EnclosesBoundingBox(R) == 1)
    {
    intersects = 1;
    }

//  3.  For each face plane F of P
//
//      Suppose the plane equation is negative inside P and
//      positive outside P. Choose the vertex (n) of R which is
//      most in the direction of the negative pointing normal of
//      the plane.  The opposite vertex (p) is most in the
//      direction of the positive pointing normal.  (This is
//      a very quick calculation.)
//
//      If n is on the positive side of the plane, R is
//      completely outside of P, so return 0.
//
//      If n and p are both on the negative side, then R is on
//      the "inside" of F.  Keep track to see if all R is inside
//      all planes defining the region.

  else
    {
    if (this->Plane == NULL) this->SetPlaneEquations(); 

    allInside = 1;

    for (plane=0; plane < nplanes; plane++)
      {
      where[plane] = this->EvaluateFacePlane(plane, R);

      if (allInside &&  (where[plane] != Inside))
        {
        allInside = 0;
        }

      if (where[plane] == Outside)
        {
        intersects = 0;

        break;
        }
      }
    }

  if (intersects == -1)
    {

//  4.  If n and p were "inside" all faces, R is inside P 
//      so return 1.

    if ( allInside)
      {
      intersects = 1;
      }
//  5.  For each of three orthographic projections (X, Y and Z)
//
//      Compute the equations of the edge lines of P in those views.
//
//      If R's projection lies outside any of these lines (using 2D
//      version of n & p tests), return 0.

    else if ((this->IntersectsProjection(R, Xdim) == 0) ||
             (this->IntersectsProjection(R, Ydim) == 0) ||
             (this->IntersectsProjection(R, Zdim) == 0)    )
      {
      }
    else
      {
//    6.  Return 1.

      intersects = 1;
      }
    }

  delete [] where;

  return (intersects==1);
}

// a static convenience function - since we have all the machinery
//  in this class, we can compute whether an arbitrary polygon intersects
//  an axis aligned box
//
// it is assumed "pts" represents a planar polygon
//

int vtkPlanesIntersection::PolygonIntersectsBBox(double bounds[6], vtkPoints *pts)
{
  // a bogus vtkPlanesIntersection object containing only one plane

  vtkPlanesIntersection *pi = vtkPlanesIntersection::New();

  pi->SetRegionVertices(pts);

  vtkPoints *Box = vtkPoints::New();
  Box->SetNumberOfPoints(8);
  Box->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  Box->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  Box->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  Box->SetPoint(3, bounds[0], bounds[3], bounds[4]);
  Box->SetPoint(4, bounds[0], bounds[2], bounds[5]);
  Box->SetPoint(5, bounds[1], bounds[2], bounds[5]);
  Box->SetPoint(6, bounds[1], bounds[3], bounds[5]);
  Box->SetPoint(7, bounds[0], bounds[3], bounds[5]);

  int intersects = -1;

//  1.  Does Box intersect the polygon's bounding box?

  if (pi->IntersectsBoundingBox(Box) == 0)
    {
    intersects = 0;
    }

//  2.  If so, does Box entirely contain the polygon's bounding box?

  else if (pi->EnclosesBoundingBox(Box) == 1)
    {
    intersects = 1;
    }


  if (intersects == -1)
    {

//  3. If not, determine whether the Box intersects the plane of the polygon

    vtkPoints *origin = vtkPoints::New();
    origin->SetNumberOfPoints(1);
    origin->SetPoint(0, pts->GetPoint(0));

    vtkFloatArray *normal = vtkFloatArray::New();
    normal->SetNumberOfComponents(3);
    normal->SetNumberOfTuples(1);

    // find 3 points that are not co-linear and compute a normal

    double nvec[3], p0[3], p1[3], pp[3];

    int npts = pts->GetNumberOfPoints();

    pts->GetPoint(0, p0);
    pts->GetPoint(1, p1);

    for (int p = 2; p < npts; p++)
      {
      pts->GetPoint(p, pp);

      vtkPlanesIntersection::ComputeNormal(p0, p1, pp, nvec);

      if (vtkPlanesIntersection::GoodNormal(nvec))
        {
        break;
        }
      }

    normal->SetTuple(0, nvec);

    pi->SetPoints(origin);
    pi->SetNormals(normal);

    origin->Delete();
    normal->Delete();

    pi->SetPlaneEquations();

    int where = pi->EvaluateFacePlane(0, Box);

    if (where != Straddle)
      {
      intersects = 0;
      }
    } 
  
  if (intersects == -1)
    {

//  4.  The Box intersects the plane of the polygon.
//  
//      For each of three orthographic projections (X, Y and Z),
//      compute the equations of the edge lines of the polygon in those views.
//
//      If Box's projection lies outside any of these projections, they
//      don't intersect in 3D.  Otherwise they do intersect in 3D.

    if ((pi->IntersectsProjection(Box, Xdim) == 0) ||
        (pi->IntersectsProjection(Box, Ydim) == 0) ||
        (pi->IntersectsProjection(Box, Zdim) == 0)    )
      {
      intersects = 0;
      }
    else
      {
      intersects = 1;
      }
    }
    
  Box->Delete();
  pi->Delete();

  return intersects;
}
    
//---------------------------------------------------------------------
// Some convenience functions that build a vtkPlanesIntersection object
// out of a convex region.             
//---------------------------------------------------------------------
      
// This function returns the vtkPlanesIntersection
// object representing a view frustum in world coordinates.  The view
// frustum is defined by a sub-rectangle of the view plane, some
// subset of [-1,1],[-1,1].
//  

vtkPlanesIntersection *vtkPlanesIntersection::ConvertFrustumToWorld(vtkRenderer *ren,
                      double x0, double x1, double y0, double y1)
{
  vtkPlanesIntersection *planes;
    
  if (ren->GetActiveCamera()->GetParallelProjection())
    {
    planes = ConvertFrustumToWorldParallel(ren, x0, x1, y0, y1);
    }
  else
    { 
    planes = ConvertFrustumToWorldPerspective(ren, x0, x1, y0, y1);
    }

  return planes;
}

vtkPlanesIntersection *vtkPlanesIntersection::ConvertFrustumToWorldPerspective(
  vtkRenderer *ren,
  double xmin, double xmax, double ymin, double ymax)
{
  int i;
  double planeEq[24];
  double worldN[3], newPt[3];

  int *winsize = ren->GetRenderWindow()->GetSize();

  ren->GetActiveCamera()->GetFrustumPlanes(winsize[0]/winsize[1], planeEq);

  vtkFloatArray *newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->SetNumberOfTuples(6);

  vtkPoints *newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(6);

  // For each plane of frustum, compute outward pointing normal
  // and a point on the plane.  This is for a frustum defined
  // by a rectangular sub-region of the viewport.

  // First, front and back clipping planes.  We use the fact that
  //   vtkCamera returns the frustum planes in a particular order,
  //   4th equation is for near clip plane, 5th is for far.

  for (i=4; i<6; i++)
    {
    double *plane = planeEq + 4*i;

    worldN[0] = -plane[0];
    worldN[1] = -plane[1];
    worldN[2] = -plane[2];

    newNormals->SetTuple(i, worldN);

    vtkPlanesIntersection::PointOnPlane(
      plane[0], plane[1], plane[2], plane[3], newPt);

    newPts->SetPoint(i, newPt);
    }
  // Then the left, right, lower, and upper frustum planes.
  // Get three points on each plane, so we can get a normal.

  double ulFront[3], urFront[3], llFront[3], lrFront[3];
  double lrBack[3], ulBack[3];

  // Need a sensible z (took me 3 days to figure that out)

  double sensibleZ = vtkPlanesIntersection::SensibleZCoordinate(ren);

  // upper left
  ulFront[0] = xmin; ulFront[1] = ymax; ulFront[2] = sensibleZ;
  ren->ViewToWorld(ulFront[0], ulFront[1], ulFront[2]);

  // upper right      
  urFront[0] = xmax; urFront[1] = ymax; urFront[2] = sensibleZ;
  ren->ViewToWorld(urFront[0], urFront[1], urFront[2]);

  // lower left
  llFront[0] = xmin; llFront[1] = ymin; llFront[2] = sensibleZ;
  ren->ViewToWorld(llFront[0], llFront[1], llFront[2]);

  // lower right
  lrFront[0] = xmax; lrFront[1] = ymin; lrFront[2] = sensibleZ;
  ren->ViewToWorld(lrFront[0], lrFront[1], lrFront[2]);

  //  We need second, different reasonable z value for back points
  
  double *dir = ren->GetActiveCamera()->GetDirectionOfProjection();
  
  double dx = dir[0] * .01;
  double dy = dir[1] * .01;
  double dz = dir[2] * .01;
  
  ulBack[0] = ulFront[0] + dx;
  ulBack[1] = ulFront[1] + dy;
  ulBack[2] = ulFront[2] + dz;

  ren->WorldToView(ulBack[0], ulBack[1], ulBack[2]);
  ulBack[0] = xmin;
  ulBack[1] = ymax;
  ren->ViewToWorld(ulBack[0], ulBack[1], ulBack[2]);
  
  lrBack[0] = lrFront[0] + dx;
  lrBack[1] = lrFront[1] + dy;
  lrBack[2] = lrFront[2] + dz;
  
  ren->WorldToView(lrBack[0], lrBack[1], lrBack[2]);
  lrBack[0] = xmax;
  lrBack[1] = ymin;
  ren->ViewToWorld(lrBack[0], lrBack[1], lrBack[2]);
  
  // left plane
  
  vtkPlanesIntersection::ComputeNormal(ulBack, ulFront, llFront, worldN);
  newNormals->SetTuple(0, worldN);
  newPts->SetPoint(0, ulFront);
  
  // right plane

  vtkPlanesIntersection::ComputeNormal(lrBack, lrFront, urFront, worldN);
  newNormals->SetTuple(1, worldN);
  newPts->SetPoint(1, urFront);

  // lower plane

  vtkPlanesIntersection::ComputeNormal(llFront, lrFront, lrBack, worldN);
  newNormals->SetTuple(2, worldN);
  newPts->SetPoint(2, lrFront);

  // upper plane

  vtkPlanesIntersection::ComputeNormal(urFront, ulFront, ulBack, worldN);
  newNormals->SetTuple(3, worldN);
  newPts->SetPoint(3, urFront);

  vtkPlanesIntersection *planes = vtkPlanesIntersection::New();
  planes->SetPoints(newPts);
  planes->SetNormals(newNormals);

  newPts->Delete();
  newNormals->Delete();

  return planes;
}
vtkPlanesIntersection *vtkPlanesIntersection::ConvertFrustumToWorldParallel(
  vtkRenderer *ren,
  double xmin, double xmax, double ymin, double ymax)
{
  double planeEq[24];
  double newPt[3], worldN[3];
  int i;

  int *winsize = ren->GetRenderWindow()->GetSize();

  ren->GetActiveCamera()->GetFrustumPlanes(winsize[0]/winsize[1], planeEq);

  vtkFloatArray *newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->SetNumberOfTuples(6);

  vtkPoints *newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(6);

  double sensibleZ = vtkPlanesIntersection::SensibleZCoordinate(ren);

  double *plane = planeEq;

  for (i=0; i<6; i++)
    {
    // outward pointing normal

    worldN[0] = -plane[0];
    worldN[1] = -plane[1];
    worldN[2] = -plane[2];

    newNormals->SetTuple(i, worldN);

    // Get a point on this plane
    //   We use the fact that vtkCamera gives us the frustum planes in
    //   this order: left, right, lower, upper, front clip plane, back
    //   clip plane.
  
    if (i < 4)
      {
      switch (i)
        {
        case 0:
          newPt[0] = xmin;
          newPt[1] = 0.0; 
          newPt[2] = sensibleZ;
          break;
        case 1:
          newPt[0] = xmax;
          newPt[1] = 0.0;
          newPt[2] = sensibleZ;
          break;
        case 2:
          newPt[0] = 0.0;
          newPt[1] = ymin;
          newPt[2] = sensibleZ;
          break;      
        case 3:       
          newPt[0] = 0.0;
          newPt[1] = ymax;
          newPt[2] = sensibleZ;
          break;
        }
      ren->ViewToWorld(newPt[0], newPt[1], newPt[2]);
      } 
    else
      {
      vtkPlanesIntersection::PointOnPlane(
        plane[0], plane[1], plane[2], plane[3], newPt);
      }
    plane += 4; 
    newPts->SetPoint(i, newPt);
    }

  vtkPlanesIntersection *planes = vtkPlanesIntersection::New();
  planes->SetPoints(newPts);
  planes->SetNormals(newNormals);
  
  newPts->Delete();
  newNormals->Delete();

  return planes; 
}

// a static convenience function that converts a 3D cell into a
// vtkPlanesIntersection object

vtkPlanesIntersection *vtkPlanesIntersection::Convert3DCell(vtkCell *cell)
{ 
  int i;
  int nfaces = cell->GetNumberOfFaces();
  
  vtkPoints *origins = vtkPoints::New();
  origins->SetNumberOfPoints(nfaces);
  
  vtkFloatArray *normals = vtkFloatArray::New();
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(nfaces);
  
  double inside[3] = {0.0, 0.0, 0.0};
  
  for (i=0; i < nfaces; i++)
    {
    vtkCell *face = cell->GetFace(i);
    
    vtkPoints *facePts = face->GetPoints();
    int npts = facePts->GetNumberOfPoints();

    double p0[3], p1[3], pp[3], n[3];
    
    facePts->GetPoint(0, p0);
    facePts->GetPoint(1, p1);

    for (int p = 2; p < npts; p++)
      {
      facePts->GetPoint(p, pp);

      vtkPlanesIntersection::ComputeNormal(pp, p1, p0, n);

      if (vtkPlanesIntersection::GoodNormal(n))
        {
        break;
        }
      }
    
    origins->SetPoint(i, pp);
    normals->SetTuple(i, n);
    
    inside[0] += p1[0];
    inside[1] += p1[1];
    inside[2] += p1[2];
    }
  
  inside[0] /= (double)nfaces;
  inside[1] /= (double)nfaces;
  inside[2] /= (double)nfaces;
  
  // ensure that all normals are outward pointing
  
  for (i=0; i < nfaces; i++)
    {
    double ns[3], xs[3];
    double n[3], x[3], p[4];
    
    normals->GetTuple(i, ns);
    origins->GetPoint(i, xs);
    
    n[0] = (double)ns[0]; x[0] = (double)xs[0];
    n[1] = (double)ns[1]; x[1] = (double)xs[1];
    n[2] = (double)ns[2]; x[2] = (double)xs[2];

    double outside[3];

    outside[0] = x[0] + n[0];
    outside[1] = x[1] + n[1];
    outside[2] = x[2] + n[2];

    vtkPlanesIntersection::PlaneEquation(n, x, p);

    double insideVal = vtkPlanesIntersection::EvaluatePlaneEquation(inside, p);

    double normalDirection =
      vtkPlanesIntersection::EvaluatePlaneEquation(outside, p);

    int sameSide =
      ( (insideVal < 0) && (normalDirection < 0)) ||
      ( (insideVal > 0) && (normalDirection > 0));

    if (sameSide)
      {
      ns[0] = -ns[0];
      ns[1] = -ns[1];
      ns[2] = -ns[2];

      normals->SetTuple(i, ns);
      }
    }

  vtkPlanesIntersection *pi = vtkPlanesIntersection::New();

  pi->SetPoints(origins);
  pi->SetNormals(normals);

  origins->Delete();
  normals->Delete();

  pi->SetRegionVertices(cell->GetPoints());

  return pi;
}

//--------------------------------------------------------------------------

void vtkPlanesIntersection::ComputeNormal(double *p1, double *p2, double *p3,
                                          double normal[3])
{
  double v1[3], v2[3];

  v1[0] = p1[0] - p2[0]; v1[1] = p1[1] - p2[1]; v1[2] = p1[2] - p2[2];
  v2[0] = p3[0] - p2[0]; v2[1] = p3[1] - p2[1]; v2[2] = p3[2] - p2[2];

  vtkMath::Cross(v1, v2, normal);
    
  return;
}
int vtkPlanesIntersection::GoodNormal(double *n)
{
  if ( (n[0] < VTK_SMALL_DOUBLE) || (n[0] > VTK_SMALL_DOUBLE) ||
       (n[1] < VTK_SMALL_DOUBLE) || (n[1] > VTK_SMALL_DOUBLE) ||
       (n[2] < VTK_SMALL_DOUBLE) || (n[2] > VTK_SMALL_DOUBLE) )
    {
    return 1; 
    }
  else
    {
    return 0;
    } 
}      
void vtkPlanesIntersection::PointOnPlane(
  double a, double b, double c, double d, double pt[3])
{   
  double px = ( a > 0.0) ? a : -a;
  double py = ( b > 0.0) ? b : -b;
  double pz = ( c > 0.0) ? c : -c;
    
  pt[0] = pt[1] = pt[2] = 0.0;
      
  if ((py >= px) && (py >= pz))
    {
    pt[1] = -(d/b);
    }
  else if ((px >= py) && (px >= pz))
    {
    pt[0] = -(d/a);
    }
  else
    {
    pt[2] = -(d/c);                  
    }
  return;
} 
double vtkPlanesIntersection::SensibleZCoordinate(vtkRenderer *ren)
{ 
  double FP[3];

  ren->GetActiveCamera()->GetFocalPoint(FP);
  ren->WorldToView(FP[0], FP[1], FP[2]);
  
  return FP[2];
}

double vtkPlanesIntersection::EvaluatePlaneEquation(double *x, double *p)
{
  return (x[0]*p[0] + x[1]*p[1] + x[2]*p[2] + p[3]); 
}                                         
void vtkPlanesIntersection::PlaneEquation(double *n, double *x, double *p)
{
  p[0] = n[0];
  p[1] = n[1];
  p[2] = n[2];
  p[3] = -(n[0]*x[0] + n[1]*x[1] + n[2]*x[2]);
}

// The plane equations ***********************************************

void vtkPlanesIntersection::SetPlaneEquations()
{
  int i;
  int nplanes = this->GetNumberOfPlanes();

  // vtkPlanes stores normals & pts instead of
  //   plane equation coefficients

  if (this->Plane) delete [] this->Plane;

  this->Plane = new double[nplanes*4];

  for (i=0; i<nplanes; i++)
    {
    double n[3], x[3];

    this->Points->GetPoint(i, x);
    this->Normals->GetTuple(i, n);
    
    double nd[3], xd[3];
    
    nd[0] = n[0]; xd[0] = x[0];
    nd[1] = n[1]; xd[1] = x[1];
    nd[2] = n[2]; xd[2] = x[2];
    
    double *p = this->Plane + (i*4);
    
    vtkPlanesIntersection::PlaneEquation(nd, xd, p);
    }
}

// Compute region vertices if not set explicity ********************

void vtkPlanesIntersection::ComputeRegionVertices()
{
  double M[3][3];
  double rhs[3];
  double testv[3];
  int i, j, k;
  int nplanes = this->GetNumberOfPlanes();

  if (this->regionPts) this->regionPts->Delete();
  this->regionPts = vtkPointsProjectedHull::New();

  if (nplanes <= 3)
    {
    vtkErrorMacro( << 
                   "vtkPlanesIntersection::ComputeRegionVertices invalid region");
    return;
    }

  if (this->Plane == NULL)
    {
    this->SetPlaneEquations();
    }

  // This is an expensive process.  Better if vertices are
  // set in SetRegionVertices().  We're testing every triple of
  // planes to see if they intersect in a point that is
  // not "outside" any plane.

  int nvertices=0;

  for (i=0; i < nplanes; i++)
    {
    for (j=i+1; j < nplanes; j++)
      {
      for (k=j+1; k < nplanes; k++)
        {
        this->planesMatrix(i, j, k, M);

        int notInvertible = this->Invert3x3(M);

        if (notInvertible) continue;

        this->planesRHS(i, j, k, rhs);

        vtkMath::Multiply3x3(M, rhs, testv);

        if (duplicate(testv)) continue;

        int outside = this->outsideRegion(testv);

        if (!outside)
          {
          this->regionPts->InsertPoint(nvertices, testv);
          nvertices++;
          }
        }
      }
    }
}
int vtkPlanesIntersection::duplicate(double testv[3]) const
{
  int i;
  double pt[3];
  int npts = this->regionPts->GetNumberOfPoints();

  for (i=0; i<npts; i++)
    {
    this->regionPts->GetPoint(i, pt);

    if ( (pt[0] == testv[0]) && (pt[1] == testv[1]) && (pt[2] == testv[2]))
      {
      return 1;
      }
    }
  return 0;
}
void vtkPlanesIntersection::planesMatrix(int p1, int p2, int p3, double M[3][3]) const
{
  int i;
  for (i=0; i<3; i++)
    {
    M[0][i] = this->Plane[p1*4 + i];
    M[1][i] = this->Plane[p2*4 + i];
    M[2][i] = this->Plane[p3*4 + i];
    }
}
void vtkPlanesIntersection::planesRHS(int p1, int p2, int p3, double r[3]) const
{
  r[0] = -(this->Plane[p1*4 + 3]);
  r[1] = -(this->Plane[p2*4 + 3]);
  r[2] = -(this->Plane[p3*4 + 3]);
}
int vtkPlanesIntersection::outsideRegion(double testv[3])
{
  int i;
  int outside = 0;
  int nplanes = this->GetNumberOfPlanes();

  for (i=0; i<nplanes; i++)
    {
    int row=i*4;

    double fx =
      vtkPlanesIntersection::EvaluatePlaneEquation(testv, this->Plane + row);

    if (fx > VTK_SMALL_DOUBLE)
      {
      outside = 1;
      break;
      }
    }
  return outside;
}
int vtkPlanesIntersection::Invert3x3(double M[3][3])
{
  int i, j;
  double temp[3][3];

  double det = vtkMath::Determinant3x3(M);

  if ( (det > -VTK_SMALL_DOUBLE) && (det < VTK_SMALL_DOUBLE)) return -1;

  vtkMath::Invert3x3(M, temp);

  for (i=0; i<3; i++)
    {
    for (j=0; j<3; j++)
      {
      M[i][j] = temp[i][j];
      }
    }

  return 0;
}

// Region / box intersection tests *******************************

int vtkPlanesIntersection::IntersectsBoundingBox(vtkPoints *R)
{
  double BoxBounds[6], RegionBounds[6];
  
  R->GetBounds(BoxBounds);

  this->regionPts->GetBounds(RegionBounds);

  if ((BoxBounds[1] <= RegionBounds[0]) ||
      (BoxBounds[0] >= RegionBounds[1]) ||
      (BoxBounds[3] <= RegionBounds[2]) ||
      (BoxBounds[2] >= RegionBounds[3]) ||
      (BoxBounds[5] <= RegionBounds[4]) ||
      (BoxBounds[4] >= RegionBounds[5])) 
    {
    return 0;
    }
  return 1;
}
int vtkPlanesIntersection::EnclosesBoundingBox(vtkPoints *R)
{
  double BoxBounds[6], RegionBounds[6];

  R->GetBounds(BoxBounds);

  this->regionPts->GetBounds(RegionBounds);

  if ((BoxBounds[0] > RegionBounds[0]) ||
      (BoxBounds[1] < RegionBounds[1]) ||
      (BoxBounds[2] > RegionBounds[2]) ||
      (BoxBounds[3] < RegionBounds[3]) ||
      (BoxBounds[4] > RegionBounds[4]) ||
      (BoxBounds[5] < RegionBounds[5])) 
    {
    return 0;
    }

  return 1;
}
int vtkPlanesIntersection::EvaluateFacePlane(int plane, vtkPoints *R)
{
  int i;
  double n[3], bounds[6]; 
  double withN[3], oppositeN[3];

  R->GetBounds(bounds);

  this->Normals->GetTuple(plane, n);

  // Find vertex of R most in direction of normal, and find
  //  oppposite vertex

  for (i=0; i<3; i++)
    {
    if (n[i] < 0)
      {
      withN[i]     = (double)bounds[i*2];
      oppositeN[i] = (double)bounds[i*2 + 1];
      } 
    else
      {
      withN[i]     = (double)bounds[i*2 + 1];
      oppositeN[i] = (double)bounds[i*2];
      }
    }
  
  // Determine whether R is in negative half plane ("inside" frustum),
  //    positive half plane, or whether it straddles the plane.
  //    The normal points in direction of positive half plane.

  double *p = this->Plane + (plane * 4);

  double negVal =

    vtkPlanesIntersection::EvaluatePlaneEquation(oppositeN, p);

  if (negVal >= 0)
    {
    return Outside;
    }

  double posVal =

    vtkPlanesIntersection::EvaluatePlaneEquation(withN, p);

  if (posVal <= 0)
    {
    return Inside;
    }

  else             return Straddle;
}
int vtkPlanesIntersection::IntersectsProjection(vtkPoints *R, int dir)
{
  int intersects = 0;

  switch (dir)
    {
    case Xdim:

      intersects = this->regionPts->RectangleIntersectionX(R);
      break;

    case Ydim:

      intersects = this->regionPts->RectangleIntersectionY(R);
      break;

    case Zdim:

      intersects = this->regionPts->RectangleIntersectionZ(R);
      break;
    }

  return intersects;
} 

void vtkPlanesIntersection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Plane: " << this->Plane << endl;
  os << indent << "regionPts: " << this->regionPts << endl;

  int i, npts;

  if (this->Points)
    {
    npts = this->Points->GetNumberOfPoints();

    for (i=0; i<npts; i++)
      {
      double *pt = this->Points->GetPoint(i);
      double *n = this->Normals->GetTuple(i);

      os << indent << "Origin " <<  pt[0] << " " << pt[1] << " " << pt[2] << " " ;

      os << indent << "Normal " <<  n[0] << " " << n[1] << " " << n[2] << endl;
      }
    }

  if (this->regionPts)
    {
    npts = this->regionPts->GetNumberOfPoints();

    for (i=0; i<npts; i++)
      {
      double *pt = this->regionPts->GetPoint(i);

      os << indent << "Vertex " <<  pt[0] << " " << pt[1] << " " << pt[2] << endl;
      }
    }
}
