/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXdmf3Reader.cxx
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


#include "vtkXdmf3Reader.h"

#include "vtksys/SystemTools.hxx"
#include "vtkCompositeDataPipeline.h"
#include "vtkDataObjectTypes.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXdmf3Common.h"

#include "XdmfCurvilinearGrid.hpp"
#include "XdmfDomain.hpp"
#include "XdmfGraph.hpp"
#include "XdmfGridCollection.hpp"
#include "XdmfGridCollectionType.hpp"
#include "XdmfReader.hpp"
#include "XdmfRectilinearGrid.hpp"
#include "XdmfRegularGrid.hpp"
#include "XdmfUnstructuredGrid.hpp"
#include "XdmfTime.hpp"
#include "XdmfVisitor.hpp"

#include <set>

//TODO: enable/disable arrays, blocks
//TODO: strides
//TODO: information elements
//TODO: test 2D cases
//TODO: test in parallel
//TODO: domains
//TODO: SIL

//=============================================================================
class vtkXdmfVisitor_GatherTimes : public XdmfVisitor
{
  //Traverses the xdmf hierarchy to build up the list of times it can serve.
public:

  static shared_ptr<vtkXdmfVisitor_GatherTimes>
  New()
  {
    shared_ptr<vtkXdmfVisitor_GatherTimes> p(new vtkXdmfVisitor_GatherTimes());
    return p;
  }

  ~vtkXdmfVisitor_GatherTimes() {}

  void
  visit(XdmfItem & item,
    const shared_ptr<XdmfBaseVisitor> visitor)
  {
    //make up default of 0..nchildren for temporal collection without times
    //TODO: handle range, list and slab timetypes similarly when they come back libxdmf3
    XdmfGridCollection *gc = dynamic_cast<XdmfGridCollection *>(&item);
    if (gc && gc->getType() == XdmfGridCollectionType::Temporal())
      {
      bool foundOne = false;
      unsigned int nGridCollections = gc->getNumberGridCollections();
      unsigned int nUnstructuredGrids = gc->getNumberUnstructuredGrids();
      unsigned int nRectilinearGrids = gc->getNumberRectilinearGrids();
      unsigned int nCurvilinearGrids= gc->getNumberCurvilinearGrids();
      unsigned int nRegularGrids = gc->getNumberRegularGrids();
      unsigned int nGraphs = gc->getNumberGraphs();
      for (unsigned int i = 0; i < nGridCollections; i++)
        {
        if (gc->getGridCollection(i)->getTime())
          {
          foundOne = true;
          break;
          }
        }
      for (unsigned int i = 0; i < nUnstructuredGrids; i++)
        {
        if (gc->getUnstructuredGrid(i)->getTime())
          {
          foundOne = true;
          break;
          }
        }
      for (unsigned int i = 0; i < nRectilinearGrids && !foundOne; i++)
        {
        if (gc->getRectilinearGrid(i)->getTime())
          {
          foundOne = true;
          break;
          }
        }
      for (unsigned int i = 0; i < nCurvilinearGrids && !foundOne; i++)
        {
        if (gc->getCurvilinearGrid(i)->getTime())
          {
          foundOne = true;
          break;
          }
        }
      for (unsigned int i = 0; i < nRegularGrids && !foundOne; i++)
        {
        if (gc->getRegularGrid(i)->getTime())
          {
          foundOne = true;
          break;
          }
        }
      /*TODO:: for... XdmfGraph->hasNoTime() */

      if (!foundOne)
        {
        //add implicit times to the immediate children to make searching for matching
        //times easier later on
        for (int i = 0;
            i < nGridCollections+nUnstructuredGrids+nRectilinearGrids+nCurvilinearGrids+nRegularGrids+nGraphs;
            i++)
          {
          times.insert(i);

          shared_ptr<XdmfTime> time = XdmfTime::New(i);
          if (gc->getGridCollection(i))
            {
            gc->getGridCollection(i)->setTime(time);
            }
          if (gc->getUnstructuredGrid(i))
            {
            gc->getUnstructuredGrid(i)->setTime(time);
            }
          if (gc->getRectilinearGrid(i))
            {
            gc->getRectilinearGrid(i)->setTime(time);
            }
          if (gc->getCurvilinearGrid(i))
            {
            gc->getCurvilinearGrid(i)->setTime(time);
            }
          if (gc->getRegularGrid(i))
            {
            gc->getRegularGrid(i)->setTime(time);
            }
          //TODO:: XdmfGraph->setTime
          }
        }
      }
    else
      {
      XdmfTime *timespec = dynamic_cast<XdmfTime *>(&item);
      if (timespec)
        {
        times.insert(timespec->getValue());
        }
      }

    item.traverse(visitor);
  }

