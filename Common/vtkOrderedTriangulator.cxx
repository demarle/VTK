/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOrderedTriangulator.cxx
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
#include "vtkOrderedTriangulator.h"

#include "vtkCellArray.h"
#include "vtkEdgeTable.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkTetra.h"
#include "vtkUnstructuredGrid.h"
#include "vtkHeap.h"

#include <vtkstd/list>
#include <vtkstd/vector>
#include <vtkstd/stack>

vtkCxxRevisionMacro(vtkOrderedTriangulator, "1.49");
vtkStandardNewMacro(vtkOrderedTriangulator);

#ifdef _WIN32_WCE
 #ifndef __PLACEMENT_NEW_INLINE
 #define __PLACEMENT_NEW_INLINE
  inline void *__cdecl operator new(size_t, void *_P)
        {return (_P); }
  #if     _MSC_VER >= 1200
   inline void __cdecl operator delete(void *, void *)
   {return; }
  #endif
 #endif
#else
 #ifdef VTK_USE_ANSI_STDLIB
  #include <new>
 #else
  #include <new.h>
 #endif
#endif

// TO DO:
// + In place new to avoid new/delete
// + Delete tetra's directly rather than traversing the entire list
//   (in CreateInsertionCavity().
// + Clean up interface to classes

// Classes are used to represent points, faces, and tetras-------------------
//
struct vtkOTPoint;
struct vtkOTFace;
struct vtkOTTetra;

//---Class represents a face (and related typedefs)--------------------------
struct vtkOTPoint
{
  vtkOTPoint() : Id(0), SortId(0), SortId2(0), InternalId(0), Type(Inside) 
    {
      this->X[0] = this->X[1] = this->X[2] = 0.0;
      this->P[0] = this->P[1] = this->P[2] = 0.0;
    }
  enum PointClassification 
    {Inside=0,Outside=1,Boundary=2,Added=3,NoInsert=4};
  vtkIdType Id; //Id to data outside this class

  //Id used to sort in triangulation
  vtkIdType SortId;
  //Second id used to sort in triangulation
  //This can be used in situations where on id is not enough
  //(for example, when the id is related to an edge which
  // is described by two points)
  vtkIdType SortId2; 

  vtkIdType InternalId; //Id (order) of point insertion
  double X[3];
  double P[3];
  PointClassification Type; //inside, outside
};
struct PointListType : public vtkstd::vector<vtkOTPoint>
{
  PointListType() : vtkstd::vector<vtkOTPoint>() {}
  vtkOTPoint* GetPointer(int ptId)
    {return &( *(this->begin()+ptId) ); }
};
typedef PointListType::iterator PointListIterator;

//---Class represents a face (and related typedefs)--------------------------
struct vtkOTFace //used during tetra construction
{
  vtkOTPoint *Points[3]; //the three points of the face
  vtkOTTetra *Neighbor;
  double     Normal[3];
  double     N2;
  void ComputePsuedoNormal()
    {
      double v20[3], v10[3];
      v20[0] = this->Points[2]->P[0] - this->Points[0]->P[0];
      v20[1] = this->Points[2]->P[1] - this->Points[0]->P[1];
      v20[2] = this->Points[2]->P[2] - this->Points[0]->P[2];
      v10[0] = this->Points[1]->P[0] - this->Points[0]->P[0];
      v10[1] = this->Points[1]->P[1] - this->Points[0]->P[1];
      v10[2] = this->Points[1]->P[2] - this->Points[0]->P[2];
      vtkMath::Cross(v10,v20,this->Normal);
      this->N2 = vtkMath::Dot(this->Normal,this->Normal);
    }
  int IsValidCavityFace(double X[3],double tol2)
    {
      double vp[3], d;
      vp[0] = X[0] - this->Points[0]->P[0];
      vp[1] = X[1] - this->Points[0]->P[1];
      vp[2] = X[2] - this->Points[0]->P[2];
      d = vtkMath::Dot(vp,this->Normal);
      return ( (d > 0.0L && (d*d) > (tol2*this->N2)) ? 1 : 0 );
    }
};
typedef vtkstd::vector<vtkOTFace>            FaceListType;
typedef vtkstd::vector<vtkOTFace>::iterator  FaceListIterator;

//---Class represents a tetrahedron (and related typedefs)--------------------
typedef vtkstd::list<vtkOTTetra*>             TetraListType;
typedef vtkstd::list<vtkOTTetra*>::iterator   TetraListIterator;
struct TetraStackType : public vtkstd::stack<vtkOTTetra*>
{
  TetraStackType() : vtkstd::stack<vtkOTTetra*>() {}
  void clear() {while (!this->empty()) this->pop();}
};
typedef vtkstd::vector<vtkOTTetra*>           TetraQueueType;
typedef vtkstd::vector<vtkOTTetra*>::iterator TetraQueueIterator;

struct vtkOTTetra
{
  void *operator new(size_t size, vtkHeap *heap)
    {return heap->AllocateMemory(size);}
  void operator delete(void*,vtkHeap*) {}
  vtkOTTetra() : Radius2(0.0L), CurrentPointId(-1), 
    Type(OutsideCavity), ListIterator(0) 
    {
    this->Center[0] = this->Center[1] = this->Center[2] = 0.0L;
    this->Points[0] = this->Points[1] = this->Points[2] = this->Points[3] = 0;
    this->Neighbors[0] = this->Neighbors[1] = 
      this->Neighbors[2] = this->Neighbors[3] = 0;
    }

