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
#include "XdmfReader.hpp"
#include "XdmfRectilinearGrid.hpp"
#include "XdmfRegularGrid.hpp"
#include "XdmfUnstructuredGrid.hpp"
#include "XdmfTime.hpp"
#include "XdmfVisitor.hpp"

#include <set>

//TODO: multiblock
//TODO: time
//TODO: active attributes
//TODO: strides
//TODO: information elements
//TODO: test 2D cases
//TODO: test in parallel
//TODO: SIL
//TODO: enable/disable arrays, blocks
//TODO: domains

//=============================================================================
class vtkXdmfVisitor_GatherTimes : public XdmfVisitor
{
  //This traverses the hierarchy and prints out child types.
  //Just for diagnostic use.
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
    XdmfTime *timespec = dynamic_cast<XdmfTime *>(&item);
    if (timespec)
      {
      times.insert(timespec->getValue());
      }
    item.traverse(visitor);
  }

  std::set<double> getTimes()
  {
    return times;
  }

protected:

  std::set<double> times;

  vtkXdmfVisitor_GatherTimes() {}
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
    XdmfRegularGrid *regGrid = dynamic_cast<XdmfRegularGrid *>(&item);
    //TODO: strides
    if (regGrid)
      {
      vtkImageData *dataSet = vtkImageData::SafeDownCast(this->dataObject);
      vtkXdmf3RegularGrid::XdmfToVTK(regGrid, dataSet);
      }
    else
      {
      XdmfRectilinearGrid *recGrid = dynamic_cast<XdmfRectilinearGrid *>(&item);
      if (recGrid)
        {
        vtkRectilinearGrid *dataSet = vtkRectilinearGrid::SafeDownCast(this->dataObject);
        vtkXdmf3RectilinearGrid::XdmfToVTK(recGrid, dataSet);
        }
      else
        {
        XdmfCurvilinearGrid *crvGrid = dynamic_cast<XdmfCurvilinearGrid *>(&item);
        if (crvGrid)
          {
          vtkStructuredGrid *dataSet = vtkStructuredGrid::SafeDownCast(this->dataObject);
          vtkXdmf3CurvilinearGrid::XdmfToVTK(crvGrid, dataSet);
          }
        else
          {
          XdmfUnstructuredGrid *unsGrid = dynamic_cast<XdmfUnstructuredGrid *>(&item);
          if (unsGrid)
            {
            vtkUnstructuredGrid *dataSet = vtkUnstructuredGrid::SafeDownCast(this->dataObject);
            vtkXdmf3UnstructuredGrid::XdmfToVTK(unsGrid, dataSet);
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
              cerr << "other " << item.getItemTag() << endl;
              item.traverse(visitor);
              }
            }
          }
        }
      }
  }

  void SetOwner(vtkXdmf3Reader *self)
  {
    this->owner = self;
  }
  void SetDataObject(vtkDataObject *dobj)
  {
    this->dataObject = dobj;
  }

protected:

  vtkXdmfVisitor_ReadGrids() {}
  vtkXdmf3Reader *owner;
  vtkDataObject *dataObject;
};

//=============================================================================
class vtkXdmf3Reader::Internals {
public:
  Internals() {};
  ~Internals() {};

  void Init(const char *filename)
  {
    this->Reader = XdmfReader::New();
    this->Domain = shared_dynamic_cast<XdmfDomain>(
      this->Reader->read(filename)
      );
    this->VTKType = -1;
  }
  //--------------------------------------------------------------------------
  void GatherTimes()
  {
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
    if (this->VTKType != -1)
      {
      return this->VTKType;
      }
    unsigned int nGridCollections = this->Domain->getNumberGridCollections();
    unsigned int nUnstructuredGrids = this->Domain->getNumberUnstructuredGrids();
    unsigned int nRectilinearGrids = this->Domain->getNumberRectilinearGrids();
    unsigned int nCurvilinearGrids= this->Domain->getNumberCurvilinearGrids();
    unsigned int nRegularGrids = this->Domain->getNumberRegularGrids();
    unsigned int nGraphs = this->Domain->getNumberGraphs();
    if (nGridCollections != 0 ||
        (nUnstructuredGrids+
         nCurvilinearGrids+nRectilinearGrids+nRegularGrids+
         nGraphs
         !=1)
       )
      {
      this->VTKType = VTK_MULTIBLOCK_DATA_SET;
      }
    else
      {
      this->VTKType = VTK_UNIFORM_GRID;
      if (nCurvilinearGrids==1)
        {
        this->VTKType = VTK_STRUCTURED_GRID;
        }
      else if (nRectilinearGrids==1)
        {
        this->VTKType = VTK_RECTILINEAR_GRID;
        }
      else if (nUnstructuredGrids==1)
        {
        this->VTKType = VTK_UNSTRUCTURED_GRID;
        }
      else if (nGraphs == 1)
        {
        this->VTKType = VTK_DIRECTED_GRAPH; //VTK_MUTABLE_DIRECTED_GRAPH more specifically
        }
      }
     return this->VTKType;
  }

  //--------------------------------------------------------------------------
  bool PrepareDocument(vtkXdmf3Reader *self, const char *FileName)
  {
    // Calling this method repeatedly is okay. It does work only when something
    // has changed.

    //TODO Implement read from buffer path

    // Parse the file...
    if (!FileName )
      {
      vtkErrorWithObjectMacro(self, "File name not set");
      return false;
      }

    // First make sure the file exists.
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

  boost::shared_ptr<XdmfReader> Reader;
  boost::shared_ptr<XdmfDomain> Domain;
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

  //TODO: What about temporal collection of single grid?

  //Determine what vtkDataObject we should produce
  int vtk_type = this->Internal->GetVTKType();
  cerr << "vtk type is " << vtk_type << " " << vtkDataObjectTypes::GetClassNameFromTypeId(vtk_type) << endl;
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
    cerr << "TIMES " << timeRange[0] << "," << timeRange[1] << endl;
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
    }

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

    //TODO: temporal containing a dataset
    shared_ptr<XdmfRegularGrid> regGrid = this->Internal->Domain->getRegularGrid(0);
    if (regGrid)
      {
      vtkImageData *dataSet = vtkImageData::New();
      vtkXdmf3RegularGrid::CopyShape(regGrid.get(), dataSet);
      dataSet->GetExtent(whole_extent);
      dataSet->GetOrigin(origin);
      dataSet->GetSpacing(spacing);
      dataSet->Delete();
      }
    shared_ptr<XdmfRectilinearGrid> recGrid = this->Internal->Domain->getRectilinearGrid(0);
    if (recGrid)
      {
      vtkRectilinearGrid *dataSet = vtkRectilinearGrid::New();
      vtkXdmf3RectilinearGrid::CopyShape(recGrid.get(), dataSet);
      dataSet->GetExtent(whole_extent);
      dataSet->Delete();
      }
    shared_ptr<XdmfCurvilinearGrid> crvGrid = this->Internal->Domain->getCurvilinearGrid(0);
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

  // Collect information about what part of the data is requested.
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

  //TODO: Why is this needed?
  this->RequestDataObject(request, inputVector, outputVector);
  shared_ptr<vtkXdmfVisitor_ReadGrids> visitor = vtkXdmfVisitor_ReadGrids::New();
  visitor->SetOwner(this);
  vtkDataObject* output = vtkDataObject::GetData(outInfo);
  if (!output)
    {
    cerr << "UH OH" << endl;
    return 0;
    }
  visitor->SetDataObject(output);
  this->Internal->Domain->accept(visitor);

  return 1;
}
