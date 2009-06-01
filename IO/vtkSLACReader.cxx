// -*- c++ -*-
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSLACReader.cxx

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

#include "vtkSLACReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataArraySelection.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationVector.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkMath.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"

#include "vtkSmartPointer.h"
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#include <netcdf.h>

#include <vtksys/RegularExpression.hxx>
#include <math.h>

using namespace vtkstd;

//=============================================================================
#define CALL_NETCDF(call)                       \
  { \
    int errorcode = call; \
    if (errorcode != NC_NOERR) \
      { \
      vtkErrorMacro(<< "netCDF Error: " << nc_strerror(errorcode)); \
      return 0; \
      } \
  }

#define WRAP_NETCDF(call) \
  { \
    int errorcode = call; \
    if (errorcode != NC_NOERR) return errorcode; \
  }

//-----------------------------------------------------------------------------
#ifdef VTK_USE_64BIT_IDS
#ifdef NC_INT64
// This may or may not work with the netCDF 4 library reading in netCDF 3 files.
#define nc_get_var_vtkIdType nc_get_var_longlong
#else // NC_INT64
static int nc_get_var_vtkIdType(int ncid, int varid, vtkIdType *ip)
{
  // Step 1, figure out how many entries in the given variable.
  int numdims, dimids[NC_MAX_VAR_DIMS];
  WRAP_NETCDF(nc_inq_varndims(ncid, varid, &numdims));
  WRAP_NETCDF(nc_inq_vardimid(ncid, varid, dimids));
  vtkIdType numValues = 1;
  for (int dim = 0; dim < numdims; dim++)
    {
    size_t dimlen;
    WRAP_NETCDF(nc_inq_dimlen(ncid, dimids[dim], &dimlen));
    numValues *= dimlen;
    }

  // Step 2, read the data in as 32 bit integers.  Recast the input buffer
  // so we do not have to create a new one.
  long *smallIp = reinterpret_cast<long*>(ip);
  WRAP_NETCDF(nc_get_var_long(ncid, varid, smallIp));

  // Step 3, recast the data from 32 bit integers to 64 bit integers.  Since we
  // are storing both in the same buffer, we need to be careful to not overwrite
  // uncopied 32 bit numbers with 64 bit numbers.  We can do that by copying
  // backwards.
  for (vtkIdType i = numValues-1; i >= 0; i--)
    {
    ip[i] = static_cast<vtkIdType>(smallIp[i]);
    }

  return NC_NOERR;
}
#endif // NC_INT64
#else // VTK_USE_64_BIT_IDS
#define nc_get_var_vtkIdType nc_get_var_int
#endif // VTK_USE_64BIT_IDS

//-----------------------------------------------------------------------------
// This convenience function gets a scalar variable as a double, doing the
// appropriate checks.
static int nc_get_scalar_double(int ncid, const char *name, double *dp)
{
  int varid;
  WRAP_NETCDF(nc_inq_varid(ncid, name, &varid));
  int numdims;
  WRAP_NETCDF(nc_inq_varndims(ncid, varid, &numdims));
  if (numdims != 0)
    {
    // Not a great error to return, but better than nothing.
    return NC_EVARSIZE;
    }
  WRAP_NETCDF(nc_get_var_double(ncid, varid, dp));

  return NC_NOERR;
}

//=============================================================================
// Describes how faces are defined in a tetrahedra in the files.
const int tetFaces[4][3] = {
  { 0, 2, 1 },
  { 0, 3, 2 },
  { 0, 1, 3 },
  { 1, 2, 3 }
};

// Describes the points on each edge of a VTK triangle.  The edges are in the
// same order as the midpoints are defined in a VTK quadratic triangle.
const int triEdges[3][2] = {
  { 0, 1 },
  { 1, 2 },
  { 0, 2 }
};

//=============================================================================
#define MY_MIN(x, y)    ((x) < (y) ? (x) : (y))
#define MY_MAX(x, y)    ((x) < (y) ? (y) : (x))

//=============================================================================
static int NetCDFTypeToVTKType(nc_type type)
{
  switch (type)
    {
    case NC_BYTE: return VTK_UNSIGNED_CHAR;
    case NC_CHAR: return VTK_CHAR;
    case NC_SHORT: return VTK_SHORT;
    case NC_INT: return VTK_INT;
    case NC_FLOAT: return VTK_FLOAT;
    case NC_DOUBLE: return VTK_DOUBLE;
    default:
      vtkGenericWarningMacro(<< "Unknown netCDF variable type "
                             << type);
      return -1;
    }
}

