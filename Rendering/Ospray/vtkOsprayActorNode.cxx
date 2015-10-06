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
#include "vtkPointData.h"
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
#define SHOW_MESH_PTS 1
#define SHOW_MESH_WIRE 1
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
void vtkOsprayActorNode::ORender(void *renderer, void *model)
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
//  cerr << "NUMPTS" << poly->GetPoints()->GetNumberOfPoints() << endl;
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

  //material
  OSPRenderer oRenderer = (OSPRenderer) renderer;
  OSPMaterial oMaterial = ospNewMaterial(oRenderer,"OBJMaterial");
  float diffusef[] = {(float)this->DiffuseColor[0],
                      (float)this->DiffuseColor[1],
                      (float)this->DiffuseColor[2]};
  float specularf[] = {(float)this->SpecularColor[0],
                       (float)this->SpecularColor[1],
                       (float)this->SpecularColor[2]};
  ospSet3fv(oMaterial,"Kd",diffusef);
  ospSet3fv(oMaterial,"Ks",specularf);
  ospSet1f(oMaterial,"Ns",float(this->SpecularPower*.5));
  ospSet1f(oMaterial,"d", float(this->Opacity));
  ospCommit(oMaterial);

  //normals
  ospray::vec3fa *normals = NULL;
  int numNormals = 0;
  if (this->Interpolation != VTK_FLAT)
    {
    vtkPointData *pointData = poly->GetPointData();
    if (vtkDataArray *vNormals = pointData->GetNormals())
      {
      numNormals= vNormals->GetNumberOfTuples();
      normals = (ospray::vec3fa *)embree::alignedMalloc
        (sizeof(ospray::vec3fa) * numNormals);

      //vtkDataArray *normals = vtkFloatArray::New();
      //normals->SetNumberOfComponents(3);
      //transform->TransformNormals(pointData->GetNormals(), normals);
      for (int i = 0; i < numNormals; i++)
        {
        double *vNormal = vNormals->GetTuple(i);
        normals[i] = ospray::vec3fa(vNormal[0],vNormal[1],vNormal[2]);
        }
      //normals->Delete();
      }
  }

  //direct colors
  int numColors = 0;
  ospray::vec4f *colors = NULL;
  act->GetMapper()->MapScalars(1.0);
  vtkUnsignedCharArray *vColors = act->GetMapper()->Colors;
  if (vColors)
    {
    numColors = vColors->GetNumberOfTuples();
    colors = (ospray::vec4f *)malloc
        (sizeof(ospray::vec4f) * numColors);

    for (int i = 0; i < numColors; i++)
      {
      unsigned char *color = vColors->GetPointer(4 * i);
      colors[i] = ospray::vec4f(color[0] / 255.0,
                                color[1] / 255.0,
                                color[2] / 255.0,
                                1);
      }
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
    cerr << "TODO SHOW_POINTS" << endl;
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
      cerr << "TODO SHOW_LINE_POINTS" << endl;
#if SHOW_LINE_POINTS
      OSPGeometry ospMesh = ospray::api::Device::current->newGeometry("spheres");
      ospAddGeometry(oModel, ospMesh);
      ospCommit(ospMesh);
      ospRelease(ospMesh);
#endif
      }
    else
      {
      cerr << "TODO SHOW_LINE_WIRE" << endl;
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
        OSPGeometry ospMesh = ospNewGeometry("spheres");
        float *mdata = new float[3*tIndexArray.size()];
        for (size_t i = 0; i < tIndexArray.size(); i++)
          {
          mdata[i*3+0] = (float)vertices[tIndexArray[i]].x;
          mdata[i*3+1] = (float)vertices[tIndexArray[i]].y;
          mdata[i*3+2] = (float)vertices[tIndexArray[i]].z;
          }
        OSPData _mdata = ospNewData(tIndexArray.size()*3, OSP_FLOAT, mdata);
        ospSetObject(ospMesh, "spheres", _mdata);
        ospSet1i(ospMesh, "bytes_per_sphere", 3*sizeof(float));
        ospSet1i(ospMesh, "offset_center", 0*sizeof(float));
        ospSet1i(ospMesh, "offset_radius", -1);//3*sizeof(float));
        ospSet1f(ospMesh, "radius", 0.02);
        ospSet1i(ospMesh, "offset_materialID", -1);
        ospSet1i(ospMesh, "materialID", 0);
        ospRelease(_mdata);

        ospAddGeometry(oModel, ospMesh);
        ospSetMaterial(ospMesh, oMaterial);

        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
        break;
        }
      case VTK_WIREFRAME:
        {
#if SHOW_MESH_WIRE
#if 1
        OSPGeometry ospMesh = ospNewGeometry("cylinders");
        float *mdata = (float *)malloc
          (sizeof(float)*tIndexArray.size()*3);
        for (size_t i = 0; i < tIndexArray.size(); i++)
          {
          mdata[i*3+0] = (float)vertices[tIndexArray[i]].x;
          mdata[i*3+1] = (float)vertices[tIndexArray[i]].y;
          mdata[i*3+2] = (float)vertices[tIndexArray[i]].z;
          }
        OSPData _mdata = ospNewData(tIndexArray.size()*3, OSP_FLOAT, &mdata[0]);
        //ospCommit(_mdata);
        ospSetData(ospMesh, "cylinders", _mdata);
        ospSet1i(ospMesh, "bytes_per_cylinder", 6*sizeof(float));
        ospSet1i(ospMesh, "offset_v0", 0);
        ospSet1i(ospMesh, "offset_v1", 3*sizeof(float));
        ospSet1f(ospMesh, "radius", 0.01);
        ospSet1i(ospMesh, "offset_radius", -1);
        ospSet1i(ospMesh, "materialID", 0);
        ospSet1i(ospMesh, "offset_materialID", -1);
        ospRelease(_mdata);

        ospSetMaterial(ospMesh, oMaterial);
        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#else
        OSPGeometry ospMesh = ospNewGeometry("streamlines");
        std::vector<int> mdata;
 #if 1
        ospray::vec3fa *mverts = (ospray::vec3fa *)embree::alignedMalloc
          (sizeof(ospray::vec3fa) * tIndexArray.size());
        for (size_t i = 0; i < tIndexArray.size(); i+=2)
          {
          mdata.push_back(i);
          mverts[i+0] = ospray::vec3fa(vertices[tIndexArray[i+0]].x,
                                       vertices[tIndexArray[i+0]].y,
                                       vertices[tIndexArray[i+0]].z);
          mverts[i+1] = ospray::vec3fa(vertices[tIndexArray[i+1]].x,
                                       vertices[tIndexArray[i+1]].y,
                                       vertices[tIndexArray[i+1]].z);
          }
        OSPData _mdata = ospNewData(mdata.size(), OSP_INT, &mdata[0]);
        OSPData _mverts = ospNewData(mdata.size()*2, OSP_FLOAT3A, &mverts[0]);
 #else
        float *mverts = (float *)embree::alignedMalloc
          (sizeof(float) * tIndexArray.size() *6);
        for (size_t i = 0; i < tIndexArray.size(); i+=2)
          {
          mdata.push_back(i);
          mverts[i*6+0] = vertices[tIndexArray[i+0]].x;
          mverts[i*6+1] = vertices[tIndexArray[i+0]].y;
          mverts[i*6+2] = vertices[tIndexArray[i+0]].z;
          mverts[i*6+3] = vertices[tIndexArray[i+1]].x;
          mverts[i*6+4] = vertices[tIndexArray[i+1]].y;
          mverts[i*6+5] = vertices[tIndexArray[i+1]].z;
          }
        OSPData _mdata = ospNewData(mdata.size(), OSP_INT, &mdata[0]);
        OSPData _mverts = ospNewData(mdata.size()*12, OSP_FLOAT, &mverts[0]);
 #endif
        ospSetData(ospMesh, "vertex", _mverts);
        ospSetData(ospMesh, "index", _mdata);
        ospSet1f(ospMesh, "radius", 0.01);

        ospSetMaterial(ospMesh, oMaterial);
        ospAddGeometry(oModel, ospMesh);
        ospCommit(ospMesh);
        ospRelease(ospMesh);
#endif
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

        if (numNormals)
          {
          OSPData _normals =
            ospNewData(numNormals, OSP_FLOAT3A, &normals[0]);
          ospSetData(ospMesh, "vertex.normal", _normals);
          ospRelease(_normals);
          }

        if (numColors)
          {
          OSPData _colors =
            ospNewData(numColors, OSP_FLOAT4, &colors[0]);
          ospSetData(ospMesh, "vertex.color", _colors);
          ospRelease(_colors);
          }

        ospAddGeometry(oModel, ospMesh);

        ospSetMaterial(ospMesh, oMaterial);

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
        cerr << "TODO SHOW_STRIP_POINTS" << endl;
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
        cerr << "TODO SHOW_STRIP_WIRE" << endl;
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
        cerr << "TODO SHOW_STRIP_MESH" << endl;
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
