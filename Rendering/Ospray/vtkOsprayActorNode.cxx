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
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

//============================================================================

namespace vtkosp {

  class Vec2 {
  public:
    Vec2(float x, float y) {
      vals[0] = x;
      vals[1] = y;
    }
    float operator[](unsigned int i) const { return vals[i]; }
    float x() { return vals[0]; }
    float y() { return vals[1]; }

    float vals[2];
  };

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

  class Vec4 {
  public:
    Vec4(float x, float y, float z, float w) {
      vals[0] = x;
      vals[1] = y;
      vals[2] = z;
      vals[3] = w;
    }
    float operator[](unsigned int i) const { return vals[i]; }
    float x() { return vals[0]; }
    float y() { return vals[1]; }
    float z() { return vals[2]; }
    float w() { return vals[3]; }

    float vals[4];
  };

  class Mesh {
  public:
    size_t size() { return vertex_indices.size() / 3; }
    std::vector<size_t> vertex_indices;
    std::vector<Vec3> vertices;
    std::vector<Vec3> vertexNormals;
    std::vector<Vec2> texCoords;
    std::vector<size_t> texture_indices;
    std::vector<size_t> normal_indices;
    std::vector<Vec4> colors;
    std::vector<ospray::vec3fa> wireframe_vertex;
    std::vector<int> wireframe_index;
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
  cerr << "Hello from " << this << " " << this->GetClassName() << endl;
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
  vtkPolyData *pd = (vtkPolyData*)(act->GetMapper()->GetInput());

  OSPModel oModel = (OSPModel) model;

  vtkPoints *points = pd->GetPoints();

  vtkosp::Mesh *mesh = new vtkosp::Mesh();

  for (int i = 0; i < points->GetNumberOfPoints(); i++)
    {
    double *pos = points->GetPoint(i);
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
        pos = points->GetPoint(fixIndex--);
        }
      } while (wasNan == true && fixIndex >= 0);
    mesh->vertices.push_back(vtkosp::Vec3(pos[0], pos[1], pos[2]));
    }

  int total_triangles = 0;
  vtkCellArray *cells = pd->GetPolys();
  vtkIdType npts = 0, *index = 0, cellNum = 0;

  for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++)
    {
    int triangle[3];

    // the first triangle
    triangle[0] = index[0];
    triangle[1] = index[1];
    triangle[2] = index[2];
    mesh->vertex_indices.push_back(triangle[0]);
    mesh->vertex_indices.push_back(triangle[1]);
    mesh->vertex_indices.push_back(triangle[2]);
    // mesh->face_material.push_back(0);

    /*
    if (!mesh->vertexNormals.empty())
      {
      mesh->normal_indices.push_back(triangle[0]);
      mesh->normal_indices.push_back(triangle[1]);
      mesh->normal_indices.push_back(triangle[2]);
      }
    */
    total_triangles++;

    // the remaining triangles, of which
    // each introduces a triangle after extraction
    for (int i = 3; i < npts; i++)
      {
      triangle[1] = triangle[2];
      triangle[2] = index[i];
      mesh->vertex_indices.push_back(triangle[0]);
      mesh->vertex_indices.push_back(triangle[1]);
      mesh->vertex_indices.push_back(triangle[2]);

      if (!mesh->vertexNormals.empty())
        {
        mesh->normal_indices.push_back(triangle[0]);
        mesh->normal_indices.push_back(triangle[1]);
        mesh->normal_indices.push_back(triangle[2]);
        }

      total_triangles++;
      }
    }

  if (mesh->size())
    {
    size_t numNormals = mesh->vertexNormals.size();
    //size_t numTexCoords = mesh->texCoords.size();
    size_t numPositions = mesh->vertices.size();
    size_t numTriangles = mesh->vertex_indices.size() / 3;

    ospray::vec3fa *vertices = (ospray::vec3fa *)embree::alignedMalloc
      (sizeof(ospray::vec3fa) * numPositions);
    ospray::vec3i *triangles = (ospray::vec3i *)embree::alignedMalloc
      (sizeof(ospray::vec3i) * numTriangles);
    ospray::vec3fa *normals = (ospray::vec3fa *)embree::alignedMalloc
      (sizeof(ospray::vec3fa) * numNormals);

    for (size_t i = 0; i < numPositions; i++)
      {
      vertices[i] =
        ospray::vec3fa(mesh->vertices[i].x(), mesh->vertices[i].y(),
                       mesh->vertices[i].z());
      }
    for (size_t i = 0, mi = 0; i < numTriangles; i++, mi += 3)
      {
      triangles[i] = embree::Vec3i(mesh->vertex_indices[mi + 0],
                                   mesh->vertex_indices[mi + 1],
                                   mesh->vertex_indices[mi + 2]);
      }
    for (size_t i = 0; i < numNormals; i++)
      {
      normals[i] = ospray::vec3fa(mesh->vertexNormals[i].x(),
                                  mesh->vertexNormals[i].y(),
                                  mesh->vertexNormals[i].z());
      }

    OSPGeometry ospMesh = ospNewTriangleMesh();
    OSPData position = ospNewData(numPositions, OSP_FLOAT3A, &vertices[0]);
    ospSetData(ospMesh, "position", position);

    if (!mesh->normal_indices.empty())
      {
      OSPData normal =
        ospNewData(mesh->vertexNormals.size(), OSP_FLOAT3A, &normals[0]);
      ospSetData(ospMesh, "vertex.normal", normal);
      }

    OSPData index = ospNewData(numTriangles, OSP_INT3, &triangles[0]);
    ospSetData(ospMesh, "index", index);

    if (!mesh->texCoords.empty())
      {
      OSPData texcoord =
        ospNewData(mesh->texCoords.size(), OSP_FLOAT2, &mesh->texCoords[0]);
      assert(mesh->texCoords.size() > 0);
      ospSetData(ospMesh, "vertex.texcoord", texcoord);
      }

    if (!mesh->colors.empty())
      {
      OSPData colors =
        ospNewData(mesh->colors.size(), OSP_FLOAT4, &mesh->colors[0]);
      ospSetData(ospMesh, "vertex.color", colors);
      }

    //ospSetMaterial(ospMesh, ospMaterial);
    ospCommit(ospMesh);

    ospAddGeometry(oModel, ospMesh);
    }
}
