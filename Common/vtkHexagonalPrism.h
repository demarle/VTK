/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkHexagonalPrism.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkHexagonalPrism - a 3D cell that represents a prism with
// hexagonal base 
// .SECTION Description
// vtkHexagonalPrism is a concrete implementation of vtkCell to represent a
// linear 3D prism with hexagonal base. Such prism is defined by the twelve points 
// (0-12) where (0,1,2,3,4,5) is the base of the prism which, using the right 
// hand rule, forms a hexagon whose normal points is in the direction of the 
// opposite face (6,7,8,9,10,11).

// .SECTION Thanks
// Thanks to Philippe Guerville who developed this class.
// Thanks to Charles Pignerol (CEA-DAM, France) who ported this class under
// VTK 4.
// Thanks to Jean Favre (CSCS, Switzerland) who contributed to integrate this
// class in VTK.
// Please address all comments to Jean Favre (jfavre at cscs.ch).


#ifndef __vtkHexagonalPrism_h
#define __vtkHexagonalPrism_h

#include "vtkCell3D.h"

class vtkLine;
class vtkPolygon;
class vtkQuad;
class vtkHexahedron;
class vtkDoubleArray;

class VTK_COMMON_EXPORT vtkHexagonalPrism : public vtkCell3D
{
public:
  static vtkHexagonalPrism *New();
  vtkTypeRevisionMacro(vtkHexagonalPrism,vtkCell3D);

  // Description:
  // See vtkCell3D API for description of these methods.
  virtual void GetEdgePoints(int edgeId, int* &pts);
  virtual void GetFacePoints(int faceId, int* &pts);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  int GetCellType() {return VTK_HEXAGONAL_PRISM;};
  int GetCellDimension() {return 3;};
  int GetNumberOfEdges() {return 18;};
  int GetNumberOfFaces() {return 8;};
  vtkCell *GetEdge(int edgeId);
  vtkCell *GetFace(int faceId);
  int CellBoundary(int subId, double pcoords[3], vtkIdList *pts);
  void Contour(double value, vtkDataArray *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys,
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);
  void Clip(double value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *tetras,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd, int insideOut);

  int EvaluatePosition(double x[3], double* closestPoint,
                       int& subId, double pcoords[3],
                       double& dist2, double *weights);
  void EvaluateLocation(int& subId, double pcoords[3], double x[3],
                        double *weights);
  int IntersectWithLine(double p1[3], double p2[3], double tol, double& t,
                        double x[3], double pcoords[3], int& subId);
  int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts);
  void Derivatives(int subId, double pcoords[3], double *values, 
                   int dim, double *derivs);
  double *GetParametricCoords();

  // Description:
  // Return the center of the wedge in parametric coordinates.
  int GetParametricCenter(double pcoords[3]);

  // Description:
  // Hexagonal prism specific
  static void InterpolationFunctions(double pcoords[3], double weights[12]);
  static void InterpolationDerivs(double pcoords[3], double derivs[36]);
  static int *GetEdgeArray(int edgeId);
  static int *GetFaceArray(int faceId);

  // Description:
  // Given parametric coordinates compute inverse Jacobian transformation
  // matrix. Returns 9 elements of 3x3 inverse Jacobian plus interpolation
  // function derivatives.
  void JacobianInverse(double pcoords[3], double **inverse, double derivs[36]);

protected:
  vtkHexagonalPrism();
  ~vtkHexagonalPrism();

  vtkLine          *Line;
  vtkQuad          *Quad;
  vtkPolygon       *Polygon;
  vtkHexahedron    *Hexahedron;
  vtkPointData     *PointData;
  vtkCellData      *CellData;
  vtkDoubleArray   *Scalars; //used to avoid New/Delete in contouring/clipping

  void Subdivide(vtkPointData *inPd, vtkCellData *inCd, vtkIdType cellId);

private:
  vtkHexagonalPrism(const vtkHexagonalPrism&);  // Not implemented.
  void operator=(const vtkHexagonalPrism&);  // Not implemented.
};

inline int vtkHexagonalPrism::GetParametricCenter(double pcoords[3])
{
  pcoords[0] = pcoords[1] = 0.333333;
  pcoords[2] = 0.5;
  return 0;
}
#endif