//=============================================================================
// This class automatically closes a netCDF file descripter when it goes out
// of scope.  This allows us to exit on error without having to close the
// file at every instance.
class vtkSLACReaderAutoCloseNetCDF
{
public:
  vtkSLACReaderAutoCloseNetCDF(const char *filename, int omode,
                               bool quiet=false) {
    int errorcode = nc_open(filename, omode, &this->fd);
    if (errorcode != NC_NOERR)
      {
      if (!quiet)
        {
        vtkGenericWarningMacro(<< "Could not open " << filename << endl
                               << nc_strerror(errorcode));
        }
      this->fd = -1;
      }
  }
  ~vtkSLACReaderAutoCloseNetCDF() {
    if (this->fd != -1)
      {
      nc_close(this->fd);
      }
  }
  int operator()() const { return this->fd; }
  bool Valid() const { return this->fd != -1; }
protected:
  int fd;
private:
  vtkSLACReaderAutoCloseNetCDF();       // Not implemented
  vtkSLACReaderAutoCloseNetCDF(const vtkSLACReaderAutoCloseNetCDF &); // Not implemented
  void operator=(const vtkSLACReaderAutoCloseNetCDF &); // Not implemented
};

//=============================================================================
// A convenience function that gets a block from a multiblock data set,
// performing allocation if necessary.
static vtkUnstructuredGrid *AllocateGetBlock(vtkMultiBlockDataSet *blocks,
                                             unsigned int blockno,
                                             vtkInformationIntegerKey *typeKey)
{
  if (blockno > 1000)
    {
    vtkGenericWarningMacro(<< "Unexpected block number: " << blockno);
    blockno = 0;
    }

  if (blocks->GetNumberOfBlocks() <= blockno)
    {
    blocks->SetNumberOfBlocks(blockno+1);
    }

  vtkUnstructuredGrid *grid
    = vtkUnstructuredGrid::SafeDownCast(blocks->GetBlock(blockno));
  if (!grid)
    {
    grid = vtkUnstructuredGrid::New();
    blocks->SetBlock(blockno, grid);
    blocks->GetMetaData(blockno)->Set(typeKey, 1);
    grid->Delete();     // Not really deleted.
    }

  return grid;
}

//=============================================================================
vtkCxxRevisionMacro(vtkSLACReader, "1.3");
vtkStandardNewMacro(vtkSLACReader);

vtkInformationKeyMacro(vtkSLACReader, IS_INTERNAL_VOLUME, Integer);
vtkInformationKeyMacro(vtkSLACReader, IS_EXTERNAL_SURFACE, Integer);
vtkInformationKeyMacro(vtkSLACReader, POINTS, ObjectBase);
vtkInformationKeyMacro(vtkSLACReader, POINT_DATA, ObjectBase);

//-----------------------------------------------------------------------------
vtkSLACReader::vtkSLACReader()
{
  this->SetNumberOfInputPorts(0);

  this->MeshFileName = NULL;

  this->ReadInternalVolume = 0;
  this->ReadExternalSurface = 1;
  this->ReadMidpoints = 1;

  this->VariableArraySelection = vtkSmartPointer<vtkDataArraySelection>::New();
  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&vtkSLACReader::SelectionModifiedCallback);
  cbc->SetClientData(this);
  this->VariableArraySelection->AddObserver(vtkCommand::ModifiedEvent, cbc);

  this->ReadModeData = false;
  this->TimeStepModes = false;
  this->FrequencyModes = false;
}

vtkSLACReader::~vtkSLACReader()
{
  this->SetMeshFileName(NULL);
}

void vtkSLACReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "MeshFileName: " << this->MeshFileName << endl;
  for (unsigned int i = 0; i < this->ModeFileNames.size(); i++)
    {
    os << indent << "ModeFileName[" << i << "]: "
       << this->ModeFileNames[i] << endl;
    }

  os << indent << "ReadInternalVolume: " << this->ReadInternalVolume << endl;
  os << indent << "ReadExternalSurface: " << this->ReadExternalSurface << endl;
  os << indent << "ReadMidpoints: " << this->ReadMidpoints << endl;

  os << indent << "VariableArraySelection:" << endl;
  this->VariableArraySelection->PrintSelf(os, indent.GetNextIndent());
}

//-----------------------------------------------------------------------------
int vtkSLACReader::CanReadFile(const char *filename)
{
  vtkSLACReaderAutoCloseNetCDF ncFD(filename, NC_NOWRITE, true);
  if (!ncFD.Valid()) return 0;

  // Check for the existence of several arrays we know should be in the file.
  int dummy;
  if (nc_inq_varid(ncFD(), "coords", &dummy) != NC_NOERR) return 0;
  if (nc_inq_varid(ncFD(), "tetrahedron_interior",&dummy) != NC_NOERR) return 0;
  if (nc_inq_varid(ncFD(), "tetrahedron_exterior",&dummy) != NC_NOERR) return 0;
  if (nc_inq_varid(ncFD(), "surface_midpoint", &dummy) != NC_NOERR) return 0;

  return 1;
}

//-----------------------------------------------------------------------------
void vtkSLACReader::AddModeFileName(const char *fname)
{
  this->ModeFileNames.push_back(fname);
  this->Modified();
}

void vtkSLACReader::RemoveAllModeFileNames()
{
  this->ModeFileNames.clear();
  this->Modified();
}

unsigned int vtkSLACReader::GetNumberOfModeFileNames()
{
  return this->ModeFileNames.size();
}

