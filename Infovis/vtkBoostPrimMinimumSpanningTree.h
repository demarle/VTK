/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBoostPrimMinimumSpanningTree.h

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
// .NAME vtkBoostPrimMinimumSpanningTree - Contructs a minimum spanning
//    tree from a graph, start node, and the weighting array
//
// .SECTION Description
//
// This vtk class uses the Boost Prim Minimum Spanning Tree 
// generic algorithm to perform a minimum spanning tree creation given
// a weighting value for each of the edges in the input graph and a 
// a starting node for the tree.
//
// .SECTION See Also
// vtkGraph vtkBoostGraphAdapter

#ifndef __vtkBoostPrimMinimumSpanningTree_h
#define __vtkBoostPrimMinimumSpanningTree_h

#include "vtkStdString.h" // For string type
#include "vtkVariant.h" // For variant type

#include "vtkTreeAlgorithm.h"

class VTK_INFOVIS_EXPORT vtkBoostPrimMinimumSpanningTree : public vtkTreeAlgorithm 
{
public:
  static vtkBoostPrimMinimumSpanningTree *New();
  vtkTypeRevisionMacro(vtkBoostPrimMinimumSpanningTree, vtkTreeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the name of the edge-weight input array, which must name an
  // array that is part of the edge data of the input graph and
  // contains numeric data. If the edge-weight array is not of type
  // vtkDoubleArray, the array will be copied into a temporary
  // vtkDoubleArray.
  vtkSetStringMacro(EdgeWeightArrayName);

  // Description:
  // Set the index (into the vertex array) of the 
  // minimum spanning tree 'origin' vertex.
  void SetOriginVertex(vtkIdType index);
  
  //BTX

  // Description:
  // Set the minimum spanning tree 'origin' vertex.
  // This method is basically the same as above
  // but allows the application to simply specify
  // an array name and value, instead of having to
  // know the specific index of the vertex.
  void SetOriginVertex(vtkStdString arrayName, vtkVariant value);
  //ETX
  
  // Description:
  // Stores the graph vertex ids for the tree vertices in an array
  // named "GraphVertexId".  Default is off.
  vtkSetMacro(CreateGraphVertexIdArray, bool);
  vtkGetMacro(CreateGraphVertexIdArray, bool);
  vtkBooleanMacro(CreateGraphVertexIdArray, bool);
  
  
  // Description:
  // Whether to negate the edge weights. By negating the edge
  // weights this algorithm will give you the 'maximal' spanning
  // tree (i.e. the algorithm will try to create a spanning tree
  // with the highest weighted edges). Defaulted to Off.
  // FIXME: put a real definition in...
  void SetNegateEdgeWeights(bool value);
  vtkGetMacro(NegateEdgeWeights, bool);
  vtkBooleanMacro(NegateEdgeWeights, bool);

protected:
  vtkBoostPrimMinimumSpanningTree();
  ~vtkBoostPrimMinimumSpanningTree();

  int RequestData(
      vtkInformation *,
      vtkInformationVector **,
      vtkInformationVector *);

  int FillInputPortInformation(
      int port, vtkInformation* info);
 
private:
  char* EdgeWeightArrayName;
  vtkIdType OriginVertexIndex;
  vtkVariant OriginValue;
  bool CreateGraphVertexIdArray;
  bool ArrayNameSet;
  char* ArrayName;
  bool NegateEdgeWeights;
  float EdgeWeightMultiplier;
  
  // Description:
  // Using the convenience function internally
  vtkSetStringMacro(ArrayName);

  // Description:
  // This method is basically a helper function to find
  // the index of a specific value within a specific array
  vtkIdType GetVertexIndex(
    vtkAbstractArray *abstract,vtkVariant value);

  vtkBoostPrimMinimumSpanningTree(const vtkBoostPrimMinimumSpanningTree&);  // Not implemented.
  void operator=(const vtkBoostPrimMinimumSpanningTree&);  // Not implemented.
};

#endif
