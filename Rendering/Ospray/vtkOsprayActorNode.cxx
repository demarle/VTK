/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayActorNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayActorNode.h"

#include "vtkActor.h"
#include "vtkCellArray.h"
#include "vtkCollectionIterator.h"
#include "vtkMapper.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkProperty.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"
#include "ospray/api/Device.h"
#include "ospray/common/OSPCommon.h"

#define SHOW_POINTS 0
#define SHOW_LINE_POINTS 0
#define SHOW_LINE_WIRE 0
#define SHOW_MESH_PTS 0
#define SHOW_MESH_WIRE 0
#define SHOW_MESH_SURF 1
#define SHOW_STRIP_PTS 0
#define SHOW_STRIP_WIRE 0
#define SHOW_STRIP_MESH 0

//============================================================================

namespace vtkosp {

  class Vec3 {
  public:
    Vec3(float x, float y, float z) {
      vals[0] = x;
      vals[1] = y;
      vals[2] = z;
    }
    float operator[](unsigned int i) const { return vals[i]; }
    float x() { return vals[0]; }
    float y() { return vals[1]; }
    float z() { return vals[2]; }

    float vals[3];
  };

}

//============================================================================
vtkStandardNewMacro(vtkOsprayActorNode);

//----------------------------------------------------------------------------
vtkOsprayActorNode::vtkOsprayActorNode()
{
}

//----------------------------------------------------------------------------
vtkOsprayActorNode::~vtkOsprayActorNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayActorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayActorNode::RenderSelf()
{
}

