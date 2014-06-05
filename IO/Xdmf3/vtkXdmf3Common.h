/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXdmf3Common.h
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
// .NAME vtkXdmf3Common - dataset level translation between xdmf3 and vtk
// .SECTION Description
// The class vtkXdmf3Common has the lowest level, Xdmf2VTKarray and such.
// From that, we build classes with simple to and from methods that
// translate individual grids to and from vtkDataSet types.

//=============================================================================

#ifndef __vtkXdmf3Common_h
#define __vtkXdmf3Common_h

#include "vtkIOXdmf3Module.h" // For export macro

#include "XdmfSharedPtr.hpp"

#include <string>
#include <vector>
#include <set>
#include <map>

class XdmfArray;
class vtkDataArray;
class XdmfGrid;
class vtkDataObject;
class XdmfTopologyType;
class XdmfRegularGrid;
class vtkImageData;
class XdmfRectilinearGrid;
class vtkRectilinearGrid;
class XdmfCurvilinearGrid;
class vtkStructuredGrid;
class XdmfUnstructuredGrid;
class vtkUnstructuredGrid;
class XdmfGraph;
class vtkMutableDirectedGraph;
class vtkDirectedGraph;

class vtkDataSet;
class XdmfDomain;

//==============================================================================

class vtkXdmf3ArraySelection : public std::map<std::string, bool>
{
public:
  void Merge(const vtkXdmf3ArraySelection& other);
  void AddArray(const char* name, bool status=true);
  bool ArrayIsEnabled(const char* name);
  bool HasArray(const char* name);
  int GetArraySetting(const char* name);
  void SetArrayStatus(const char* name, bool status);
  const char* GetArrayName(int index);
  int GetNumberOfArrays();
  std::set<std::string> GetEnabledArrays();
};

//==============================================================================

class VTKIOXDMF3_EXPORT vtkXdmf3Common
{
public:

  static vtkDataArray *XdmfToVTKArray(
    XdmfArray* xArray,
    std::string attrName, //TODO: passing in attrName, because XdmfArray::getName() is oddly not virtual
    int preferredComponents = 0);

  static bool VTKToXdmfArray(
    vtkDataArray *vArray,
    XdmfArray* xArray,
    int rank = 0, int *dims = NULL);

  static void XdmfToVTKAttributes(
    vtkXdmf3ArraySelection *fselection,
    vtkXdmf3ArraySelection *cselection,
    vtkXdmf3ArraySelection *pselection,
    XdmfGrid *grid, vtkDataObject *dObject);

  static void VTKToXdmfAttributes(vtkDataObject *dObject, XdmfGrid *grid);

  static int GetNumberOfPointsPerCell(int vtk_cell_type);

  static int GetVTKCellType(shared_ptr<const XdmfTopologyType> topologyType);
  static int GetXdmfCellType(int vtkType);

  static void SetTime(XdmfGrid *grid, double hasTime, double time);
};

//==============================================================================

class VTKIOXDMF3_EXPORT vtkXdmf3RegularGrid
{
public:
  static void XdmfToVTK(
    vtkXdmf3ArraySelection *fselection,
    vtkXdmf3ArraySelection *cselection,
    vtkXdmf3ArraySelection *pselection,
    XdmfRegularGrid *grid,
    vtkImageData *dataSet);

  static void CopyShape(
    XdmfRegularGrid *grid,
    vtkImageData *dataSet);

  static void VTKToXdmf(
    vtkDataSet *dataSet,
    XdmfDomain *domain,
    bool hasTime, double time);
};

//==============================================================================

class VTKIOXDMF3_EXPORT vtkXdmf3RectilinearGrid
{
public:
  static void XdmfToVTK(
    vtkXdmf3ArraySelection *fselection,
    vtkXdmf3ArraySelection *cselection,
    vtkXdmf3ArraySelection *pselection,
    XdmfRectilinearGrid *grid,
    vtkRectilinearGrid *dataSet);

  static void CopyShape(
    XdmfRectilinearGrid *grid,
    vtkRectilinearGrid *dataSet);

  static void VTKToXdmf(
    vtkDataSet *dataSet,
    XdmfDomain *domain,
    bool hasTime, double time);
};

//==============================================================================

class VTKIOXDMF3_EXPORT vtkXdmf3CurvilinearGrid
{
public:
  static void XdmfToVTK(
    vtkXdmf3ArraySelection *fselection,
    vtkXdmf3ArraySelection *cselection,
    vtkXdmf3ArraySelection *pselection,
    XdmfCurvilinearGrid *grid,
    vtkStructuredGrid *dataSet);

  static void CopyShape(
    XdmfCurvilinearGrid *grid,
    vtkStructuredGrid *dataSet);

  static void VTKToXdmf(
    vtkDataSet *dataSet,
    XdmfDomain *domain,
    bool hasTime, double time);
};

//==============================================================================

class VTKIOXDMF3_EXPORT vtkXdmf3UnstructuredGrid
{
public:
  static void XdmfToVTK(
    vtkXdmf3ArraySelection *fselection,
    vtkXdmf3ArraySelection *cselection,
    vtkXdmf3ArraySelection *pselection,
    XdmfUnstructuredGrid *grid,
    vtkUnstructuredGrid *dataSet);

  static void CopyShape(
    XdmfUnstructuredGrid *grid,
    vtkUnstructuredGrid *dataSet);

  static void VTKToXdmf(
    vtkDataSet *dataSet,
    XdmfDomain *domain,
    bool hasTime, double time);
};

//==============================================================================

class VTKIOXDMF3_EXPORT vtkXdmf3Graph
{
public:
  static void XdmfToVTK(
    XdmfGraph *grid,
    vtkMutableDirectedGraph *dataSet);

  static void VTKToXdmf(
    vtkDirectedGraph *dataSet,
    XdmfDomain *domain,
    bool hasTime, double time);
};

#endif
