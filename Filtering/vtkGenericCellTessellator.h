/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGenericCellTessellator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGenericCellTessellator - helper class to perform cell tessellation
// .SECTION Description
// vtkGenericCellTessellator is a helper class to perform adaptive tessellation
// of particular cell topologies. The major purpose for this class is to
// transform higher-order cell types (e.g., higher-order finite elements)
// into linear cells that can then be easily visualized by VTK. This class
// works in conjunction with the vtkGenericDataSet and vtkGenericAdaptorCell
// classes.
//
// This algorithm is based on edge subdivision. An error metric along each
// edge is evaluated, and if the error is greater than some tolerance, the
// edge is subdivided (as well as all connected 2D and 3D cells). The process
// repeats until the error metric is satisfied. 
//
// A significant issue addressed by this algorithm is to insure face
// compatibility across neigboring cells. That is, diagaonals due to face
// triangulation must match to insure that the mesh is compatible. The
// algorithm employs a precomputed table to accelerate the tessellation
// process. The table was generated with the help of vtkOrderedTriangulator;
// the basic idea is that the choice of diagonal is made by considering the
// relative value of the point ids.


#ifndef __vtkGenericCellTessellator_h
#define __vtkGenericCellTessellator_h

#include "vtkObject.h"

class vtkCellArray;
class vtkDoubleArray;
class vtkGenericSubdivisionErrorMetric;
class vtkGenericAttributeCollection;
class vtkGenericAdaptorCell;
class vtkGenericCellIterator;
class vtkPointData;
class vtkGenericDataSet;

//-----------------------------------------------------------------------------
//
// The tessellation object
class VTK_FILTERING_EXPORT vtkGenericCellTessellator : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkGenericCellTessellator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Tessellate a face of a tetrahedron cell. The face is specified by the
  // index value.
  // \pre cell_exists: cell!=0
  // \pre is_a_tetra: (cell->GetType()==VTK_TETRA)
  //                 ||(cell->GetType()==VTK_QUADRATIC_TETRA)
  //                 ||(cell->GetType()==VTK_HIGHER_ORDER_TETRAHEDRON)
  // \pre valid_index_range: (index>=0) && (index<4)
  virtual void TessellateTriangleFace(vtkGenericAdaptorCell *cell,
                                      vtkGenericAttributeCollection *att,
                                      vtkIdType index,
                                      vtkDoubleArray *points,
                                      vtkCellArray *cellArray,
                                      vtkPointData *internalPd)=0;

  // Description:
  // Tessellate a tetrahedron `cell'. The result is a set of smaller linear
  // cells `cellArray' with `points' and point data `scalars'.
  // \pre cell_exists: cell!=0
  // \pre is_a_tetra: (cell->GetType()==VTK_TETRA)
  //                 ||(cell->GetType()==VTK_QUADRATIC_TETRA)
  //                 ||(cell->GetType()==VTK_HIGHER_ORDER_TETRAHEDRON)
  virtual void Tessellate(vtkGenericAdaptorCell *cell,
                          vtkGenericAttributeCollection *att,
                          vtkDoubleArray *points,
                          vtkCellArray *cellArray,
                          vtkPointData *internalPd)=0;

  // Description:
  // Triangulate a triangle `cell'.
  // \pre cell_exists: cell!=0
  // \pre is_a_triangle: (cell->GetType()==VTK_TRIANGLE)
  //                 ||(cell->GetType()==VTK_QUADRATIC_TRIANGLE)
  //                 ||(cell->GetType()==VTK_HIGHER_ORDER_TRIANGLE)
  virtual void Triangulate(vtkGenericAdaptorCell *cell,
                           vtkGenericAttributeCollection *att,
                           vtkDoubleArray *points,
                           vtkCellArray *cellArray,
                           vtkPointData *internalPd)=0;

  // Description:
  // Specify the object to use to compute the error metric.
  virtual void SetErrorMetric(vtkGenericSubdivisionErrorMetric *anErrorMetric);
  vtkGetObjectMacro(ErrorMetric,vtkGenericSubdivisionErrorMetric);
  
  // Description:
  // Initialize the tessellator with a data set `ds'.
  virtual void Initialize(vtkGenericDataSet *ds)=0;
  
protected:
  vtkGenericCellTessellator();
  ~vtkGenericCellTessellator();
  
  // Description:
  // Contains the error metric
  vtkGenericSubdivisionErrorMetric *ErrorMetric;
  
private:
  vtkGenericCellTessellator(const vtkGenericCellTessellator&);  // Not implemented.
  void operator=(const vtkGenericCellTessellator&);  // Not implemented.
};

#endif