namespace {
  //borrowed from vtkOpenGLIndexBufferObject
  //TODO: rule of three if this is copied anywhere else in VTK
  void CreatePointIndexBuffer(vtkCellArray *cells, std::vector<unsigned int> &indexArray)
  {
    //TODO: restore the preallocate and append to offset features I omitted
    vtkIdType* indices(NULL);
    vtkIdType npts(0);
    if (!cells->GetNumberOfCells())
      {
      return;
      }
    for (cells->InitTraversal(); cells->GetNextCell(npts, indices); )
      {
      for (int i = 0; i < npts; ++i)
        {
        indexArray.push_back(static_cast<unsigned int>(*(indices++)));
        }
      }
    }
  void CreateLineIndexBuffer(vtkCellArray *cells, std::vector<unsigned int> &indexArray)
  {
    //TODO: restore the preallocate and append to offset features I omitted
    vtkIdType* indices(NULL);
    vtkIdType npts(0);
    if (!cells->GetNumberOfCells())
      {
      return;
      }
    for (cells->InitTraversal(); cells->GetNextCell(npts, indices); )
      {
      for (int i = 0; i < npts-1; ++i)
        {
        indexArray.push_back(static_cast<unsigned int>(indices[i]));
        indexArray.push_back(static_cast<unsigned int>(indices[i+1]));
        }
      }
  }
  void CreateTriangleLineIndexBuffer(vtkCellArray *cells, std::vector<unsigned int> &indexArray)
  {
    //TODO: restore the preallocate and append to offset features I omitted
    vtkIdType* indices(NULL);
    vtkIdType npts(0);
    if (!cells->GetNumberOfCells())
      {
      return;
      }
    for (cells->InitTraversal(); cells->GetNextCell(npts, indices); )
      {
      for (int i = 0; i < npts; ++i)
        {
        indexArray.push_back(static_cast<unsigned int>(indices[i]));
        indexArray.push_back(static_cast<unsigned int>
                             (indices[i < npts-1 ? i+1 : 0]));
        }
      }
  }
  void CreateTriangleIndexBuffer(vtkCellArray *cells, vtkPoints *points, std::vector<unsigned int> &indexArray)
  {
    //TODO: restore the preallocate and append to offset features I omitted
    vtkIdType* indices(NULL);
    vtkIdType npts(0);
    if (!cells->GetNumberOfCells())
      {
      return;
      }

    // the folowing are only used if we have to triangulate a polygon
    // otherwise they just sit at NULL
    vtkPolygon *polygon = NULL;
    vtkIdList *tris = NULL;
    vtkPoints *triPoints = NULL;

    for (cells->InitTraversal(); cells->GetNextCell(npts, indices); )
      {
      // ignore degenerate triangles
      if (npts < 3)
        {
        continue;
        }

      // triangulate needed
      if (npts > 3)
        {
        // special case for quads, penta, hex which are common
        if (npts == 4)
          {
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[1]));
          indexArray.push_back(static_cast<unsigned int>(indices[2]));
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[2]));
          indexArray.push_back(static_cast<unsigned int>(indices[3]));
          }
        else if (npts == 5)
          {
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[1]));
          indexArray.push_back(static_cast<unsigned int>(indices[2]));
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[2]));
          indexArray.push_back(static_cast<unsigned int>(indices[3]));
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[3]));
          indexArray.push_back(static_cast<unsigned int>(indices[4]));
          }
        else if (npts == 6)
          {
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[1]));
          indexArray.push_back(static_cast<unsigned int>(indices[2]));
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[2]));
          indexArray.push_back(static_cast<unsigned int>(indices[3]));
          indexArray.push_back(static_cast<unsigned int>(indices[0]));
          indexArray.push_back(static_cast<unsigned int>(indices[3]));
          indexArray.push_back(static_cast<unsigned int>(indices[5]));
          indexArray.push_back(static_cast<unsigned int>(indices[3]));
          indexArray.push_back(static_cast<unsigned int>(indices[4]));
          indexArray.push_back(static_cast<unsigned int>(indices[5]));
          }
        else // 7 sided polygon or higher, do a full smart triangulation
          {
          if (!polygon)
            {
            polygon = vtkPolygon::New();
            tris = vtkIdList::New();
            triPoints = vtkPoints::New();
            }

          vtkIdType *triIndices = new vtkIdType[npts];
          triPoints->SetNumberOfPoints(npts);
          for (int i = 0; i < npts; ++i)
            {
            int idx = indices[i];
            triPoints->SetPoint(i, points->GetPoint(idx));
            triIndices[i] = i;
            }
          polygon->Initialize(npts, triIndices, triPoints);
          polygon->Triangulate(tris);
          for (int j = 0; j < tris->GetNumberOfIds(); ++j)
            {
            indexArray.push_back(static_cast<unsigned int>
                                 (indices[tris->GetId(j)]));
            }
          delete [] triIndices;
          }
        }
      else
        {
        indexArray.push_back(static_cast<unsigned int>(*(indices++)));
        indexArray.push_back(static_cast<unsigned int>(*(indices++)));
        indexArray.push_back(static_cast<unsigned int>(*(indices++)));
        }
      }
    if (polygon)
      {
      polygon->Delete();
      tris->Delete();
      triPoints->Delete();
      }
  }
  void CreateStripIndexBuffer(vtkCellArray *cells, std::vector<unsigned int> &indexArray, bool wireframeTriStrips)
  {
    if (!cells->GetNumberOfCells())
      {
      return;
      }
    vtkIdType      *pts = 0;
    vtkIdType      npts = 0;

    size_t triCount = cells->GetNumberOfConnectivityEntries()
      - 3*cells->GetNumberOfCells();
    size_t targetSize = wireframeTriStrips ? 2*(triCount*2+1)
      : triCount*3;
    indexArray.reserve(targetSize);

    if (wireframeTriStrips)
      {
      for (cells->InitTraversal(); cells->GetNextCell(npts,pts); )
        {
        indexArray.push_back(static_cast<unsigned int>(pts[0]));
        indexArray.push_back(static_cast<unsigned int>(pts[1]));
        for (int j = 0; j < npts-2; ++j)
          {
          indexArray.push_back(static_cast<unsigned int>(pts[j]));
          indexArray.push_back(static_cast<unsigned int>(pts[j+2]));
          indexArray.push_back(static_cast<unsigned int>(pts[j+1]));
          indexArray.push_back(static_cast<unsigned int>(pts[j+2]));
          }
        }
      }
    else
      {
      for (cells->InitTraversal(); cells->GetNextCell(npts,pts); )
        {
        for (int j = 0; j < npts-2; ++j)
          {
          indexArray.push_back(static_cast<unsigned int>(pts[j]));
          indexArray.push_back(static_cast<unsigned int>(pts[j+1+j%2]));
          indexArray.push_back(static_cast<unsigned int>(pts[j+1+(j+1)%2]));
          }
        }
      }
  }

}
//----------------------------------------------------------------------------
void vtkOsprayActorNode::ORender(void *model)
{
  if (this->Visibility == false)
    {
    return;
    }

  //TODO: not safe assumption, in general may not be actor or polydata.
  vtkActor *act = (vtkActor*)this->GetRenderable();
  vtkPolyData *poly = (vtkPolyData*)(act->GetMapper()->GetInput());

  vtkCellArray *prims[4];
  prims[0] =  poly->GetVerts();
  std::vector<unsigned int> pIndexArray;
  prims[1] =  poly->GetLines();
  std::vector<unsigned int> lIndexArray;
  prims[2] =  poly->GetPolys();
  std::vector<unsigned int> tIndexArray;
  prims[3] =  poly->GetStrips();
  std::vector<unsigned int> sIndexArray;

  CreatePointIndexBuffer(prims[0], pIndexArray);
  switch (this->Representation)
    {
    case VTK_POINTS:
      {
      CreatePointIndexBuffer(prims[1], lIndexArray);
      CreatePointIndexBuffer(prims[2], tIndexArray);
      CreatePointIndexBuffer(prims[3], sIndexArray);
      break;
      }
    case VTK_WIREFRAME:
      {
      CreateLineIndexBuffer(prims[1], lIndexArray);
      CreateTriangleLineIndexBuffer(prims[2], tIndexArray);
      CreateStripIndexBuffer(prims[3], sIndexArray, true);
      break;
      }
    default:
      {
      CreateTriangleIndexBuffer(prims[2], poly->GetPoints(), tIndexArray);
      CreateStripIndexBuffer(prims[3], sIndexArray, false);
      }
    }

  std::vector<vtkosp::Vec3> _vertices;
  for (int i = 0; i < poly->GetPoints()->GetNumberOfPoints(); i++)
    {
    double *pos = poly->GetPoints()->GetPoint(i);
    bool wasNan = false;
    int fixIndex = i - 1;
    do
      {
      wasNan = false;
      for (int j = 0; j < 3; j++)
        {
        if (std::isnan(pos[j]))
          {
          wasNan = true;
          }
        }
      if (wasNan && fixIndex >= 0)
        {
        pos = poly->GetPoints()->GetPoint(fixIndex--);
        }
      } while (wasNan == true && fixIndex >= 0);
    _vertices.push_back(vtkosp::Vec3(pos[0], pos[1], pos[2]));
    }

  //points
  OSPModel oModel = (OSPModel) model;
  size_t numPositions = _vertices.size();
  ospray::vec3fa *vertices = (ospray::vec3fa *)embree::alignedMalloc
    (sizeof(ospray::vec3fa) * numPositions);
  for (size_t i = 0; i < numPositions; i++)
    {
    vertices[i] =
      ospray::vec3fa(_vertices[i].x(),
                     _vertices[i].y(),
                     _vertices[i].z());
    }
  OSPData position = ospNewData(numPositions, OSP_FLOAT3A, &vertices[0]);

  if (pIndexArray.size())
    {
    //draw vertices
#if SHOW_POINTS
    OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("spheres");
    ospAddGeometry(oModel, ospMesh);
    ospCommit(ospMesh);
    ospRelease(ospMesh);
#endif
    }

  if (lIndexArray.size())
    {
    //draw lines
    if (this->Representation == VTK_POINTS)
      {
#if SHOW_LINE_POINTS
      OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("spheres");
      ospAddGeometry(oModel, ospMesh);
      ospCommit(ospMesh);
      ospRelease(ospMesh);
#endif
      }
    else
      {
#if SHOW_LINE_WIRE
      OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("cylinders");
      ospAddGeometry(oModel, ospMesh);
      ospCommit(ospMesh);
      ospRelease(ospMesh);
#endif
      }
    }

  if (tIndexArray.size())
    {
    //draw triangles
    switch (this->Representation)
      {
      case VTK_POINTS:
        {
#if SHOW_MESH_PTS
        OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("spheres");
        ospray::vec4f *mdata = (ospray::vec4f *)embree::alignedMalloc
          (sizeof(ospray::vec4f) * tIndexArray.size());
        OSPData _mdata = ospNewData(tIndexArray.size(), OSP_FLOAT4, &mdata[0]);
        for (size_t i = 0; i < tIndexArray.size(); i++)
          {
          mdata[i] = embree::Vec4f(vertices[tIndexArray[i]].x,
                                   vertices[tIndexArray[i]].y,
                                   vertices[tIndexArray[i]].z,
                                   100);
          }
        ospSetData(ospMesh, "spheres", _mdata);
        ospSet1f(ospMesh, "radius", 0.05); //TODO: control by mapper pointsize AND/or variable
        ospSet1i(ospMesh, "bytes_per_sphere", 4*sizeof(float));
        ospRelease(_mdata);

        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        break;
        }
      case VTK_WIREFRAME:
        {
#if SHOW_MESH_WIRE
        OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("cylinders");
        cerr << tIndexArray.size() << " " << prims[2]->GetNumberOfCells() << endl;
        float *mdata = (float *)embree::alignedMalloc
          (sizeof(float )*tIndexArray.size()*3);
        OSPData _mdata = ospNewData(tIndexArray.size()*3, OSP_FLOAT, &mdata[0]);
        for (size_t i = 0; i < tIndexArray.size(); i++)
          {
          mdata[i*3+0] = vertices[tIndexArray[i]].x;
          mdata[i*3+1] = vertices[tIndexArray[i]].y;
          mdata[i*3+2] = vertices[tIndexArray[i]].z;
          }
        ospSetData(ospMesh, "cylinders", _mdata);
        ospSet1f(ospMesh, "radius", 0.05); //TODO: control by mapper linewidth AND/or variable
        ospSet1i(ospMesh, "bytes_per_cylinder", 6*sizeof(float));
        ospRelease(_mdata);

        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        break;
        }
      default:
        {
#if SHOW_MESH_SURF
//      OSPGeometry ospMesh = ospNewTriangleMesh();
        OSPGeometry ospMesh = ospNewGeometry("trianglemesh");
        ospSetData(ospMesh, "position", position);

        size_t numTriangles = tIndexArray.size() / 3;
        ospray::vec3i *triangles = (ospray::vec3i *)embree::alignedMalloc
          (sizeof(ospray::vec3i) * numTriangles);
        for (size_t i = 0, mi = 0; i < numTriangles; i++, mi += 3)
          {
          triangles[i] = embree::Vec3i(tIndexArray[mi + 0],
                                       tIndexArray[mi + 1],
                                       tIndexArray[mi + 2]);
          }
        OSPData index = ospNewData(numTriangles, OSP_INT3, &triangles[0]);
        ospSetData(ospMesh, "index", index);
        ospRelease(index);

        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        }
      }
    }

  if (sIndexArray.size())
    {
    //draw strips
    switch (this->Representation)
      {
      case VTK_POINTS:
        {
#if SHOW_STRIP_PTS
        OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("spheres");
        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        break;
        }
      case VTK_WIREFRAME:
        {
#if SHOW_STRIP_WIRE
        OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("cylinders");
        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        break;
        }
      default:
        {
#if SHOW_STRIP_MESH
        OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("trianglemesh");
        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        }
      }
    }
  ospRelease(position);
}
