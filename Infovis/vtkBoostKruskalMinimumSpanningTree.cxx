/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBoostKruskalMinimumSpanningTree.cxx

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
#include "vtkBoostKruskalMinimumSpanningTree.h"

#include "vtkBoostGraphAdapter.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkMath.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkDataArray.h"
#include "vtkSelection.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkUndirectedGraph.h"
#include "vtkTree.h"

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/vector_property_map.hpp>
#include <boost/pending/queue.hpp>

using namespace boost;

vtkCxxRevisionMacro(vtkBoostKruskalMinimumSpanningTree, "1.1");
vtkStandardNewMacro(vtkBoostKruskalMinimumSpanningTree);

// Constructor/Destructor
vtkBoostKruskalMinimumSpanningTree::vtkBoostKruskalMinimumSpanningTree()
{
  this->EdgeWeightArrayName = 0;
  this->OutputSelectionType = 0;
  this->SetOutputSelectionType("MINIMUM_SPANNING_TREE_EDGES");
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkBoostKruskalMinimumSpanningTree::~vtkBoostKruskalMinimumSpanningTree()
{
  this->SetEdgeWeightArrayName(0);
}

int vtkBoostKruskalMinimumSpanningTree::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkGraph *input = vtkGraph::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkSelection *output = vtkSelection::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Retrieve the edge-weight array.
  if (!this->EdgeWeightArrayName)
    {
    vtkErrorMacro("Edge-weight array name is required");
    return 1;
    }
  vtkDataArray* edgeWeightArray = input->GetEdgeData()->GetArray(this->EdgeWeightArrayName);
    
  // Does the edge-weight array exist at all?  
  if (edgeWeightArray == NULL)
    {
    vtkErrorMacro("Could not find edge-weight array named " 
                  << this->EdgeWeightArrayName);
    return 1;
    }

  vtkGraphEdgePropertyMapHelper<vtkDataArray*> weight_helper(edgeWeightArray);
  
  // Run the algorithm
  vtkstd::vector<vtkEdgeType> mstEdges;
  if (vtkDirectedGraph::SafeDownCast(output))
    {
    vtkDirectedGraph *g = vtkDirectedGraph::SafeDownCast(input);
    kruskal_minimum_spanning_tree(g, vtkstd::back_inserter(mstEdges), weight_map(weight_helper));
    }
  else
    {
    vtkUndirectedGraph *g = vtkUndirectedGraph::SafeDownCast(input);
    kruskal_minimum_spanning_tree(g, vtkstd::back_inserter(mstEdges), weight_map(weight_helper));
    }

  // Select the minimum spanning tree edges.
  if (!strcmp(OutputSelectionType,"MINIMUM_SPANNING_TREE_EDGES"))
  {
    vtkIdTypeArray* ids = vtkIdTypeArray::New();
    
    // Add the ids of each MST edge.
    for (vtkstd::vector<vtkEdgeType>::iterator i = mstEdges.begin();
         i != mstEdges.end(); ++i)
    {
      ids->InsertNextValue(i->Id);
    }
    
    output->SetSelectionList(ids);
    output->GetProperties()->Set(vtkSelection::CONTENT_TYPE(), 
                              vtkSelection::INDICES);
    output->GetProperties()->Set(vtkSelection::FIELD_TYPE(), 
                              vtkSelection::EDGE);
    ids->Delete();
  }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkBoostKruskalMinimumSpanningTree::FillInputPortInformation(
  int port, vtkInformation* info)
{
  // now add our info
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkBoostKruskalMinimumSpanningTree::FillOutputPortInformation(
  int port, vtkInformation* info)
{
  // now add our info
  if (port == 0)
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkSelection");
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkBoostKruskalMinimumSpanningTree::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "EdgeWeightArrayName: " 
     << (this->EdgeWeightArrayName ? this->EdgeWeightArrayName : "(none)") 
     << endl;

  os << indent << "OutputSelectionType: "
     << (this->OutputSelectionType ? this->OutputSelectionType : "(none)") << endl;
}