const char *vtkSLACReader::GetModeFileName(unsigned int idx)
{
  return this->ModeFileNames[idx].c_str();
}

//-----------------------------------------------------------------------------
vtkIdType vtkSLACReader::GetNumTuplesInVariable(int ncFD, int varId,
                                                int expectedNumComponents)
{
  int numDims;
  CALL_NETCDF(nc_inq_varndims(ncFD, varId, &numDims));
  if (numDims != 2)
    {
    char name[NC_MAX_NAME+1];
    CALL_NETCDF(nc_inq_varname(ncFD, varId, name));
    vtkErrorMacro(<< "Wrong dimensions on " << name);
    return 0;
    }

  int dimIds[2];
  CALL_NETCDF(nc_inq_vardimid(ncFD, varId, dimIds));

  size_t dimLength;
  CALL_NETCDF(nc_inq_dimlen(ncFD, dimIds[1], &dimLength));
  if (static_cast<int>(dimLength) != expectedNumComponents)
    {
    char name[NC_MAX_NAME+1];
    CALL_NETCDF(nc_inq_varname(ncFD, varId, name));
    vtkErrorMacro(<< "Unexpected tuple size on " << name);
    return 0;
    }

  CALL_NETCDF(nc_inq_dimlen(ncFD, dimIds[0], &dimLength));
  return static_cast<vtkIdType>(dimLength);
}

