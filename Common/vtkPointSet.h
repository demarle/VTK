/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPointSet.h
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
// .NAME vtkPointSet - abstract class for specifying dataset behavior
// .SECTION Description
// vtkPointSet is an abstract class that specifies the interface for 
// datasets that explicitly use "point" arrays to represent geometry.
// For example, vtkPolyData and vtkUnstructuredGrid require point arrays
// to specify point position, while vtkStructuredPoints generates point
// positions implicitly.

// .SECTION See Also
// vtkPolyData vtkStructuredGrid vtkUnstructuredGrid

#ifndef __vtkPointSet_h
#define __vtkPointSet_h

#include "vtkDataSet.h"

class vtkPointLocator;

class VTK_COMMON_EXPORT vtkPointSet : public vtkDataSet
{
public:
  vtkTypeRevisionMacro(vtkPointSet,vtkDataSet);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reset to an empty state and free any memory.
  void Initialize();

  // Description:
  // Copy the geometric structure of an input point set object.
  void CopyStructure(vtkDataSet *pd);

  // Description:
  // See vtkDataSet for additional information.
  vtkIdType GetNumberOfPoints();
  float *GetPoint(vtkIdType ptId) {return this->Points->GetPoint(ptId);};
  void GetPoint(vtkIdType ptId, float x[3]) {this->Points->GetPoint(ptId,x);};
  vtkIdType FindPoint(float x[3]);
  vtkIdType FindPoint(float x, float y, float z) { return this->vtkDataSet::FindPoint(x, y, z);};
  vtkIdType FindCell(float x[3], vtkCell *cell, vtkIdType cellId, float tol2,
               int& subId, float pcoords[3], float *weights);
  vtkIdType FindCell(float x[3], vtkCell *cell, vtkGenericCell *gencell,
               vtkIdType cellId, float tol2, int& subId, 
               float pcoords[3], float *weights);

  // Description:
  // Get MTime which also considers its vtkPoints MTime.
  unsigned long GetMTime();

  // Description:
  // Compute the (X, Y, Z)  bounds of the data.
  void ComputeBounds();
  
  // Description:
  // Reclaim any unused memory.
  void Squeeze();

  // Description:
  // Specify point array to define point coordinates.
  virtual void SetPoints(vtkPoints*);
  vtkGetObjectMacro(Points,vtkPoints);

  // Description:
  // Detect reference loop PointSet <-> locator.
  virtual void UnRegister(vtkObjectBase *o);
  
  // Description:
  // Get the net reference count. That is the count minus
  // any self created loops. This is used in the Source/Data
  // registration to properly free the objects.
  virtual int GetNetReferenceCount();

  // Description:
  // Return the actual size of the data in kilobytes. This number
  // is valid only after the pipeline has updated. The memory size
  // returned is guaranteed to be greater than or equal to the
  // memory required to represent the data (e.g., extra space in
  // arrays, etc. are not included in the return value). THIS METHOD
  // IS THREAD SAFE.
  unsigned long GetActualMemorySize();

  // Description:
  // Shallow and Deep copy.
  void ShallowCopy(vtkDataObject *src);  
  void DeepCopy(vtkDataObject *src);

protected:
  vtkPointSet();
  ~vtkPointSet();

  vtkPoints *Points;
  vtkPointLocator *Locator;

private:
  vtkPointSet(const vtkPointSet&);  // Not implemented.
  void operator=(const vtkPointSet&);  // Not implemented.
};

inline vtkIdType vtkPointSet::GetNumberOfPoints()
{
  if (this->Points)
    {
    return this->Points->GetNumberOfPoints();
    }
  else
    {
    return 0;
    }
}


#endif