  double Radius2; //center and radius squared of circumsphere of this tetra
  double Center[3];
  // Note: there is a direct correlation between the points and the faces
  // i.e., the ordering of the points and face neighbors.
  vtkOTTetra *Neighbors[4]; //the four face neighbors
  vtkOTPoint *Points[4]; //the four points
  // These are used during point insertion
  int CurrentPointId;
  enum TetraClassification 
    {Inside=0,Outside=1,All=2,InCavity=3,OutsideCavity=4};
  TetraClassification Type;

  void GetFacePoints(int i, vtkOTFace& face);
  int InCircumSphere(double x[3]);
  TetraClassification GetType(); //inside, outside
  TetraListIterator ListIterator; //points to the list of tetras
};

//---Class represents the Delaunay triangulation using points, faces,
//---and tetras.
struct vtkOTMesh
{
  vtkOTMesh() 
    {
      this->EdgeTable = vtkEdgeTable::New();
    }
  ~vtkOTMesh() 
    {
      this->EdgeTable->Delete();
    }
  
  PointListType   Points;
  TetraListType   Tetras;
  FaceListType    CavityFaces;
  TetraQueueType  CavityTetras;
  TetraStackType  TetraStack;
  TetraQueueType  DegenerateQueue;
  vtkEdgeTable   *EdgeTable;
  double          Tolerance2;
  
  void Reset()
    {
      this->Points.clear();
      this->Tetras.clear();
      this->CavityFaces.clear();
      this->CavityTetras.clear();
      this->TetraStack.clear();
      this->DegenerateQueue.clear();
      this->EdgeTable->Reset();
    }

  vtkOTTetra *CreateTetra(vtkHeap *heap, vtkOTPoint *p, vtkOTFace *face);
  vtkOTTetra *WalkToTetra(vtkOTTetra *t,double x[3],int depth,double bc[4]);
  int CreateInsertionCavity(vtkOTPoint* p, vtkOTTetra *tetra, double bc[4]);
  void DumpInsertionCavity(double x[3]);
};

//------------------------------------------------------------------------
vtkOrderedTriangulator::vtkOrderedTriangulator()
{
  this->Mesh = new vtkOTMesh;
  this->NumberOfPoints = 0;
  this->PreSorted = 0;
  this->UseTwoSortIds = 0;

  // In place news (using allocators) are done here
  this->Heap = vtkHeap::New();
  this->Heap->SetBlockSize(500000);
}

//------------------------------------------------------------------------
vtkOrderedTriangulator::~vtkOrderedTriangulator()
{
  delete this->Mesh;
  this->Heap->Delete();
}

//------------------------------------------------------------------------
void vtkOrderedTriangulator::InitTriangulation(float xmin, float xmax,
                                               float ymin, float ymax,
                                               float zmin, float zmax,
                                               int numPts)
{
  float bounds[6];
  bounds[0] = xmin;
  bounds[1] = xmax;
  bounds[2] = ymin;
  bounds[3] = ymax;
  bounds[4] = zmin;
  bounds[5] = zmax;
  
  this->InitTriangulation(bounds,numPts);
}

