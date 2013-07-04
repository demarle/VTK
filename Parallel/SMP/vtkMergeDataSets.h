/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMergeDataSets.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMergeDataSets - !!!!
// .SECTION Description
// !!!!

#ifndef __vtkMergeDataSets_h__
#define __vtkMergeDataSets_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"
#include "vtkThreadLocal.h" // makes things easy

class vtkSMPMergePoints;
class vtkPointData;
class vtkPoints;
class vtkCellArray;
class vtkCellData;
class vtkTask;
class vtkIdList;

class VTKPARALLELSMP_EXPORT vtkMergeDataSets : public vtkObject
{
    void Parallel(
        const vtkTask* function,
        vtkThreadLocal<vtkSMPMergePoints>::iterator data);
    void Parallel(
        const vtkTask* function,
        vtkThreadLocal<vtkIdList>::iterator data1,
        vtkThreadLocal<vtkCellData>::iterator data2,
        vtkThreadLocal<vtkCellArray>::iterator data3,
        vtkThreadLocal<vtkCellArray>::iterator data4,
        vtkThreadLocal<vtkCellArray>::iterator data5,
        vtkThreadLocal<vtkCellArray>::iterator data6,
        vtkstd::vector<vtkIdType>::iterator offset1,
        vtkstd::vector<vtkIdType>::iterator offset2,
        vtkstd::vector<vtkIdType>::iterator offset3,
        vtkstd::vector<vtkIdType>::iterator offset4,
        vtkstd::vector<vtkIdType>::iterator offset5,
        vtkstd::vector<vtkIdType>::iterator offset6,
        vtkstd::vector<vtkIdType>::iterator offset7,
        vtkstd::vector<vtkIdType>::iterator offset8);

  public:
    vtkTypeMacro(vtkMergeDataSets,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);
    static vtkMergeDataSets* New();

    vtkSetMacro(MasterThreadPopulatedOutput,int);
    vtkBooleanMacro(MasterThreadPopulatedOutput,int);

    void MergePolyData(
        vtkPoints* outPoints,
        vtkThreadLocal<vtkPoints>* inPoints,
        const double bounds[6],
        vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
        vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
        vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
        vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
        vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
        vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData);
    void MergePolyData(
        vtkSMPMergePoints* outPoints,
        vtkThreadLocal<vtkSMPMergePoints>* inPoints,
        vtkPointData* outPtsData, vtkThreadLocal<vtkPointData>* inPtsData,
        vtkCellArray* outVerts, vtkThreadLocal<vtkCellArray>* inVerts,
        vtkCellArray* outLines, vtkThreadLocal<vtkCellArray>* inLines,
        vtkCellArray* outPolys, vtkThreadLocal<vtkCellArray>* inPolys,
        vtkCellArray* outStrips, vtkThreadLocal<vtkCellArray>* inStrips,
        vtkCellData* outCellsData, vtkThreadLocal<vtkCellData>* inCellsData);

  protected:
    vtkMergeDataSets();
    ~vtkMergeDataSets();

    int MasterThreadPopulatedOutput;
    vtkIdType** TreatedTable;

  private:
    vtkMergeDataSets(const vtkMergeDataSets&); // Not implemented
    void operator=(const vtkMergeDataSets&); // Not implemented
};

#endif
