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
#include "vtkMutableDirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
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

#define DEBUG_VXDMF3 0
#if DEBUG_VXDMF3
//these two are just here for debug code
#include "vtkPointData.h"
#include "vtkDataArray.h"
#endif

#include <set>

//TODO: multiblock
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
    //TODO: handle range, list and slab timetypes similarly when they come to libxdmf3
    XdmfGridCollection *gc = dynamic_cast<XdmfGridCollection *>(&item);
    if (gc && gc->getType() == XdmfGridCollectionType::Temporal())
      {
      bool foundOne = false;
      unsigned int nUnstructuredGrids = gc->getNumberUnstructuredGrids();
      unsigned int nRectilinearGrids = gc->getNumberRectilinearGrids();
      unsigned int nCurvilinearGrids= gc->getNumberCurvilinearGrids();
      unsigned int nRegularGrids = gc->getNumberRegularGrids();
      unsigned int nGraphs = gc->getNumberGraphs();
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
            i < nUnstructuredGrids+nRectilinearGrids+nCurvilinearGrids+nRegularGrids+nGraphs;
            i++)
          {
          shared_ptr<XdmfTime> time = XdmfTime::New(i);
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
          times.insert(i);
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
class vtkXdmfVisitor_ReadGrids : public XdmfVisitor
{
  //This traverses the hierarchy and reads each grid.
public:

  static shared_ptr<vtkXdmfVisitor_ReadGrids>
  New()
  {
    shared_ptr<vtkXdmfVisitor_ReadGrids> p(new vtkXdmfVisitor_ReadGrids());
    return p;
  }

  ~vtkXdmfVisitor_ReadGrids()
  {
  }

  void
  visit(XdmfItem & item,
    const shared_ptr<XdmfBaseVisitor> visitor)
  {
    //TODO: return ONLY requested time
    XdmfRegularGrid *regGrid = dynamic_cast<XdmfRegularGrid *>(&item);
    if (regGrid)
      {
      if (!this->doTime ||
          (regGrid->getTime() && regGrid->getTime()->getValue() == this->time)
          )
        {
        vtkImageData *dataSet = vtkImageData::SafeDownCast(this->dataObject);
        vtkXdmf3RegularGrid::XdmfToVTK(regGrid, dataSet);
        }
      }
    else
      {
      XdmfRectilinearGrid *recGrid = dynamic_cast<XdmfRectilinearGrid *>(&item);
      if (recGrid)
        {
        if (!this->doTime ||
            (recGrid->getTime() && recGrid->getTime()->getValue() == this->time)
            )
          {
          vtkRectilinearGrid *dataSet = vtkRectilinearGrid::SafeDownCast(this->dataObject);
          vtkXdmf3RectilinearGrid::XdmfToVTK(recGrid, dataSet);
          }
        }
      else
        {
        XdmfCurvilinearGrid *crvGrid = dynamic_cast<XdmfCurvilinearGrid *>(&item);
        if (crvGrid)
          {
          if (!this->doTime ||
              (crvGrid->getTime() && crvGrid->getTime()->getValue() == this->time)
              )
            {
            vtkStructuredGrid *dataSet = vtkStructuredGrid::SafeDownCast(this->dataObject);
            vtkXdmf3CurvilinearGrid::XdmfToVTK(crvGrid, dataSet);
            }
          }
        else
          {
          XdmfUnstructuredGrid *unsGrid = dynamic_cast<XdmfUnstructuredGrid *>(&item);
          if (unsGrid)
            {
            if (!this->doTime ||
                (unsGrid->getTime() && unsGrid->getTime()->getValue() == this->time)
                )
              {
              vtkUnstructuredGrid *dataSet = vtkUnstructuredGrid::SafeDownCast(this->dataObject);
              vtkXdmf3UnstructuredGrid::XdmfToVTK(unsGrid, dataSet);
#if DEBUG_VXDMF3
              cerr << "UGPD0 " << *dataSet->GetPointData()->GetArray("NodeScalar")->GetTuple(0) << endl;
#endif
              }
            }
          else
            {
            XdmfGraph *graph = dynamic_cast<XdmfGraph *>(&item);
            if (graph)
              {
              vtkMutableDirectedGraph *dataSet = vtkMutableDirectedGraph::SafeDownCast(this->dataObject);
              vtkXdmf3Graph::XdmfToVTK(graph, dataSet);
              }
            else
              {
              //cerr << "other " << item.getItemTag() << endl;
              item.traverse(visitor);
              }
            }
          }
        }
      }
  }

  void SetDataObject(vtkDataObject *dobj)
  {
    this->dataObject = dobj;
  }
  void SetTimeRequest(bool dt, double t)
  {
    this->doTime = dt;
    this->time = t;
  }

protected:
  vtkXdmfVisitor_ReadGrids()
  {
    this->dataObject = NULL;
    this->doTime = false;
  }
  vtkDataObject *dataObject;
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
    //Domains are no longer used. Ken wants us to handle files without domains.
    this->Domain = shared_dynamic_cast<XdmfDomain>(
      this->Reader->read(filename)
      );
    //this->TopAtomicGrid = NULL;
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
    shared_ptr<XdmfDomain> toCheck = this->Domain;
    //this->TopAtomicGrid = NULL;
    unsigned int nGridCollections = toCheck->getNumberGridCollections();

    //check for temporal of atomic, in that case return atomic type
    bool temporal = false;
    if (nGridCollections == 1)
      {
      shared_ptr<XdmfGridCollection> gc = toCheck->getGridCollection(0);
      if (gc->getType() == XdmfGridCollectionType::Temporal() &&
          gc->getNumberGridCollections() == 0)
        {
        temporal = true;
        toCheck = gc;
        nGridCollections = 0;
        }
      }
    unsigned int nUnstructuredGrids = toCheck->getNumberUnstructuredGrids();
    unsigned int nRectilinearGrids = toCheck->getNumberRectilinearGrids();
    unsigned int nCurvilinearGrids= toCheck->getNumberCurvilinearGrids();
    unsigned int nRegularGrids = toCheck->getNumberRegularGrids();
    unsigned int nGraphs = toCheck->getNumberGraphs();
    int numTypes = 0;
    numTypes = numTypes + (nUnstructuredGrids!=0?1:0);
    numTypes = numTypes + (nRectilinearGrids!=0?1:0);
    numTypes = numTypes + (nCurvilinearGrids!=0?1:0);
    numTypes = numTypes + (nRegularGrids!=0?1:0);
    numTypes = numTypes + (nGraphs!=0?1:0);
    if (!(nGridCollections == 0 ||
         (numTypes==1 && temporal) ||
         (nUnstructuredGrids+
          nCurvilinearGrids+nRectilinearGrids+nRegularGrids+
          nGraphs
          ==1)
         )
       )
      {
      this->VTKType = VTK_MULTIBLOCK_DATA_SET;
      }
    else
      {
      this->VTKType = VTK_UNIFORM_GRID;
      this->TopAtomicGrid = toCheck->getRegularGrid(0);
      if (nCurvilinearGrids>0)
        {
        this->VTKType = VTK_STRUCTURED_GRID;
        this->TopAtomicGrid = toCheck->getCurvilinearGrid(0);
        }
      else if (nRectilinearGrids>0)
        {
        this->VTKType = VTK_RECTILINEAR_GRID;
        this->TopAtomicGrid = toCheck->getRectilinearGrid(0);
        }
      else if (nUnstructuredGrids>0)
        {
        this->VTKType = VTK_UNSTRUCTURED_GRID;
        this->TopAtomicGrid = toCheck->getUnstructuredGrid(0);
        }
      else if (nGraphs>0)
        {
        this->VTKType = VTK_DIRECTED_GRAPH; //VTK_MUTABLE_DIRECTED_GRAPH more specifically
        this->TopAtomicGrid = toCheck->getGraph(0);
        }
      }
     return this->VTKType;
  }

  boost::shared_ptr<XdmfReader> Reader;
  boost::shared_ptr<XdmfDomain> Domain;
  boost::shared_ptr<XdmfItem> TopAtomicGrid;
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
int vtkXdmf3Reader::RequestDataObject(vtkInformation *,
    vtkInformationVector **,
    vtkInformationVector *outputVector)
{
  //TODO: Why is this never called?

  //let libXdmf parse XML
  cerr << "RDO?" << endl;
  if (!this->Internal->PrepareDocument(this, this->FileName))
    {
    return 0;
    }
  cerr << "RDO!" << endl;

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
  cerr << "RI?" << endl;
  if (!this->Internal->PrepareDocument(this, this->FileName))
    {
    return 0;
    }
  cerr << "RI!" << endl;

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
      shared_dynamic_cast<XdmfRegularGrid>(this->Internal->TopAtomicGrid);
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
      shared_dynamic_cast<XdmfRectilinearGrid>(this->Internal->TopAtomicGrid);
    if (recGrid)
      {
      vtkRectilinearGrid *dataSet = vtkRectilinearGrid::New();
      vtkXdmf3RectilinearGrid::CopyShape(recGrid.get(), dataSet);
      dataSet->GetExtent(whole_extent);
      dataSet->Delete();
      }
    shared_ptr<XdmfCurvilinearGrid> crvGrid =
      shared_dynamic_cast<XdmfCurvilinearGrid>(this->Internal->TopAtomicGrid);
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
  cerr << "RD?" << endl;
  if (!this->Internal->PrepareDocument(this, this->FileName))
    {
    return 0;
    }
  cerr << "RD!" << endl;

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
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
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

  //TODO: Why is this needed?
  this->RequestDataObject(request, inputVector, outputVector);

  //traverse the xdmf hierarchy, and convert and return what was requested
  shared_ptr<vtkXdmfVisitor_ReadGrids> visitor = vtkXdmfVisitor_ReadGrids::New();
  vtkDataObject* output = vtkDataObject::GetData(outInfo);
  if (!output)
    {
    cerr << "UH OH" << endl;
    return 0;
    }
  output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEP(), time);
  visitor->SetDataObject(output);
  visitor->SetTimeRequest(doTime, time);
  this->Internal->Domain->accept(visitor);

  return 1;
}