//------------------------------------------------------------------------
void vtkOrderedTriangulator::InitTriangulation(float bounds[6], int numPts)
{
  double length;
  double center[3];
  double radius2;

  // Set up the internal data structures. Space for six extra points
  // is allocated for the bounding triangulation.
  this->NumberOfPoints = 0;
  this->MaximumNumberOfPoints = numPts;
  this->Mesh->Reset();
  this->Mesh->Points.reserve(numPts+6);
  this->Heap->Reset();
  
  // Create the initial Delaunay triangulation which is a
  // bounding octahedron: 6 points & 4 tetra.
  center[0] = (double) (bounds[0]+bounds[1])/2.0;
  center[1] = (double) (bounds[2]+bounds[3])/2.0;
  center[2] = (double) (bounds[4]+bounds[5])/2.0;
  length = 2.0 * sqrt( (radius2 = (bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                 (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                 (bounds[5]-bounds[4])*(bounds[5]-bounds[4])) );
  radius2 /= 2.0;
  this->Mesh->Tolerance2 = length*length*1.0e-10;
  
  //Define the points (-x,+x,-y,+y,-z,+z)
  this->Mesh->Points[numPts].P[0] = center[0] - length;
  this->Mesh->Points[numPts].P[1] = center[1];
  this->Mesh->Points[numPts].P[2] = center[2];
  this->Mesh->Points[numPts].Id = numPts;
  this->Mesh->Points[numPts].InternalId = numPts;
  this->Mesh->Points[numPts].Type = vtkOTPoint::Added;

  this->Mesh->Points[numPts+1].P[0] = center[0] + length;
  this->Mesh->Points[numPts+1].P[1] = center[1];
  this->Mesh->Points[numPts+1].P[2] = center[2];
  this->Mesh->Points[numPts+1].Id = numPts + 1;
  this->Mesh->Points[numPts+1].InternalId = numPts + 1;
  this->Mesh->Points[numPts+1].Type = vtkOTPoint::Added;

  this->Mesh->Points[numPts+2].P[0] = center[0];              
  this->Mesh->Points[numPts+2].P[1] = center[1] - length;
  this->Mesh->Points[numPts+2].P[2] = center[2];
  this->Mesh->Points[numPts+2].Id = numPts + 2;
  this->Mesh->Points[numPts+2].InternalId = numPts + 2;
  this->Mesh->Points[numPts+2].Type = vtkOTPoint::Added;

  this->Mesh->Points[numPts+3].P[0] = center[0];              
  this->Mesh->Points[numPts+3].P[1] = center[1] + length;
  this->Mesh->Points[numPts+3].P[2] = center[2];
  this->Mesh->Points[numPts+3].Id = numPts + 3;
  this->Mesh->Points[numPts+3].InternalId = numPts + 3;
  this->Mesh->Points[numPts+3].Type = vtkOTPoint::Added;

  this->Mesh->Points[numPts+4].P[0] = center[0];              
  this->Mesh->Points[numPts+4].P[1] = center[1];
  this->Mesh->Points[numPts+4].P[2] = center[2] - length;
  this->Mesh->Points[numPts+4].Id = numPts + 4;
  this->Mesh->Points[numPts+4].InternalId = numPts + 4;
  this->Mesh->Points[numPts+4].Type = vtkOTPoint::Added;

  this->Mesh->Points[numPts+5].P[0] = center[0];              
  this->Mesh->Points[numPts+5].P[1] = center[1];
  this->Mesh->Points[numPts+5].P[2] = center[2] + length;
  this->Mesh->Points[numPts+5].Id = numPts + 5;
  this->Mesh->Points[numPts+5].InternalId = numPts + 5;
  this->Mesh->Points[numPts+5].Type = vtkOTPoint::Added;

  // Create bounding tetras (there are four) as well as the associated faces
  // They all share the same center and radius
  vtkOTTetra *tetras[4];
  for (int i=0; i<4; ++i)
    {
    tetras[i] = new(this->Heap) vtkOTTetra();
    this->Mesh->Tetras.push_front(tetras[i]);
    tetras[i]->ListIterator = this->Mesh->Tetras.begin();
    tetras[i]->Center[0] = center[0];
    tetras[i]->Center[1] = center[1];
    tetras[i]->Center[2] = center[2];
    tetras[i]->Radius2 = radius2;
    }

  //Okay now set up the points and neighbors in the tetras
  tetras[0]->Points[0] = this->Mesh->Points.GetPointer(numPts + 0);
  tetras[0]->Points[1] = this->Mesh->Points.GetPointer(numPts + 2);
  tetras[0]->Points[2] = this->Mesh->Points.GetPointer(numPts + 4);
  tetras[0]->Points[3] = this->Mesh->Points.GetPointer(numPts + 5);
  tetras[0]->Neighbors[0] = 0; //outside
  tetras[0]->Neighbors[1] = tetras[1];
  tetras[0]->Neighbors[2] = tetras[3];
  tetras[0]->Neighbors[3] = 0;
  
  tetras[1]->Points[0] = this->Mesh->Points.GetPointer(numPts + 2);
  tetras[1]->Points[1] = this->Mesh->Points.GetPointer(numPts + 1);
  tetras[1]->Points[2] = this->Mesh->Points.GetPointer(numPts + 4);
  tetras[1]->Points[3] = this->Mesh->Points.GetPointer(numPts + 5);
  tetras[1]->Neighbors[0] = 0;
  tetras[1]->Neighbors[1] = tetras[2];
  tetras[1]->Neighbors[2] = tetras[0];
  tetras[1]->Neighbors[3] = 0;
  
  tetras[2]->Points[0] = this->Mesh->Points.GetPointer(numPts + 1);
  tetras[2]->Points[1] = this->Mesh->Points.GetPointer(numPts + 3);
  tetras[2]->Points[2] = this->Mesh->Points.GetPointer(numPts + 4);
  tetras[2]->Points[3] = this->Mesh->Points.GetPointer(numPts + 5);
  tetras[2]->Neighbors[0] = 0;
  tetras[2]->Neighbors[1] = tetras[3];
  tetras[2]->Neighbors[2] = tetras[1];
  tetras[2]->Neighbors[3] = 0;
  
  tetras[3]->Points[0] = this->Mesh->Points.GetPointer(numPts + 3);
  tetras[3]->Points[1] = this->Mesh->Points.GetPointer(numPts + 0);
  tetras[3]->Points[2] = this->Mesh->Points.GetPointer(numPts + 4);
  tetras[3]->Points[3] = this->Mesh->Points.GetPointer(numPts + 5);
  tetras[3]->Neighbors[0] = 0;
  tetras[3]->Neighbors[1] = tetras[0];
  tetras[3]->Neighbors[2] = tetras[2];
  tetras[3]->Neighbors[3] = 0;
}


//------------------------------------------------------------------------
vtkIdType vtkOrderedTriangulator::InsertPoint(vtkIdType id, float x[3],
                                              float p[3], int type)
{
  vtkIdType idx = this->NumberOfPoints++;
  if ( idx > this->MaximumNumberOfPoints )
    {
    vtkErrorMacro(<< "Trying to insert more points than specified");
    return idx;
    }
  
  this->Mesh->Points[idx].Id = id;
  this->Mesh->Points[idx].SortId = id;
  this->Mesh->Points[idx].SortId2 = -1;
  this->Mesh->Points[idx].InternalId = -1; //dummy value
  this->Mesh->Points[idx].X[0] = (double) x[0];
  this->Mesh->Points[idx].X[1] = (double) x[1];
  this->Mesh->Points[idx].X[2] = (double) x[2];
  this->Mesh->Points[idx].P[0] = (double) p[0];
  this->Mesh->Points[idx].P[1] = (double) p[1];
  this->Mesh->Points[idx].P[2] = (double) p[2];
  this->Mesh->Points[idx].Type = (vtkOTPoint::PointClassification) type;
  
  return idx;
}

vtkIdType vtkOrderedTriangulator::InsertPoint(vtkIdType id, vtkIdType sortid,
                                              float x[3], float p[3], int type)
{
  vtkIdType idx = this->NumberOfPoints++;
  if ( idx > this->MaximumNumberOfPoints )
    {
    vtkErrorMacro(<< "Trying to insert more points than specified");
    return idx;
    }
  
  this->Mesh->Points[idx].Id = id;
  this->Mesh->Points[idx].SortId = sortid;
  this->Mesh->Points[idx].SortId2 = -1;
  this->Mesh->Points[idx].InternalId = -1; //dummy value
  this->Mesh->Points[idx].X[0] = (double) x[0];
  this->Mesh->Points[idx].X[1] = (double) x[1];
  this->Mesh->Points[idx].X[2] = (double) x[2];
  this->Mesh->Points[idx].P[0] = (double) p[0];
  this->Mesh->Points[idx].P[1] = (double) p[1];
  this->Mesh->Points[idx].P[2] = (double) p[2];
  this->Mesh->Points[idx].Type = (vtkOTPoint::PointClassification) type;
  
  return idx;
}

vtkIdType vtkOrderedTriangulator::InsertPoint(vtkIdType id, vtkIdType sortid,
                                              vtkIdType sortid2,
                                              float x[3], float p[3], int type)
{
  vtkIdType idx = this->NumberOfPoints++;
  if ( idx > this->MaximumNumberOfPoints )
    {
    vtkErrorMacro(<< "Trying to insert more points than specified");
    return idx;
    }
  
  this->Mesh->Points[idx].Id = id;
  this->Mesh->Points[idx].SortId = sortid;
  this->Mesh->Points[idx].SortId2 = sortid2;
  this->Mesh->Points[idx].InternalId = -1; //dummy value
  this->Mesh->Points[idx].X[0] = (double) x[0];
  this->Mesh->Points[idx].X[1] = (double) x[1];
  this->Mesh->Points[idx].X[2] = (double) x[2];
  this->Mesh->Points[idx].P[0] = (double) p[0];
  this->Mesh->Points[idx].P[1] = (double) p[1];
  this->Mesh->Points[idx].P[2] = (double) p[2];
  this->Mesh->Points[idx].Type = (vtkOTPoint::PointClassification) type;
  
  return idx;
}

//------------------------------------------------------------------------
void vtkOrderedTriangulator::UpdatePointType(vtkIdType internalId, int type)
{
  this->Mesh->Points[internalId].Type = (vtkOTPoint::PointClassification) type;
}

//------------------------------------------------------------------------
void vtkOTTetra::GetFacePoints(int i, vtkOTFace& face)
{
  // the order is carefully choosen to produce a tetrahedron
  // that is not inside out; i.e., the ordering produces a positive 
  // jacobian (computed from first three points points to fourth).
  switch (i)
    {
    case 0:
      face.Points[0] = this->Points[0];
      face.Points[1] = this->Points[3];
      face.Points[2] = this->Points[1];
      break;
    case 1:
      face.Points[0] = this->Points[1];
      face.Points[1] = this->Points[3];
      face.Points[2] = this->Points[2];
      break;
    case 2:
      face.Points[0] = this->Points[0];
      face.Points[1] = this->Points[2];
      face.Points[2] = this->Points[3];
      break;
    case 3:
      face.Points[0] = this->Points[0];
      face.Points[1] = this->Points[1];
      face.Points[2] = this->Points[2];
      break;
    }
  face.ComputePsuedoNormal();
}

//------------------------------------------------------------------------

extern "C" {
#ifdef _WIN32_WCE
  int __cdecl vtkSortOnIds(const void *val1, const void *val2)
#else
  int vtkSortOnIds(const void *val1, const void *val2)
#endif
  {
    if (((vtkOTPoint *)val1)->SortId < ((vtkOTPoint *)val2)->SortId)
      {
      return (-1);
      }
    else if (((vtkOTPoint *)val1)->SortId > ((vtkOTPoint *)val2)->SortId)
      {
      return (1);
      }
    else
      {
      return (0);
      }
  }
}

extern "C" {
#ifdef _WIN32_WCE
  int __cdecl vtkSortOnTwoIds(const void *val1, const void *val2)
#else
  int vtkSortOnTwoIds(const void *val1, const void *val2)
#endif
  {
    if (((vtkOTPoint *)val1)->SortId2 < ((vtkOTPoint *)val2)->SortId2)
      {
      return (-1);
      }
    else if (((vtkOTPoint *)val1)->SortId2 > ((vtkOTPoint *)val2)->SortId2)
      {
      return (1);
      }
    
    if (((vtkOTPoint *)val1)->SortId < ((vtkOTPoint *)val2)->SortId)
      {
      return (-1);
      }
    else if (((vtkOTPoint *)val1)->SortId > ((vtkOTPoint *)val2)->SortId)
      {
      return (1);
      }
    else
      {
      return (0);
      }
  }
}

//------------------------------------------------------------------------
// See whether point is in sphere of tetrahedron
int vtkOTTetra::InCircumSphere(double x[3])
{
  double dist2;
  
  // check if inside/outside circumsphere
  dist2 = (x[0] - this->Center[0]) * (x[0] - this->Center[0]) + 
          (x[1] - this->Center[1]) * (x[1] - this->Center[1]) +
          (x[2] - this->Center[2]) * (x[2] - this->Center[2]);

  return (dist2 < (0.9999L * this->Radius2) ? 1 : 0);
}

//------------------------------------------------------------------------
// Get type based on point types
inline vtkOTTetra::TetraClassification vtkOTTetra::GetType()
{
  if ( (this->Points[0]->Type == vtkOTPoint::Inside || 
        this->Points[0]->Type == vtkOTPoint::Boundary ) &&
       (this->Points[1]->Type == vtkOTPoint::Inside || 
        this->Points[1]->Type == vtkOTPoint::Boundary ) &&
       (this->Points[2]->Type == vtkOTPoint::Inside || 
        this->Points[2]->Type == vtkOTPoint::Boundary ) &&
       (this->Points[3]->Type == vtkOTPoint::Inside || 
        this->Points[3]->Type == vtkOTPoint::Boundary ) )
    {
    return vtkOTTetra::Inside;
    }
  else
    {
    return vtkOTTetra::Outside;
    }
}

inline static int IsAPoint(vtkOTTetra *t, vtkIdType id)
{
  if ( id == t->Points[0]->InternalId || id == t->Points[1]->InternalId ||
       id == t->Points[2]->InternalId || id == t->Points[3]->InternalId )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

static void AssignNeighbors(vtkOTTetra* t1, vtkOTTetra* t2)
{
  static int CASE_MASK[4] = {1,2,4,8};
  int i, index;

  for (i=0, index=0; i<4; ++i)
    {
    if (IsAPoint(t2,t1->Points[i]->InternalId) )
      {
      index |= CASE_MASK[i];
      }
    }
  switch (index)
    {
    case 11:
      t1->Neighbors[0] = t2;
      break;
    case 14:
      t1->Neighbors[1] = t2;
      break;
    case 13:
      t1->Neighbors[2] = t2;
      break;
    case 7:
      t1->Neighbors[3] = t2;
      break;
    default:
      vtkGenericWarningMacro(<<"Really bad");
    }

  for (i=0, index=0; i<4; ++i)
    {
    if (IsAPoint(t1,t2->Points[i]->InternalId) )
      {
      index |= CASE_MASK[i];
      }
    }
  switch (index)
    {
    case 11:
      t2->Neighbors[0] = t1;
      break;
    case 14:
      t2->Neighbors[1] = t1;
      break;
    case 13:
      t2->Neighbors[2] = t1;
      break;
    case 7:
      t2->Neighbors[3] = t1;
      break;
    default:
      vtkGenericWarningMacro(<<"Really bad");
    }
}

vtkOTTetra *vtkOTMesh::CreateTetra(vtkHeap *heap, 
                                   vtkOTPoint *p, vtkOTFace *face)
{
  vtkOTTetra *tetra = new(heap) vtkOTTetra;
  this->Tetras.push_front(tetra);
  tetra->ListIterator = this->Tetras.begin();
  tetra->Radius2 = vtkTetra::Circumsphere(p->P,
                                          face->Points[0]->P,
                                          face->Points[1]->P,
                                          face->Points[2]->P,
                                          tetra->Center);

  // the order is carefully choosen to produce a tetrahedron
  // that is not inside out; i.e., the ordering produces a positive 
  // jacobian (normal computed from first three points points to fourth).
  tetra->Points[0] = face->Points[0];
  tetra->Points[1] = face->Points[1];
  tetra->Points[2] = face->Points[2];
  tetra->Points[3] = p;
  
  if ( face->Neighbor )
    {
    AssignNeighbors(tetra,face->Neighbor);
    }

  return tetra;
}

//------------------------------------------------------------------------
// We start with a point that is inside a tetrahedron. We find face
// neighbors of the tetrahedron that also contain the point. The
// process continues recursively until no more tetrahedron are found.
// Faces that lie between a tetrahedron that is in the cavity and one
// that is not form the cavity boundary, these are kept track of in
// a list. Eventually the point and boundary faces form new tetrahedra.
int vtkOTMesh::CreateInsertionCavity(vtkOTPoint* p, vtkOTTetra *initialTet, 
                                     double [4])
{
  // Prepare to insert deleted tetras and cavity faces
  //
  this->CavityFaces.clear(); //cavity face boundary
  this->CavityTetras.clear(); //cavity tetra
  this->TetraStack.clear(); //queue of tetras being processed
  this->DegenerateQueue.clear(); //queue of tetras that have degenerate faces
  this->TetraStack.push(initialTet);
  initialTet->Type = vtkOTTetra::InCavity; //the seed of the cavity
  initialTet->CurrentPointId = p->InternalId; //mark visited
  
  // Process queue of tetras until exhausted
  //
  int i, valid;
  int somethingNotValid=0;
  vtkOTFace face;
  vtkOTTetra *nei, *tetra;
  TetraQueueIterator t;
  for ( int numCycles=0; !this->TetraStack.empty(); numCycles++)
    {
    tetra = this->TetraStack.top();
    this->TetraStack.pop();
    this->CavityTetras.push_back(tetra);
    
    //for each face, see whether the neighbors are in the cavity
    for (valid=1, i=0; i<4 && valid; ++i)
      {
      nei = tetra->Neighbors[i];
      // If a mesh boundary face, the face is added to the 
      // list of insertion cavity faces
      if ( nei == 0 )
        {
        tetra->GetFacePoints(i,face);
        face.Neighbor = 0;
        this->CavityFaces.push_back(face);
        valid = face.IsValidCavityFace(p->P,this->Tolerance2);
        }
      // Neighbor tetra has not been visited, check for possible face boundary
      else if ( nei->CurrentPointId != p->InternalId )
        {
        nei->CurrentPointId = p->InternalId; //mark visited
        if ( nei->InCircumSphere(p->P) )
          {
          nei->Type = vtkOTTetra::InCavity;
          this->TetraStack.push(nei);
          }
        else //a cavity boundary
          {
          nei->Type = vtkOTTetra::OutsideCavity;
          tetra->GetFacePoints(i,face);
          face.Neighbor = nei;
          this->CavityFaces.push_back(face);
          valid = face.IsValidCavityFace(p->P,this->Tolerance2);
          }
        }//if a not-visited face neighbor
      // Visited before, add this face as a boundary
      else if ( nei->Type == vtkOTTetra::OutsideCavity )
        {
        tetra->GetFacePoints(i,face);
        face.Neighbor = nei;
        this->CavityFaces.push_back(face);
        valid = face.IsValidCavityFace(p->P,this->Tolerance2);
        }
      }//for each of the four tetra faces

    //check for validity
    if ( !valid ) //broke out due to invalid face
      {
      somethingNotValid++;
      //add this tetra to queue
      this->DegenerateQueue.push_back(tetra);
      
      //mark all current tetras unvisited
      for (t = this->CavityTetras.begin(); t != this->CavityTetras.end(); ++t)
        {
        (*t)->CurrentPointId = -1;
        }

      //mark degenerate tetras visited and outside cavity
      TetraQueueIterator titer;
      for ( titer=this->DegenerateQueue.begin(); 
            titer != this->DegenerateQueue.end(); ++titer)
        {
        (*titer)->CurrentPointId = p->InternalId;
        (*titer)->Type = vtkOTTetra::OutsideCavity;
        }

      //reinitialize queue
      this->CavityFaces.clear();  //cavity face boundary
      this->CavityTetras.clear(); //tetras forming cavity
      this->TetraStack.clear();   //reprocess
      this->TetraStack.push(initialTet);
      initialTet->CurrentPointId = p->InternalId;
      initialTet->Type = vtkOTTetra::InCavity;
      }
    if ( numCycles > 1000 ) return 0;
    }//while queue not empty
  
  // Make final pass and delete tetras in the cavity
  for (t = this->CavityTetras.begin(); t != this->CavityTetras.end(); ++t)
    {
    this->Tetras.erase((*t)->ListIterator);
    }

#if 0
  //please leave this for debugging purposes
  if ( somethingNotValid )
    {
    this->DumpInsertionCavity(p->P);
//    exit(1);
    }
#endif

  return 1;
}

void vtkOTMesh::DumpInsertionCavity(double x[3])
{
  FaceListIterator fptr;

  cout << "# vtk DataFile Version 3.0\n";
  cout << "ordered triangulator output\n";
  cout << "ASCII\n";
  cout << "DATASET POLYDATA\n";

  //write out points
  int numFaces = this->CavityFaces.size();
  cout << "POINTS " << 3*numFaces+1 << " float\n";
  
  for (fptr=this->CavityFaces.begin(); 
       fptr != this->CavityFaces.end(); ++fptr)
    {
    cout << fptr->Points[0]->P[0] << " "
         << fptr->Points[0]->P[1] << " "
         << fptr->Points[0]->P[2] << " "
         << fptr->Points[1]->P[0] << " "
         << fptr->Points[1]->P[1] << " "
         << fptr->Points[1]->P[2] << " "
         << fptr->Points[2]->P[0] << " "
         << fptr->Points[2]->P[1] << " "
         << fptr->Points[2]->P[2] << "\n";
    }

  //write out point insertion vertex
  cout << x[0] << " " << x[1] << " " << x[2] << "\n\n";
  cout << "VERTICES 1 2 \n";
  cout << "1 " << 3*numFaces << "\n\n";
  
  //write out triangles
  cout << "POLYGONS " << numFaces << " " <<4*numFaces << "\n";
  
  int idx=0;
  for (fptr=this->CavityFaces.begin(); 
       fptr != this->CavityFaces.end(); ++fptr, idx+=3)
    {
    cout << 3 << " " << idx << " " << idx+1 << " " << idx+2 << "\n";
    }
}

// Walk to the tetra tha contains this point. Walking is done by moving
// in the direction of the most negative barycentric coordinate (i.e.,
// into the face neighbor).
vtkOTTetra*
vtkOTMesh::WalkToTetra(vtkOTTetra *tetra, double x[3], int depth, double bc[4])
{
  int neg = 0;
  int j, numNeg;
  double negValue;
  
  // prevent aimless wandering and death by recursion
  if ( depth > 200 )
    {
    return 0;
    }

  vtkTetra::BarycentricCoords(x, tetra->Points[0]->P, tetra->Points[1]->P, 
                              tetra->Points[2]->P, tetra->Points[3]->P, bc);

  // find the most negative face
  for ( negValue=VTK_LARGE_FLOAT, numNeg=j=0; j<4; j++ )
    {
    if ( bc[j] < -0.000001 ) //if close enough that's okay
      {
      numNeg++;
      if ( bc[j] < negValue )
        {
        negValue = bc[j];
        neg = j;
        }
      }
    }

  // if no negatives, then inside this tetra
  if ( numNeg <= 0 )
    {
    return tetra;
    }
  
  // okay, march towards the most negative direction
  switch (neg) 
    {
    case 0:
      tetra = tetra->Neighbors[1];
      break;
    case 1:
      tetra = tetra->Neighbors[2];
      break;
    case 2:
      tetra = tetra->Neighbors[0];
      break;
    case 3:
      tetra = tetra->Neighbors[3];
      break;
    }

  if ( tetra )
    {
    return this->WalkToTetra(tetra, x, ++depth, bc);
    }
  else
    {
    return 0;
    }
}
  
//------------------------------------------------------------------------
// Use an ordered insertion process in combination with a consistent
// degenerate resolution process to generate a unique Delaunay triangulation.
void vtkOrderedTriangulator::Triangulate()
{
  vtkOTPoint *p;
  int i;
  vtkIdType ptId;
  
  // Sort the points according to id. The last six points are left
  // where they are (at the end of the list).
  if ( ! this->PreSorted )
    {
    if (this->UseTwoSortIds)
      {
      qsort((void *)this->Mesh->Points.GetPointer(0), this->NumberOfPoints, 
            sizeof(vtkOTPoint), vtkSortOnTwoIds);
      }
    else
      {
      qsort((void *)this->Mesh->Points.GetPointer(0), this->NumberOfPoints, 
            sizeof(vtkOTPoint), vtkSortOnIds);
      }
    }

  // Insert each point into the triangulation. Assign internal ids 
  // as we progress.
  for (ptId=0, p=this->Mesh->Points.GetPointer(0); 
       ptId < this->NumberOfPoints; ++p, ++ptId)
    {
    if ( p->Type == vtkOTPoint::NoInsert )
      {
      continue; //skip this point
      }
      
    p->InternalId = ptId;

    // Walk to a tetrahedron (start with first one on list)
    double bc[4];
    vtkOTTetra *tetra = 
      this->Mesh->WalkToTetra(*(this->Mesh->Tetras.begin()),p->P,0,bc);

    if ( tetra == 0 || !this->Mesh->CreateInsertionCavity(p, tetra, bc) )
      {
      vtkDebugMacro(<<"Point not in tetrahedron");
      continue;
      }

    // For each face on the boundary of the cavity, create a new 
    // tetrahedron with the face and point. We've also got to set
    // up tetrahedron face neighbors, so we'll use an edge table
    // to keep track of the tetrahedron that generated the face as
    // a result of sweeping an edge.
    vtkIdType v1, v2;

    this->Mesh->EdgeTable->InitEdgeInsertion(this->MaximumNumberOfPoints+6,2);
    this->Mesh->TetraStack.clear();
    FaceListIterator fptr;
    void *tptr;
    vtkOTTetra *neiTetra;
    
    for (fptr=this->Mesh->CavityFaces.begin(); 
         fptr != this->Mesh->CavityFaces.end(); ++fptr)
      {
      //create a tetra (it's added to the list of tetras as a side effect)
      tetra = this->Mesh->CreateTetra(this->Heap,p,&(*fptr));

      for (i=0; i<3; ++i)
        {
        v1 = fptr->Points[i%3]->InternalId;
        v2 = fptr->Points[(i+1)%3]->InternalId;
        this->Mesh->EdgeTable->IsEdge(v1,v2,tptr);
        if ( ! tptr )
          {
          this->Mesh->EdgeTable->InsertEdge(v1,v2,tetra);
          }
        else
          {
          neiTetra = static_cast<vtkOTTetra*>(tptr);
          AssignNeighbors(tetra, neiTetra);
          }
        }//for three edges
      }//for each face on the insertion cavity
    }//for all points to be inserted
}


vtkIdType vtkOrderedTriangulator::GetTetras(int classification, 
                                            vtkUnstructuredGrid *ugrid)
{
  // Create the points
  //
  vtkIdType numTetras=0;
  PointListIterator p;
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(this->MaximumNumberOfPoints+6);
  for ( p=this->Mesh->Points.begin(); p != this->Mesh->Points.end(); ++p)
    {
    points->SetPoint(p->InternalId,p->X);
    }
  ugrid->SetPoints(points);
  
  ugrid->Allocate(1000);
  TetraListIterator t;
  vtkOTTetra *tetra;
  vtkOTTetra::TetraClassification type; //inside, outside

  // loop over all tetras getting the ones with the classification requested
  vtkIdType pts[4];
  for (t=this->Mesh->Tetras.begin(); t != this->Mesh->Tetras.end(); ++t)
    {
    tetra = *t;
    type = tetra->GetType();

    if ( type == classification || classification == vtkOTTetra::All)
      {
      numTetras++;
      pts[0] = tetra->Points[0]->InternalId;
      pts[1] = tetra->Points[1]->InternalId;
      pts[2] = tetra->Points[2]->InternalId;
      pts[3] = tetra->Points[3]->InternalId;
      ugrid->InsertNextCell(VTK_TETRA,4,pts);
      }
    }//for all tetras
  
  return numTetras;
}

//------------------------------------------------------------------------
vtkIdType vtkOrderedTriangulator::AddTetras(int classification, 
                                            vtkCellArray *outConnectivity)
{
  TetraListIterator t;
  vtkOTTetra *tetra;
  vtkOTTetra::TetraClassification type; //inside, outside
  vtkIdType numTetras=0;

  // loop over all tetras getting the ones with the classification requested
  for (t=this->Mesh->Tetras.begin(); t != this->Mesh->Tetras.end(); ++t)
    {
    tetra = *t;
    type = tetra->GetType();

    if ( type == classification || classification == vtkOTTetra::All)
      {
      numTetras++;
      outConnectivity->InsertNextCell(4);
      outConnectivity->InsertCellPoint(tetra->Points[0]->Id);
      outConnectivity->InsertCellPoint(tetra->Points[1]->Id);
      outConnectivity->InsertCellPoint(tetra->Points[2]->Id);
      outConnectivity->InsertCellPoint(tetra->Points[3]->Id);
      }
    }//for all tetras

  return numTetras;
}

vtkIdType vtkOrderedTriangulator::AddTetras(int classification, 
                                            vtkIdList *ptIds, 
                                            vtkPoints *pts)
{
  TetraListIterator t;
  vtkOTTetra *tetra;
  vtkOTTetra::TetraClassification type; //inside, outside
  vtkIdType numTetras=0;
  int i;

  // loop over all tetras getting the ones with the classification requested
  for (t=this->Mesh->Tetras.begin(); t != this->Mesh->Tetras.end(); ++t)
    {
    tetra = *t;
    type = tetra->GetType();

    if ( type == classification || classification == vtkOTTetra::All)
      {
      numTetras++;
      for (i=0; i<4; i++)
        {
        ptIds->InsertNextId(tetra->Points[i]->Id);
        pts->InsertNextPoint(tetra->Points[i]->X);
        }
      }
    }//for all tetras

  return numTetras;
}


vtkIdType vtkOrderedTriangulator::AddTetras(int classification, 
                                            vtkUnstructuredGrid *ugrid)

{
  vtkIdType numTetras=0;
  TetraListIterator t;
  vtkOTTetra *tetra;
  vtkOTTetra::TetraClassification type; //inside, outside

  // loop over all tetras getting the ones with the classification requested
  vtkIdType pts[4];
  for (t=this->Mesh->Tetras.begin(); t != this->Mesh->Tetras.end(); ++t)
    {
    tetra = *t;
    type = tetra->GetType();
    
    if ( type == classification || classification == vtkOTTetra::All)
      {
      numTetras++;
      pts[0] = tetra->Points[0]->Id;
      pts[1] = tetra->Points[1]->Id;
      pts[2] = tetra->Points[2]->Id;
      pts[3] = tetra->Points[3]->Id;
      ugrid->InsertNextCell(VTK_TETRA,4,pts);
      }
    }//for all tetras
  
  return numTetras;
}

vtkIdType vtkOrderedTriangulator::AddTriangles(vtkCellArray *tris)
{
  vtkIdType numTris=0;
  int i;

  // Loop over all tetras examining each unvisited face. Faces whose
  // points are all classified "boundary" are added to the list of
  // faces.
  TetraListIterator t;
  vtkOTTetra *tetra;
  vtkOTFace face;

  // loop over all tetras getting the faces classified on the boundary
  for (t=this->Mesh->Tetras.begin(); t != this->Mesh->Tetras.end(); ++t)
    {
    tetra = *t;
    tetra->CurrentPointId = VTK_LARGE_INTEGER; //mark visited
    for (i=0; i<4; i++)
      {
      if ( tetra->Neighbors[i] &&
           tetra->Neighbors[i]->CurrentPointId != VTK_LARGE_INTEGER &&
           tetra->GetType() != tetra->Neighbors[i]->GetType() )
        {//face not yet visited
        tetra->GetFacePoints(i,face);
        numTris++;
        tris->InsertNextCell(3);
        tris->InsertCellPoint(face.Points[0]->Id);
        tris->InsertCellPoint(face.Points[1]->Id);
        tris->InsertCellPoint(face.Points[2]->Id);
        }
      }
    }//for all tetras

  return numTris;
}

vtkIdType vtkOrderedTriangulator::AddTriangles(vtkIdType id, vtkCellArray *tris)
{
  vtkIdType numTris=0;
  int i;

  // Loop over all tetras examining each unvisited face. Faces whose
  // points are all classified "boundary" are added to the list of
  // faces.
  TetraListIterator t;
  vtkOTTetra *tetra;
  vtkOTFace face;

  // loop over all tetras getting the faces classified on the boundary
  for (t=this->Mesh->Tetras.begin(); t != this->Mesh->Tetras.end(); ++t)
    {
    tetra = *t;
    tetra->CurrentPointId = VTK_LARGE_INTEGER; //mark visited
    for (i=0; i<4; i++)
      {
      if ( tetra->Neighbors[i] &&
           tetra->Neighbors[i]->CurrentPointId != VTK_LARGE_INTEGER &&
           tetra->GetType() != tetra->Neighbors[i]->GetType() )
        {//face not yet visited
        tetra->GetFacePoints(i,face);
        if ( face.Points[0]->Id == id || face.Points[1]->Id == id ||
             face.Points[2]->Id == id )
          {
          numTris++;
          tris->InsertNextCell(3);
          tris->InsertCellPoint(face.Points[0]->Id);
          tris->InsertCellPoint(face.Points[1]->Id);
          tris->InsertCellPoint(face.Points[2]->Id);
          }
        }
      }
    }//for all tetras

  return numTris;
}


//------------------------------------------------------------------------
void vtkOrderedTriangulator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "PreSorted: " << (this->PreSorted ? "On\n" : "Off\n");
  os << indent << "UseTwoSortIds: " << (this->UseTwoSortIds ? "On\n" : "Off\n");

}