//-----------------------------------------------------------------------------
int vtkSLACReader::RequestInformation(
                                 vtkInformation *vtkNotUsed(request),
                                 vtkInformationVector **vtkNotUsed(inputVector),
                                 vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  outInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_RANGE());

  if (!this->MeshFileName)
    {
    vtkErrorMacro("No filename specified.");
    return 0;
    }

  this->VariableArraySelection->RemoveAllArrays();

  vtkSLACReaderAutoCloseNetCDF meshFD(this->MeshFileName, NC_NOWRITE);
  if (!meshFD.Valid()) return 0;

  this->ReadModeData = false;   // Assume false until everything checks out.
  this->TimeStepModes = false;
  this->TimeStepToFile.clear();
  this->FrequencyModes = false;
  this->Frequency = 0.0;
  if (!this->ModeFileNames.empty())
    {
    // Check the first mode file, assume that the rest follow.
    vtkSLACReaderAutoCloseNetCDF modeFD(this->ModeFileNames[0], NC_NOWRITE);
    if (!modeFD.Valid()) return 0;

    int meshCoordsVarId, modeCoordsVarId;
    CALL_NETCDF(nc_inq_varid(meshFD(), "coords", &meshCoordsVarId));
    CALL_NETCDF(nc_inq_varid(modeFD(), "coords", &modeCoordsVarId));

    if (   this->GetNumTuplesInVariable(meshFD(), meshCoordsVarId, 3)
        != this->GetNumTuplesInVariable(modeFD(), modeCoordsVarId, 3) )
      {
      vtkWarningMacro(<< "Mode file " << this->ModeFileNames[0].c_str()
                      << " invalid for mesh file " << this->MeshFileName
                      << "; the number of coordinates do not match.");
      }
    else
      {
      this->ReadModeData = true;

      // Read the "frequency".  When a time series is written, the frequency
      // variable is overloaded to mean time.  There is no direct way to tell
      // the difference, but things happen very quickly (less than nanoseconds)
      // in simulations that write out this data.  Thus, we expect large numbers
      // to be frequency (in Hz) and small numbers to be time (in seconds).
      if (   (nc_get_scalar_double(modeFD(), "frequency", &this->Frequency) != NC_NOERR)
          && (nc_get_scalar_double(modeFD(), "frequencyreal", &this->Frequency) != NC_NOERR) )
        {
        vtkWarningMacro(<< "Could not find frequency in mode data.");
        return 0;
        }
      if (this->Frequency < 100)
        {
        this->TimeStepModes = true;
        this->TimeStepToFile[this->Frequency] = this->ModeFileNames[0];
        }
      else
        {
        this->FrequencyModes = true;
        }

      vtksys::RegularExpression imaginaryVar("_imag$");

      int ncoordDim;
      CALL_NETCDF(nc_inq_dimid(modeFD(), "ncoord", &ncoordDim));

      int numVariables;
      CALL_NETCDF(nc_inq_nvars(modeFD(), &numVariables));

      for (int i = 0; i < numVariables; i++)
        {
        int numDims;
        CALL_NETCDF(nc_inq_varndims(modeFD(), i, &numDims));
        if ((numDims < 1) || (numDims > 2)) continue;

        int dimIds[2];
        CALL_NETCDF(nc_inq_vardimid(modeFD(), i, dimIds));
        if (dimIds[0] != ncoordDim) continue;

        char name[NC_MAX_NAME+1];
        CALL_NETCDF(nc_inq_varname(modeFD(), i, name));
        if (strcmp(name, "coords") == 0) continue;
        if (this->FrequencyModes && imaginaryVar.find(name)) continue;

        this->VariableArraySelection->AddArray(name);
        }
      }
    }

  if (this->TimeStepModes)
    {
    // If we are in time steps modes, we need to read in the time values from
    // all the files (and we have already read the first one).  We then report
    // the time steps we have.
    vtkstd::vector<vtkStdString>::iterator fileitr =this->ModeFileNames.begin();
    fileitr++;
    for ( ; fileitr != this->ModeFileNames.end(); fileitr++)
      {
      vtkSLACReaderAutoCloseNetCDF modeFD(*fileitr, NC_NOWRITE);
      if (!modeFD.Valid()) return 0;

      if (   (nc_get_scalar_double(modeFD(), "frequency", &this->Frequency) != NC_NOERR)
          && (nc_get_scalar_double(modeFD(), "frequencyreal", &this->Frequency) != NC_NOERR) )
        {
        vtkWarningMacro(<< "Could not find frequency in mode data.");
        return 0;
        }
      this->TimeStepToFile[this->Frequency] = *fileitr;
      }

    double range[2];
    outInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    vtkstd::map<double, vtkStdString>::iterator timeitr
      = this->TimeStepToFile.begin();
    range[0] = timeitr->first;
    for ( ; timeitr != this->TimeStepToFile.end(); timeitr++)
      {
      range[1] = timeitr->first;        // Eventually set to last value.
      outInfo->Append(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
                      timeitr->first);
      }
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), range, 2);
    }
  else if (this->FrequencyModes)
    {
    double range[2];
    range[0] = 0;
    range[1] = 1.0/this->Frequency;
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), range, 2);
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::RequestData(vtkInformation *vtkNotUsed(request),
                               vtkInformationVector **vtkNotUsed(inputVector),
                               vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::GetData(outInfo);

  if (!this->MeshFileName)
    {
    vtkErrorMacro("No filename specified.");
    return 0;
    }

  double time =  0.0;
  bool timeValid = false;
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
    {
    time =outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS(),0);
    timeValid = true;
    }

  if (this->FrequencyModes)
    {
    this->Phase = vtkMath::DoubleTwoPi()*(time*this->Frequency);
    }

  int readMesh = !this->MeshUpToDate();

  if (readMesh)
    {
    this->MidpointIdCache.clear();
    this->MeshCache = vtkSmartPointer<vtkMultiBlockDataSet>::New();

    vtkSLACReaderAutoCloseNetCDF meshFD(this->MeshFileName, NC_NOWRITE);
    if (!meshFD.Valid()) return 0;

    // Set up point data.
    VTK_CREATE(vtkPoints, points);
    VTK_CREATE(vtkPointData, pd);
    output->GetInformation()->Set(vtkSLACReader::POINTS(), points);
    output->GetInformation()->Set(vtkSLACReader::POINT_DATA(), pd);

    if (!this->ReadInternalVolume && !this->ReadExternalSurface) return 1;

    if (!this->ReadConnectivity(meshFD(), output)) return 0;

    this->UpdateProgress(0.25);

    if (!this->ReadCoordinates(meshFD(), output)) return 0;

    this->UpdateProgress(0.5);

    if (this->ReadMidpoints)
      {
      if (!this->ReadMidpointData(meshFD(), output, this->MidpointIdCache))
        {
        return 0;
        }
      }

    this->MeshCache->ShallowCopy(output);
    this->PointCache = points;
    this->MeshReadTime.Modified();
    }
  else
    {
    if (!this->RestoreMeshCache(output)) return 0;
    }

  this->UpdateProgress(0.75);

  if (this->ReadModeData)
    {
    vtkStdString modeFileName;
    if (this->TimeStepModes && timeValid)
      {
      modeFileName = this->TimeStepToFile.lower_bound(time)->second;
      }
    else
      {
      modeFileName = this->ModeFileNames[0];
      }
    vtkSLACReaderAutoCloseNetCDF modeFD(modeFileName, NC_NOWRITE);
    if (!modeFD.Valid()) return 0;

    if (!this->ReadFieldData(modeFD(), output)) return 0;

    this->UpdateProgress(0.875);

    if (!this->InterpolateMidpointData(output, this->MidpointIdCache)) return 0;
    }

  // Push points to output.
  vtkPoints *points = vtkPoints::SafeDownCast(
                        output->GetInformation()->Get(vtkSLACReader::POINTS()));
  VTK_CREATE(vtkCompositeDataIterator, outputIter);
  for (outputIter.TakeReference(output->NewIterator());
       !outputIter->IsDoneWithTraversal(); outputIter->GoToNextItem())
    {
    vtkUnstructuredGrid *ugrid
      = vtkUnstructuredGrid::SafeDownCast(output->GetDataSet(outputIter));
    ugrid->SetPoints(points);
    }

  // Push point field data to output.
  vtkPointData *pd = vtkPointData::SafeDownCast(
                    output->GetInformation()->Get(vtkSLACReader::POINT_DATA()));
  for (outputIter.TakeReference(output->NewIterator());
       !outputIter->IsDoneWithTraversal(); outputIter->GoToNextItem())
    {
    vtkUnstructuredGrid *ugrid
      = vtkUnstructuredGrid::SafeDownCast(output->GetDataSet(outputIter));
    ugrid->GetPointData()->ShallowCopy(pd);
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkSLACReader::SelectionModifiedCallback(vtkObject*, unsigned long,
                                              void* clientdata, void*)
{
  static_cast<vtkSLACReader*>(clientdata)->Modified();
}

//-----------------------------------------------------------------------------
int vtkSLACReader::GetNumberOfVariableArrays()
{
  return this->VariableArraySelection->GetNumberOfArrays();
}

//-----------------------------------------------------------------------------
const char* vtkSLACReader::GetVariableArrayName(int index)
{
  return this->VariableArraySelection->GetArrayName(index);
}

//-----------------------------------------------------------------------------
int vtkSLACReader::GetVariableArrayStatus(const char* name)
{
  return this->VariableArraySelection->ArrayIsEnabled(name);
}

//-----------------------------------------------------------------------------
void vtkSLACReader::SetVariableArrayStatus(const char* name, int status)
{
  vtkDebugMacro("Set cell array \"" << name << "\" status to: " << status);
  if(status)
    {
    this->VariableArraySelection->EnableArray(name);
    }
  else
    {
    this->VariableArraySelection->DisableArray(name);
    }
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadTetrahedronInteriorArray(int meshFD,
                                                vtkIdTypeArray *connectivity)
{
  int tetInteriorVarId;
  CALL_NETCDF(nc_inq_varid(meshFD, "tetrahedron_interior", &tetInteriorVarId));
  vtkIdType numTetsInterior
    = this->GetNumTuplesInVariable(meshFD, tetInteriorVarId, NumPerTetInt);

  connectivity->Initialize();
  connectivity->SetNumberOfComponents(NumPerTetInt);
  connectivity->SetNumberOfTuples(numTetsInterior);
  CALL_NETCDF(nc_get_var_vtkIdType(meshFD, tetInteriorVarId,
                                   connectivity->GetPointer(0)));

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadTetrahedronExteriorArray(int meshFD,
                                                vtkIdTypeArray *connectivity)
{
  int tetExteriorVarId;
  CALL_NETCDF(nc_inq_varid(meshFD, "tetrahedron_exterior", &tetExteriorVarId));
  vtkIdType numTetsExterior
    = this->GetNumTuplesInVariable(meshFD, tetExteriorVarId, NumPerTetExt);

  connectivity->Initialize();
  connectivity->SetNumberOfComponents(NumPerTetExt);
  connectivity->SetNumberOfTuples(numTetsExterior);
  CALL_NETCDF(nc_get_var_vtkIdType(meshFD, tetExteriorVarId,
                                   connectivity->GetPointer(0)));

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadConnectivity(int meshFD, vtkMultiBlockDataSet *output)
{
  // Get ready to read in cells and separate into assembly based on element
  // attributes and boundary conditions.
  VTK_CREATE(vtkMultiBlockDataSet, solidMeshes);
  VTK_CREATE(vtkMultiBlockDataSet, externalFaces);

  // Read in interior tetrahedra.
  VTK_CREATE(vtkIdTypeArray, connectivity);
  if (this->ReadInternalVolume)
    {
    if (!this->ReadTetrahedronInteriorArray(meshFD, connectivity)) return 0;
    vtkIdType numTetsInterior = connectivity->GetNumberOfTuples();
    for (vtkIdType i = 0; i < numTetsInterior; i++)
      {
      // Interior tetrahedra are defined with 5 integers.  The first is an
      // element attribute (which we will use to separate into multiple blocks)
      // and the other four are ids for the 4 points of the tetrahedra.  The
      // faces of the tetrahedra are the following:
      // Face 0:  0,  2,  1
      // Face 1:  0,  3,  2
      // Face 2:  0,  1,  3
      // Face 3:  1,  2,  3
      // We are fortunate in that the winding of the tetrahedron agrees with the
      // VTK winding, so we can just copy indices.
      vtkIdType tetInfo[NumPerTetInt];
      connectivity->GetTupleValue(i, tetInfo);
      vtkUnstructuredGrid *ugrid = AllocateGetBlock(solidMeshes, tetInfo[0],
                                                    IS_INTERNAL_VOLUME());
      ugrid->InsertNextCell(VTK_TETRA, 4, tetInfo+1);
      }
    }

  // Read in exterior tetrahedra.
  if (!this->ReadTetrahedronExteriorArray(meshFD, connectivity)) return 0;
  vtkIdType numTetsExterior = connectivity->GetNumberOfTuples();
  for (vtkIdType i = 0; i < numTetsExterior; i++)
    {
    // Exterior tetrahedra are defined with 9 integers.  The first is an element
    // attribute and the next 4 are point ids, which is the same as interior
    // tetrahedra (see above).  The last 4 define the boundary condition of
    // each face (see above for the order of faces).  A flag of -1 is used
    // when the face is internal.  Other flags separate faces in a multiblock
    // data set.
    vtkIdType tetInfo[NumPerTetExt];
    connectivity->GetTupleValue(i, tetInfo);
    if (this->ReadInternalVolume)
      {
      vtkUnstructuredGrid *ugrid = AllocateGetBlock(solidMeshes, tetInfo[0],
                                                    IS_INTERNAL_VOLUME());
      ugrid->InsertNextCell(VTK_TETRA, 4, tetInfo+1);
      }

    if (this->ReadExternalSurface)
      {
      for (int face = 0; face < 4; face++)
        {
        int boundaryCondition = tetInfo[5+face];
        if (boundaryCondition >= 0)
          {
          vtkUnstructuredGrid *ugrid = AllocateGetBlock(externalFaces,
                                                        boundaryCondition,
                                                        IS_EXTERNAL_SURFACE());
          vtkIdType ptids[3];
          ptids[0] = tetInfo[1+tetFaces[face][0]];
          ptids[1] = tetInfo[1+tetFaces[face][1]];
          ptids[2] = tetInfo[1+tetFaces[face][2]];
          ugrid->InsertNextCell(VTK_TRIANGLE, 3, ptids);
          }
        }
      }
    }

  // Push connectivity to output.
  if (this->ReadInternalVolume && this->ReadExternalSurface)
    {
    output->SetNumberOfBlocks(2);
    output->SetBlock(0, solidMeshes);
    output->SetBlock(1, externalFaces);

    output->GetMetaData(0u)
      ->Set(vtkCompositeDataSet::NAME(), "Internal Volume");
    output->GetMetaData(1u)
      ->Set(vtkCompositeDataSet::NAME(), "External Surface");
    }
  else if (this->ReadInternalVolume) // && !this->ReadExternalSurface
    {
    output->ShallowCopy(solidMeshes);
    }
  else // this->ReadExternalSurface && !this->ReadInternalVolume
    {
    output->ShallowCopy(externalFaces);
    }

  return 1;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkDataArray> vtkSLACReader::ReadPointDataArray(int ncFD,
                                                                int varId)
{
  // Get the dimension info.  We should only need to worry about 1 or 2D arrays.
  int numDims;
  CALL_NETCDF(nc_inq_varndims(ncFD, varId, &numDims));
  if (numDims > 2) // don't support 3d or higher arrays
    {
    vtkErrorMacro(<< "Sanity check failed.  "
                  << "Encountered array with too many dimensions.");
    return 0;
    }
  if (numDims < 1) // don't support 0d arrays
    {
    vtkErrorMacro(<< "Sanity check failed.  "
                  << "Encountered array no dimensions.");
    return 0;
    }
  int dimIds[2];
  CALL_NETCDF(nc_inq_vardimid(ncFD, varId, dimIds));
  size_t numCoords;
  CALL_NETCDF(nc_inq_dimlen(ncFD, dimIds[0], &numCoords));
  size_t numComponents = 1;
  if (numDims > 1)
    {
    CALL_NETCDF(nc_inq_dimlen(ncFD, dimIds[1], &numComponents));
    }

  // Allocate an array of the right type.
  nc_type ncType;
  CALL_NETCDF(nc_inq_vartype(ncFD, varId, &ncType));
  int vtkType = NetCDFTypeToVTKType(ncType);
  if (vtkType < 1) return 0;
  vtkSmartPointer<vtkDataArray> dataArray;
  dataArray.TakeReference(vtkDataArray::CreateDataArray(vtkType));
  dataArray->SetNumberOfComponents(numComponents);
  dataArray->SetNumberOfTuples(numCoords);

  // Read the data from the file.
  size_t start[2], count[2];
  start[0] = start[1] = 0;
  count[0] = numCoords;  count[1] = numComponents;
  CALL_NETCDF(nc_get_vars(ncFD, varId, start, count, NULL,
                          dataArray->GetVoidPointer(0)));

  return dataArray;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadCoordinates(int meshFD, vtkMultiBlockDataSet *output)
{
  // Read in the point coordinates.  The coordinates are 3-tuples in an array
  // named "coords".
  int coordsVarId;
  CALL_NETCDF(nc_inq_varid(meshFD, "coords", &coordsVarId));

  vtkSmartPointer<vtkDataArray> coordData
    = this->ReadPointDataArray(meshFD, coordsVarId);
  if (!coordData) return 0;
  if (coordData->GetNumberOfComponents() != 3)
    {
    vtkErrorMacro(<< "Failed sanity check!  Coords have wrong dimensions.");
    return 0;
    }
  coordData->SetName("coords");

  vtkPoints *points = vtkPoints::SafeDownCast(
                        output->GetInformation()->Get(vtkSLACReader::POINTS()));
  points->SetData(coordData);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadFieldData(int modeFD, vtkMultiBlockDataSet *output)
{
  vtkPointData *pd = vtkPointData::SafeDownCast(
                    output->GetInformation()->Get(vtkSLACReader::POINT_DATA()));

  // Get the number of coordinates (which determines how many items are read
  // per variable).
  int ncoordDim;
  CALL_NETCDF(nc_inq_dimid(modeFD, "ncoord", &ncoordDim));
  size_t numCoords;
  CALL_NETCDF(nc_inq_dimlen(modeFD, ncoordDim, &numCoords));

  int numArrays = this->VariableArraySelection->GetNumberOfArrays();
  for (int arrayIndex = 0; arrayIndex < numArrays; arrayIndex++)
    {
    // skip array if not enabled
    if (!this->VariableArraySelection->GetArraySetting(arrayIndex)) continue;

    // from the variable name, get the variable id
    const char *name = this->VariableArraySelection->GetArrayName(arrayIndex);
    int varId;
    CALL_NETCDF(nc_inq_varid(modeFD, name, &varId));

    // if this variable isn't 1d or 2d array, skip it.
    int numDims;
    CALL_NETCDF(nc_inq_varndims(modeFD, varId, &numDims));
    if (numDims < 1 || numDims > 2)
      {
      vtkWarningMacro(<< "Encountered invalid variable dimensions.")
      continue;
      }

    // Read in the array data.
    vtkSmartPointer<vtkDataArray> dataArray
      = this->ReadPointDataArray(modeFD, varId);
    if (!dataArray) continue;

    // Check for imaginary component of mode data.
    if (this->FrequencyModes)
      {
      vtkStdString imagName = name;
      imagName += "_imag";
      if (nc_inq_varid(modeFD, imagName.c_str(), &varId) == NC_NOERR)
        {
        // I am assuming here that the imaginary data (if it exists) has the
        // same dimensions as the real data.
        vtkSmartPointer<vtkDataArray> imagDataArray
          = this->ReadPointDataArray(modeFD, varId);
        if (imagDataArray)
          {
          int numComponents = dataArray->GetNumberOfComponents();
          vtkIdType numTuples = dataArray->GetNumberOfTuples();
          for (vtkIdType i = 0; i < numTuples; i++)
            {
            for (int j = 0; j < numComponents; j++)
              {
              double real = dataArray->GetComponent(i, j);
              double imag = imagDataArray->GetComponent(i, j);
              double mag = sqrt(real*real + imag*imag);
              double startphase = atan2(imag, real);
              dataArray->SetComponent(i, j, mag*cos(startphase*this->Phase));
              }
            }
          }
        }
      }

    // Add the data to the point data.
    dataArray->SetName(name);
    pd->AddArray(dataArray);
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadMidpointCoordinates(
                                       int meshFD,
                                       vtkMultiBlockDataSet *output,
                                       vtkMidpointCoordinateMap &map)
{
  // Get the number of midpoints.
  int midpointsVar;
  CALL_NETCDF(nc_inq_varid(meshFD, "surface_midpoint", &midpointsVar));
  vtkIdType numMidpoints = this->GetNumTuplesInVariable(meshFD,midpointsVar,5);
  if (numMidpoints < 1) return 0;

  // Read in the raw data.
  VTK_CREATE(vtkDoubleArray, midpointData);
  midpointData->SetNumberOfComponents(5);
  midpointData->SetNumberOfTuples(numMidpoints);
  CALL_NETCDF(nc_get_var_double(meshFD, midpointsVar,
                                midpointData->GetPointer(0)));

  vtkPoints *points = vtkPoints::SafeDownCast(
                        output->GetInformation()->Get(vtkSLACReader::POINTS()));
  vtkIdType pointTotal = points->GetNumberOfPoints ();
  // Create a searchable structure.
  for (vtkIdType i = 0; i < numMidpoints; i++)
    {
    double *mp = midpointData->GetPointer(i*5);

    map.insert(make_pair(make_pair(static_cast<vtkIdType>(mp[0]),
                                   static_cast<vtkIdType>(mp[1])),
                         vtkSLACReader::vtkMidpoint(mp+2, i + pointTotal)));
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::ReadMidpointData(int meshFD, vtkMultiBlockDataSet *output,
                                    vtkMidpointIdMap &midpointIds)
{
  static bool GaveMidpointWarning = false;
  if (!GaveMidpointWarning)
    {
    vtkWarningMacro(<< "Quadratic elements not displayed entirely correctly yet.  Quadratic triangles are drawn as 4 linear triangles.");
    GaveMidpointWarning = true;
    }

  // Get the point information from the data.
  vtkPoints *points = vtkPoints::SafeDownCast(
                        output->GetInformation()->Get(vtkSLACReader::POINTS()));

  // Read in the midpoint coordinates.
  vtkMidpointCoordinateMap midpointCoords;
  if (!this->ReadMidpointCoordinates(meshFD, output, midpointCoords)) return 0;

  vtkIdType newPointTotal = points->GetNumberOfPoints() + midpointCoords.size();

  // Iterate over all of the parts in the output and visit the ones for the
  // external surface.
  VTK_CREATE(vtkCompositeDataIterator, outputIter);
  for (outputIter.TakeReference(output->NewIterator());
       !outputIter->IsDoneWithTraversal(); outputIter->GoToNextItem())
    {
    if (!output->GetMetaData(outputIter)->Get(IS_EXTERNAL_SURFACE())) continue;

    // Create a new cell array so that we can convert all the cells from
    // triangles to quadratic triangles.
    vtkUnstructuredGrid *ugrid
      = vtkUnstructuredGrid::SafeDownCast(output->GetDataSet(outputIter));
    vtkCellArray *oldCells = ugrid->GetCells();
    VTK_CREATE(vtkCellArray, newCells);
    newCells->Allocate(newCells->EstimateSize(oldCells->GetNumberOfCells(), 6));

    // Iterate over all of the cells.
    vtkIdType npts, *pts;
    for (oldCells->InitTraversal(); oldCells->GetNextCell(npts, pts); )
      {
      newCells->InsertNextCell(6);

      // Copy corner points.
      newCells->InsertCellPoint(pts[0]);
      newCells->InsertCellPoint(pts[1]);
      newCells->InsertCellPoint(pts[2]);

      // Add edge midpoints.
      for (int edgeInc = 0; edgeInc < 3; edgeInc++)
        {
        // Get the points defining the edge.
        vtkIdType p0 = pts[triEdges[edgeInc][0]];
        vtkIdType p1 = pts[triEdges[edgeInc][1]];
        pair<vtkIdType,vtkIdType> edge = make_pair(MY_MIN(p0,p1),MY_MAX(p0,p1));

        // See if we have already copied this midpoint.
        vtkMidpointIdMap::iterator idLoc = midpointIds.find(edge);
        vtkIdType midId;
        if (idLoc != midpointIds.end())
          {
          midId = idLoc->second;
          }
        else
          {
          // Check to see if the midpoint was read from the file.  If not,
          // then interpolate linearly between the two edge points.
          vtkMidpointCoordinateMap::iterator coordLoc
            = midpointCoords.find(edge);
          vtkMidpoint midpoint;
          if (coordLoc == midpointCoords.end())
            {
            double coord0[3], coord1[3], coordMid[3];
            points->GetPoint(p0, coord0);
            points->GetPoint(p1, coord1);
            coordMid[0] = 0.5*(coord0[0] + coord1[0]);
            coordMid[1] = 0.5*(coord0[1] + coord1[1]);
            coordMid[2] = 0.5*(coord0[2] + coord1[2]);
            midpoint = vtkMidpoint(coordMid, newPointTotal);
            newPointTotal ++;
            }
          else
            {
            midpoint = coordLoc->second;
            // Erase the midpoint from the map.  We don't need it anymore since
            // we will insert a point id in the midpointIds map (see below).
            midpointCoords.erase(coordLoc);
            }

          // Add the new point to the point data.
          points->InsertPoint(midpoint.ID, midpoint.Coordinate);

          // Add the new point to the id map.
          midpointIds.insert(make_pair(edge, midpoint.ID));
          midId = midpoint.ID;
          }

        // Record the midpoint in the quadratic cell.
        newCells->InsertCellPoint(midId);
        }
      }

    // Save the new cells in the data.
    ugrid->SetCells(VTK_QUADRATIC_TRIANGLE, newCells);
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::InterpolateMidpointData(vtkMultiBlockDataSet *output,
                                           vtkMidpointIdMap &map)
{
  // Get the point information from the output data (where it was placed
  // earlier).
  vtkPoints *points = vtkPoints::SafeDownCast(
                        output->GetInformation()->Get(vtkSLACReader::POINTS()));
  vtkPointData *pd = vtkPointData::SafeDownCast(
                    output->GetInformation()->Get(vtkSLACReader::POINT_DATA()));
  if (!pd)
    {
    vtkWarningMacro(<< "Missing point data.");
    return 0;
    }

  // Set up the point data for adding new points and interpolating their values.
  pd->InterpolateAllocate(pd, points->GetNumberOfPoints());

  for (vtkMidpointIdMap::iterator i = map.begin(); i != map.end(); i++)
    {
    vtkIdType edgePoint0 = i->first.first;
    vtkIdType edgePoint1 = i->first.second;
    vtkIdType midpoint = i->second;
    pd->InterpolateEdge(pd, midpoint, edgePoint0, edgePoint1, 0.5);
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::MeshUpToDate()
{
  if (this->MeshReadTime < this->GetMTime())
    {
    return 0;
    }
  if (this->MeshReadTime < this->VariableArraySelection->GetMTime())
    {
    return 0;
    }
  return 1;
}

//-----------------------------------------------------------------------------
int vtkSLACReader::RestoreMeshCache(vtkMultiBlockDataSet *output)
{
  output->ShallowCopy(this->MeshCache);
  output->GetInformation()->Set(vtkSLACReader::POINTS(), this->PointCache);

  VTK_CREATE(vtkPointData, pd);
  output->GetInformation()->Set(vtkSLACReader::POINT_DATA(), pd);

  return 1;
}