  std::set<double> getTimes()
  {
    return times;
  }

protected:

  vtkXdmfVisitor_GatherTimes() {}
  std::set<double> times; //semi-relying on implicit sort from set<double>

};

//=============================================================================
class vtkXdmfVisitor_ReadGrids
{
  //This traverses the hierarchy and reads each grid.
public:

  static shared_ptr<vtkXdmfVisitor_ReadGrids> New()
  {
    shared_ptr<vtkXdmfVisitor_ReadGrids> p(new vtkXdmfVisitor_ReadGrids());
    return p;
  }

  ~vtkXdmfVisitor_ReadGrids()
  {
  }

  vtkDataObject *Populate(XdmfItem & item, vtkDataObject *toFill)
  {
    assert(toFill);
    cerr << "populate " << toFill << " " << toFill->GetClassName() << endl;

    XdmfRegularGrid *regGrid = dynamic_cast<XdmfRegularGrid *>(&item);
    if (regGrid)
      {
      vtkImageData *dataSet = vtkImageData::SafeDownCast(toFill);
      //cerr << "ID" << endl;
      if (!this->doTime ||
          (regGrid->getTime() && regGrid->getTime()->getValue() == this->time))
        {
        vtkXdmf3RegularGrid::XdmfToVTK(regGrid, dataSet);
        return dataSet;
        }
      return NULL;
      }

    XdmfRectilinearGrid *recGrid = dynamic_cast<XdmfRectilinearGrid *>(&item);
    if (recGrid)
      {
      //cerr << "RGRID" << endl;
      if (!this->doTime ||
          (recGrid->getTime() && recGrid->getTime()->getValue() == this->time))
        {
        vtkRectilinearGrid *dataSet = vtkRectilinearGrid::SafeDownCast(toFill);
        vtkXdmf3RectilinearGrid::XdmfToVTK(recGrid, dataSet);
        return dataSet;
        }
      return NULL;
      }

    XdmfCurvilinearGrid *crvGrid = dynamic_cast<XdmfCurvilinearGrid *>(&item);
    if (crvGrid)
      {
      //cerr << "SGRID" << endl;
      if (!this->doTime ||
          (crvGrid->getTime() && crvGrid->getTime()->getValue() == this->time))
        {
        vtkStructuredGrid *dataSet =  vtkStructuredGrid::SafeDownCast(toFill);
        vtkXdmf3CurvilinearGrid::XdmfToVTK(crvGrid, dataSet);
        return dataSet;
        }
      return NULL;
      }

    XdmfUnstructuredGrid *unsGrid = dynamic_cast<XdmfUnstructuredGrid *>(&item);
    if (unsGrid)
      {
      //cerr << "UNS" << endl;
      if (!this->doTime ||
          (unsGrid->getTime() && unsGrid->getTime()->getValue() == this->time))
        {
        vtkUnstructuredGrid *dataSet = vtkUnstructuredGrid::SafeDownCast(toFill);
        vtkXdmf3UnstructuredGrid::XdmfToVTK(unsGrid, dataSet);
        return dataSet;
        }
      return NULL;
      }

    XdmfGraph *graph = dynamic_cast<XdmfGraph *>(&item);
    if (graph)
      {
      //cerr << "GRAPH" << endl;
      //TODO: XdmfGraph has no getTime() yet
      vtkMutableDirectedGraph *dataSet = vtkMutableDirectedGraph::SafeDownCast(toFill);
      vtkXdmf3Graph::XdmfToVTK(graph, dataSet);
      return dataSet;
      }

    XdmfDomain *group = dynamic_cast<XdmfDomain *>(&item);
    if (group)
      {
      XdmfDomain *toCheck = group;
      unsigned int nGridCollections = toCheck->getNumberGridCollections();
      XdmfGridCollection *asGC = dynamic_cast<XdmfGridCollection *>(&item);
      int gtype = -1;
      if (!asGC)
        {
        gtype = 0;
        cerr << "DOMAIN" << endl;
        }
      else
        {
        if (asGC->getType() == XdmfGridCollectionType::Temporal())
          {
          gtype = 1;
          cerr << "TEMPORAL GROUP" << endl;
          }
        else if (asGC->getType() == XdmfGridCollectionType::Spatial())
          {
          gtype = 2;
          cerr << "SPATIAL GROUP" << endl;
          }
        else
          {
          gtype = 3;
          cerr << "UNSPECIFIED GROUP" << endl;
          }
        }

      bool stacked = false;
      if (this->doTime && gtype > 1 && asGC->getTime())
        {
        if (asGC->getTime()->getValue() != this->time)
          {
          //don't return MB that don't match the requested time
          return NULL;
          }
        //when we find a match, turn off time request, so that
        //we get everything underneath
        this->doTime = false;
        stacked = true;
        }

      if (nGridCollections == 1 &&
          toCheck->getGridCollection(0)->getType() == XdmfGridCollectionType::Temporal())
        {
        //skip over temporal collections and return their sieved content
        toCheck = group->getGridCollection(0).get();
        }

      vtkMultiBlockDataSet *top = vtkMultiBlockDataSet::SafeDownCast(toFill);
      vtkDataObject *result;
      int cnt = 0;
      nGridCollections = toCheck->getNumberGridCollections();
      unsigned int nUnstructuredGrids = toCheck->getNumberUnstructuredGrids();
      unsigned int nRectilinearGrids = toCheck->getNumberRectilinearGrids();
      unsigned int nCurvilinearGrids= toCheck->getNumberCurvilinearGrids();
      unsigned int nRegularGrids = toCheck->getNumberRegularGrids();
      unsigned int nGraphs = toCheck->getNumberGraphs();
      for (unsigned int i = 0; i < nGridCollections; i++)
        {
        vtkMultiBlockDataSet *child = vtkMultiBlockDataSet::New();
        result = this->Populate(*(toCheck->getGridCollection(i)), child);
        if (result)
          {
          top->SetBlock(cnt++, result);
          }
        child->Delete();
        }
      for (unsigned int i = 0; i < nUnstructuredGrids; i++)
        {
        vtkUnstructuredGrid *child = vtkUnstructuredGrid::New();
        result = this->Populate(*(toCheck->getUnstructuredGrid(i)), child);
        if (result)
          {
          top->SetBlock(cnt++, result);
          }
        child->Delete();
        }
      for (unsigned int i = 0; i < nRectilinearGrids; i++)
        {
        vtkRectilinearGrid *child = vtkRectilinearGrid::New();
        result = this->Populate(*(toCheck->getRectilinearGrid(i)), child);
        if (result)
          {
          top->SetBlock(cnt++, result);
          }
        child->Delete();
        }
      for (unsigned int i = 0; i < nCurvilinearGrids; i++)
        {
        vtkStructuredGrid *child = vtkStructuredGrid::New();
        result = this->Populate(*(toCheck->getCurvilinearGrid(i)), child);
        if (result)
          {
          top->SetBlock(cnt++, result);
          }
        child->Delete();
        }
      for (unsigned int i = 0; i < nRegularGrids; i++)
        {
        vtkUniformGrid *child = vtkUniformGrid::New();
        result = this->Populate(*(group->getRegularGrid(i)), child);
        if (result)
          {
          top->SetBlock(cnt++, result);
          }
        child->Delete();
        }
      for (unsigned int i = 0; i < nGraphs; i++)
        {
        vtkMutableDirectedGraph *child = vtkMutableDirectedGraph::New();
        result = this->Populate(*(group->getGraph(i)), child);
        if (result)
          {
          top->SetBlock(cnt++, result);
          }
        child->Delete();
        }

      if (stacked)
        {
        //restore time search now that we've done the group contents
        doTime = true;
        }
      return top;
      }

    //shoule never get here
    return NULL;
  }

