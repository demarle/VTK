/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMutableDirectedGraph.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkMutableDirectedGraph - An editable directed graph.
//
// .SECTION Description
// vtkMutableDirectedGraph is a directed graph which has additional methods
// for adding edges and vertices. AddChild() is a convenience method for
// constructing trees. ShallowCopy(), DeepCopy(), CheckedShallowCopy() and
// CheckedDeepCopy() will succeed for instances of vtkDirectedGraph,
// vtkMutableDirectedGraph and vtkTree.
//
// .SECTION See Also
// vtkDirectedGraph vtkGraph vtkTree

#ifndef __vtkMutableDirectedGraph_h
#define __vtkMutableDirectedGraph_h

#include "vtkDirectedGraph.h"

class vtkEdgeListIterator;
class vtkGraphEdge;
class vtkVariant;

class VTK_FILTERING_EXPORT vtkMutableDirectedGraph : public vtkDirectedGraph
{
public:
  static vtkMutableDirectedGraph *New();
  vtkTypeRevisionMacro(vtkMutableDirectedGraph, vtkDirectedGraph);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Adds a vertex to the graph and returns the index of the new vertex.
  //
  // \note In a distributed graph (i.e. a graph whose DistributedHelper
  // is non-null), this routine cannot be used to add a vertex
  // if the vertices in the graph have pedigree IDs, because this routine
  // will always add the vertex locally, which may conflict with the
  // proper location of the vertex based on the distribution of the
  // pedigree IDs.
  vtkIdType AddVertex();

  // Description:
  // Adds a vertex to the graph with associated properties defined in
  // \p propertyArr and returns the index of the new vertex.
  // The number and order of values in \p propertyArr must match up with the
  // arrays in the vertex data retrieved by GetVertexData().
  // 
  // If a vertex with the given pedigree ID already exists, its properties will be
  // overwritten with the properties in \p propertyArr and the existing
  // vertex index will be returned.
  //
  // \note In a distributed graph (i.e. a graph whose DistributedHelper
  // is non-null) the vertex added or found might not be local. In this case,
  // AddVertex will wait until the vertex can be added or found
  // remotely, so that the proper vertex index can be returned. If you
  // don't actually need to use the vertex index, consider calling
  // LazyAddVertex, which provides better performance by eliminating
  // the delays associated with returning the vertex index.
  vtkIdType AddVertex(vtkVariantArray *propertyArr);

  //BTX
  // Description:
  // Adds a vertex with the given \p pedigreeID to the graph and
  // returns the index of the new vertex.
  //
  // If a vertex with the given pedigree ID already exists,
  // the existing vertex index will be returned.
  //
  // \note In a distributed graph (i.e. a graph whose DistributedHelper
  // is non-null) the vertex added or found might not be local. In this case,
  // AddVertex will wait until the vertex can be added or found
  // remotely, so that the proper vertex index can be returned. If you
  // don't actually need to use the vertex index, consider calling
  // LazyAddVertex, which provides better performance by eliminating
  // the delays associated with returning the vertex index.
  vtkIdType AddVertex(const vtkVariant& pedigreeId);
  //ETX

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u and \p v are vertex indices,
  // and returns a \p vtkEdgeType structure describing that edge.
  //
  // \p vtkEdgeType contains fields for \p Source vertex index,
  // \p Target vertex index, and edge index \p Id.
//BTX
  vtkEdgeType AddEdge(vtkIdType u, vtkIdType v);
//ETX

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u and \p v are vertex indices,
  // with associated properties defined in \p propertyArr
  // and returns a \p vtkEdgeType structure describing that edge.
  //
  // The number and order of values in \p propertyArr must match up with the
  // arrays in the edge data retrieved by GetEdgeData().
  //
  // \p vtkEdgeType contains fields for \p Source vertex index,
  // \p Target vertex index, and edge index \p Id.
//BTX
  vtkEdgeType AddEdge(vtkIdType u, vtkIdType v,
                      vtkVariantArray *propertyArr);
//ETX