  void SetTimeRequest(bool dt, double t)
  {
    this->doTime = dt;
    this->time = t;
  }

protected:
  vtkXdmfVisitor_ReadGrids()
  {
    this->doTime = false;
  }
  bool doTime;
  double time;
};

//=============================================================================
class vtkXdmf3Reader::Internals {
  //Private implementation details for vtkXdmf3Reader
public:
  Internals() {};
  ~Internals() {};
  //--------------------------------------------------------------------------
  bool PrepareDocument(vtkXdmf3Reader *self, const char *FileName)
  {
    // Calling this method repeatedly is okay. It does work only when something
    // has changed.
    //TODO Implement read from buffer path
    if (!FileName )
      {
      vtkErrorWithObjectMacro(self, "File name not set");
      return false;
      }
    if (!vtksys::SystemTools::FileExists(FileName))
      {
      vtkErrorWithObjectMacro(self, "Error opening file " << FileName);
      return false;
      }
    if (!this->Domain)
      {
      cerr << "READING XML" << endl;
      this->Init(FileName);
      }
    return true;
  }
  //--------------------------------------------------------------------------
  void Init(const char *filename)
  {
    this->Reader = XdmfReader::New();
    //TODO:
    //Domains are no longer used in practice.
    //So we should handle files without domains.
    this->Domain = shared_dynamic_cast<XdmfDomain>(
      this->Reader->read(filename)
      );
    //this->TopGrid = NULL;
    this->VTKType = -1;
  }
  //--------------------------------------------------------------------------
  void GatherTimes()
  {
    //ask XDMF for the times it can provide data for
    if (this->TimeSteps.size())
      {
      this->TimeSteps.erase(this->TimeSteps.begin());
      }
    shared_ptr<vtkXdmfVisitor_GatherTimes> visitor = vtkXdmfVisitor_GatherTimes::New();
    this->Domain->accept(visitor);
    std::set<double> times = visitor->getTimes();
    std::set<double>::const_iterator it; // declare an iterator
    it = times.begin();
    while (it != times.end())
      {
      this->TimeSteps.push_back(*it);
      it++;
      }
  }
  //--------------------------------------------------------------------------
  int GetVTKType()
  {
    //find out what kind of vtkdataobject we should make
    if (this->VTKType != -1)
      {
      return this->VTKType;
      }
    unsigned int nGridCollections = this->Domain->getNumberGridCollections();
    shared_ptr<XdmfDomain> toCheck = this->Domain;

    //check for temporal of atomic, act on the atomic type
    bool temporal = false;
    if (nGridCollections == 1)
      {
      shared_ptr<XdmfGridCollection> gc = this->Domain->getGridCollection(0);
      if (gc->getType() == XdmfGridCollectionType::Temporal())
        {
        if (gc->getNumberGridCollections() == 0)
          {
          temporal = true;
          toCheck = gc;
          }
        }
      }
    unsigned int nUnstructuredGrids = toCheck->getNumberUnstructuredGrids();
    unsigned int nRectilinearGrids = toCheck->getNumberRectilinearGrids();
    unsigned int nCurvilinearGrids= toCheck->getNumberCurvilinearGrids();
    unsigned int nRegularGrids = toCheck->getNumberRegularGrids();
    unsigned int nGraphs = toCheck->getNumberGraphs();
    int numtypes = 0;
    numtypes = numtypes + (nUnstructuredGrids>0?1:0);
    numtypes = numtypes + (nRectilinearGrids>0?1:0);
    numtypes = numtypes + (nCurvilinearGrids>0?1:0);
    numtypes = numtypes + (nRegularGrids>0?1:0);
    numtypes = numtypes + (nGraphs>0?1:0);
    bool atomic =
        temporal ||
        (numtypes==1 &&
          (
          nUnstructuredGrids==1||
          nRectilinearGrids==1||
          nCurvilinearGrids==1||
          nRegularGrids==1||
          nGraphs==1));
    if (!atomic
        )
      {
      this->VTKType = VTK_MULTIBLOCK_DATA_SET;
      }
    else
      {
      this->VTKType = VTK_UNIFORM_GRID;
      this->TopGrid = toCheck->getRegularGrid(0); //keep a reference to get extent from
      if (nRectilinearGrids>0)
        {
        this->VTKType = VTK_RECTILINEAR_GRID;
        this->TopGrid = toCheck->getRectilinearGrid(0);//keep a reference to get extent from
        }
      else if (nCurvilinearGrids>0)
        {
        this->VTKType = VTK_STRUCTURED_GRID;
        this->TopGrid = toCheck->getCurvilinearGrid(0);//keep a reference to get extent from
        }
      else if (nUnstructuredGrids>0)
        {
        this->VTKType = VTK_UNSTRUCTURED_GRID;
        }
      else if (nGraphs>0)
        {
        this->VTKType = VTK_DIRECTED_GRAPH; //VTK_MUTABLE_DIRECTED_GRAPH more specifically
        }
      }
     return this->VTKType;
  }

  boost::shared_ptr<XdmfReader> Reader;
  boost::shared_ptr<XdmfDomain> Domain;
  boost::shared_ptr<XdmfItem> TopGrid;
  int VTKType;
  std::vector<double> TimeSteps;
  };

//==============================================================================

vtkStandardNewMacro(vtkXdmf3Reader);

//----------------------------------------------------------------------------
vtkXdmf3Reader::vtkXdmf3Reader()
{
  this->FileName = NULL;
  this->Internal = new vtkXdmf3Reader::Internals();
}

//----------------------------------------------------------------------------
vtkXdmf3Reader::~vtkXdmf3Reader()
{
  this->SetFileName(NULL);
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkXdmf3Reader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << endl;
}

//----------------------------------------------------------------------------
int vtkXdmf3Reader::CanReadFile(const char* filename)
{
  if (!vtksys::SystemTools::FileExists(filename))
    {
    return 0;
    }

  shared_ptr<XdmfReader> tester = XdmfReader::New();
  try {
    shared_ptr<XdmfItem> item = tester->read(filename);
  } catch (...) {
    //TODO: xdmf crashes when I give it something it can't handle and we never get here.
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkXdmf3Reader::FillOutputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataObject");
  return 1;
}

//----------------------------------------------------------------------------
int vtkXdmf3Reader::ProcessRequest(vtkInformation *request,
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // create the output
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
    return this->RequestDataObject(outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkXdmf3Reader::RequestDataObject(vtkInformationVector *outputVector)
{
  //let libXdmf parse XML
  if (!this->Internal->PrepareDocument(this, this->FileName))
    {
    return 0;
    }

  //Determine what vtkDataObject we should produce
  int vtk_type = this->Internal->GetVTKType();
  cerr << "vtk type is " << vtk_type << " "
       << vtkDataObjectTypes::GetClassNameFromTypeId(vtk_type) << endl;
  //Make an empty vtkDataObject
  vtkDataObject* output = vtkDataObject::GetData(outputVector, 0);
  if (!output || output->GetDataObjectType() != vtk_type)
    {
    if (vtk_type == VTK_DIRECTED_GRAPH)
      {
      output = vtkMutableDirectedGraph::New();
      }
    else
      {
      output = vtkDataObjectTypes::NewDataObject(vtk_type);
      }
    cerr << "MADE " << output << " A " << output->GetClassName() << endl;
    outputVector->GetInformationObject(0)->Set(
        vtkDataObject::DATA_OBJECT(), output );
    this->GetOutputPortInformation(0)->Set(
      vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType());
    output->Delete();
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkXdmf3Reader::RequestInformation(vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *outputVector)
{
  if (!this->Internal->PrepareDocument(this, this->FileName))
    {
    return 0;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // Publish the fact that this reader can satisfy any piece request.
  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

  // Determine and announce what times we have to offer.
  this->Internal->GatherTimes();
  if (this->Internal->TimeSteps.size() > 0)
    {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &this->Internal->TimeSteps[0],
      static_cast<int>(this->Internal->TimeSteps.size()));
    double timeRange[2];
    timeRange[0] = this->Internal->TimeSteps.front();
    timeRange[1] = this->Internal->TimeSteps.back();
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
    }

  // Structured atomic must announce the whole extent it can provide
  int vtk_type = this->Internal->GetVTKType();
  if (vtk_type == VTK_STRUCTURED_GRID ||
      vtk_type == VTK_RECTILINEAR_GRID ||
      vtk_type == VTK_IMAGE_DATA ||
      vtk_type == VTK_UNIFORM_GRID)
    {
    int whole_extent[6];
    double origin[3];
    double spacing[3];
    whole_extent[0] = 0;
    whole_extent[1] = -1;
    whole_extent[2] = 0;
    whole_extent[3] = -1;
    whole_extent[4] = 0;
    whole_extent[5] = -1;
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;
    spacing[0] = 1.0;
    spacing[1] = 1.0;
    spacing[2] = 1.0;

    shared_ptr<XdmfRegularGrid> regGrid =
      shared_dynamic_cast<XdmfRegularGrid>(this->Internal->TopGrid);
    if (regGrid)
      {
      vtkImageData *dataSet = vtkImageData::New();
      vtkXdmf3RegularGrid::CopyShape(regGrid.get(), dataSet);
      dataSet->GetExtent(whole_extent);
      dataSet->GetOrigin(origin);
      dataSet->GetSpacing(spacing);
      dataSet->Delete();
      }
    shared_ptr<XdmfRectilinearGrid> recGrid =
      shared_dynamic_cast<XdmfRectilinearGrid>(this->Internal->TopGrid);
    if (recGrid)
      {
      vtkRectilinearGrid *dataSet = vtkRectilinearGrid::New();
      vtkXdmf3RectilinearGrid::CopyShape(recGrid.get(), dataSet);
      dataSet->GetExtent(whole_extent);
      dataSet->Delete();
      }
    shared_ptr<XdmfCurvilinearGrid> crvGrid =
      shared_dynamic_cast<XdmfCurvilinearGrid>(this->Internal->TopGrid);
    if (crvGrid)
      {
      vtkStructuredGrid *dataSet = vtkStructuredGrid::New();
      vtkXdmf3CurvilinearGrid::CopyShape(crvGrid.get(), dataSet);
      dataSet->GetExtent(whole_extent);
      dataSet->Delete();
      }

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
        whole_extent, 6);
    outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);
    outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
    }

  //TODO: SIL

  return 1;
}

//----------------------------------------------------------------------------
int vtkXdmf3Reader::RequestData(vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if (!this->Internal->PrepareDocument(this, this->FileName))
    {
    return 0;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // Collect information about what spatial extent is requested.
  unsigned int updatePiece = 0;
  unsigned int updateNumPieces = 1;
  int ghost_levels = 0;
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) &&
      outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()))
    {
    updatePiece = static_cast<unsigned int>(
        outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
    updateNumPieces =  static_cast<unsigned int>(
        outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
    }
  if (outInfo->Has(
      vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()))
    {
    ghost_levels = outInfo->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());
    }
  int update_extent[6] = {0, -1, 0, -1, 0, -1};
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()))
    {
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
        update_extent);
    }

  // Collect information about what temporal extent is requested.
  double time;
  bool doTime = false;
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()) &&
      this->Internal->TimeSteps.size())
    {
    doTime = true;
    time =
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    //find the nearest match (floor), so we have something exact to search for
    std::vector<double>::iterator it = upper_bound(
      this->Internal->TimeSteps.begin(), this->Internal->TimeSteps.end(), time);
    if (it != this->Internal->TimeSteps.begin())
      {
      it--;
      }
    time = *it;
    }

  //traverse the xdmf hierarchy, and convert and return what was requested
  shared_ptr<vtkXdmfVisitor_ReadGrids> visitor = vtkXdmfVisitor_ReadGrids::New();
  vtkDataObject* output = vtkDataObject::GetData(outInfo);
  if (!output)
    {
    return 0;
    }
  if (doTime)
    {
    output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEP(), time);
    }

  vtkMultiBlockDataSet *mbds = vtkMultiBlockDataSet::New();
  cerr << "MAKING TEMPSTORE " << mbds << endl;
  visitor->SetTimeRequest(doTime, time);
  visitor->Populate(*(this->Internal->Domain), mbds);
  //mbds->PrintSelf(cerr, vtkIndent(0));
  if (mbds->GetNumberOfBlocks()==1)
    {
    cerr << "COPY ONE\n " << mbds->GetBlock(0) << " " << mbds->GetBlock(0)->GetClassName() << " INTO " << output << " " << output->GetClassName() << endl;
    output->ShallowCopy(mbds->GetBlock(0));
    }
  else
    {
    cerr << "COPY ALL\n " << mbds << " " << mbds->GetClassName() << " INTO " << output << " " << output->GetClassName() << endl;
    output->ShallowCopy(mbds);
    }
  mbds->Delete();

  return 1;
}