  //BTX
  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u is a vertex pedigree ID and \p v is a vertex index,
  // and returns a \p vtkEdgeType structure describing that edge.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  //
  // \p vtkEdgeType contains fields for \p Source vertex index,
  // \p Target vertex index, and edge index \p Id.
  vtkEdgeType AddEdge(const vtkVariant& u, vtkIdType v,
                      vtkVariantArray *propertyArr = 0);

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u is a vertex index and \p v is a vertex pedigree ID,
  // and returns a \p vtkEdgeType structure describing that edge.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  //
  // \p vtkEdgeType contains fields for \p Source vertex index,
  // \p Target vertex index, and edge index \p Id.
  vtkEdgeType AddEdge(vtkIdType u, const vtkVariant& v,
                      vtkVariantArray *propertyArr = 0);

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u and \p v are vertex pedigree IDs,
  // and returns a \p vtkEdgeType structure describing that edge.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  //
  // \p vtkEdgeType contains fields for \p Source vertex index,
  // \p Target vertex index, and edge index \p Id.
  vtkEdgeType AddEdge(const vtkVariant& u,
                      const vtkVariant& v,
                      vtkVariantArray *propertyArr = 0);
  //ETX

  // Description:
  // Adds a vertex to the graph.
  // 
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddVertex();

  // Description:
  // Adds a vertex to the graph with associated properties defined in
  // \p propertyArr.
  // The number and order of values in \p propertyArr must match up with the
  // arrays in the vertex data retrieved by GetVertexData().
  // 
  // If a vertex with the given pedigree ID already exists, its properties will be
  // overwritten with the properties in \p propertyArr.
  // 
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddVertex(vtkVariantArray *propertyArr);

  //BTX
  // Description:
  // Adds a vertex with the given \p pedigreeID to the graph.
  //
  // If a vertex with the given pedigree ID already exists,
  // no operation is performed.
  //
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddVertex(const vtkVariant& pedigreeId);

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u and \p v are vertex indices.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  //
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddEdge(vtkIdType u, vtkIdType v, vtkVariantArray *propertyArr = 0);

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u is a vertex pedigree ID and \p v is a vertex index.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  // 
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddEdge(const vtkVariant& u, vtkIdType v,
                   vtkVariantArray *propertyArr = 0);

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u is a vertex index and \p v is a vertex pedigree ID.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  // 
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddEdge(vtkIdType u, const vtkVariant& v,
                   vtkVariantArray *propertyArr = 0);

  // Description:
  // Adds a directed edge from \p u to \p v,
  // where \p u and \p v are vertex pedigree IDs.
  //
  // The number and order of values in the optional parameter
  // \p propertyArr must match up with the arrays in the edge data
  // retrieved by GetEdgeData().
  //
  // This method is lazily evaluated for distributed graphs (i.e. graphs
  // whose DistributedHelper is non-null) the next time Synchronize is
  // called on the helper.
  void LazyAddEdge(const vtkVariant& u,
                   const vtkVariant& v,
                   vtkVariantArray *propertyArr = 0);
  //ETX

  // Description:
  // Variant of AddEdge() that returns a heavyweight \p vtkGraphEdge object.
  // The graph owns the reference of the edge and will replace
  // its contents on the next call to AddGraphEdge().
  // 
  // \note This is a less efficient method for use with wrappers.
  // In C++ you should use the faster AddEdge().
  vtkGraphEdge *AddGraphEdge(vtkIdType u, vtkIdType v);

  // Description:
  // Convenience method for creating trees.
  // Returns the newly created vertex id.
  // Shortcut for
  // \code
  // vtkIdType v = g->AddVertex();
  // g->AddEdge(parent, v);
  // \endcode
  // If non-null, \p propertyArr provides edge properties
  // for the newly-created edge. The values in \p propertyArr must match
  // up with the arrays in the edge data returned by GetEdgeData().
  vtkIdType AddChild(vtkIdType parent,
                     vtkVariantArray *propertyArr);
  vtkIdType AddChild(vtkIdType parent)
    { return this->AddChild(parent, 0); }

protected:
  vtkMutableDirectedGraph();
  ~vtkMutableDirectedGraph();

  // Description:
  // Graph edge that is reused of \p AddGraphEdge calls.
  vtkGraphEdge *GraphEdge;

private:
  vtkMutableDirectedGraph(const vtkMutableDirectedGraph&);  // Not implemented.
  void operator=(const vtkMutableDirectedGraph&);  // Not implemented.
};

#endif
