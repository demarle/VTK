/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParametricDini.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParametricDini - Generate Dini's surface.
// .SECTION Description
// vtkParametricDini generates Dini's surface.
// Dini's surface is a surface that posesses constant negative 
// Gaussian curvature
// .SECTION Thanks
// Andrew Maclean a.maclean@cas.edu.au for 
// creating and contributing the class.
//
#ifndef __vtkParametricDini_h
#define __vtkParametricDini_h

#include "vtkParametricFunction.h"

class VTK_COMMON_EXPORT vtkParametricDini : public vtkParametricFunction
{
public:

  vtkTypeRevisionMacro(vtkParametricDini,vtkParametricFunction);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Construct Dini's surface with the following parameters:
  // MinimumU = 0, MaximumU = 4*Pi,
  // MinimumV = 0.001, MaximumV = 2, 
  // JoinU = 0, JoinV = 0,
  // TwistU = 0, TwistV = 0, 
  // ClockwiseOrdering = 1, 
  // DerivativesAvailable = 1
  // A = 1, B = 0.2
  static vtkParametricDini *New();

  // Description
  // Return the parametric dimension of the class.
  virtual int GetDimension() {return 2;}

  // Description:
  // Set/Get the scale factor. 
  // Default = 1.
  vtkSetMacro(A,double);
  vtkGetMacro(A,double);

  // Description:
  // Set/Get the scale factor. 
  // Default = 0.2
  vtkSetMacro(B,double);
  vtkGetMacro(B,double);

  // Description:
  // Dini's surface is an example of a surface that posesses
  // constant negative Gaussian curvature.
  //
  // A parametric representation of Dini's surface
  // Define:
  // -  X(u,v) = a*cos(u)*sin(v)
  // -  Y(u,v) = a*sin(u)*sin(v)
  // -  Z(u,v) = a*(cos(v)+log(tan((v/2))))+b*u
  //
  // Then
  // - S(u,v) = (X(u,v),Y(u,v),Z(u,v)) defines the surface. 
  //
  // The derivatives are given by:
  // - d(X(u,v)/du = -Y(u,v)
  // - d(X(u,v)/dv = a*cos(u)*cos(v)
  // - d(Y(u,v)/du = X(u,v)
  // - d(Y(u,v)/dv = a*sin(u)*cos(v)
  // - d(Z(u,v)/du = b
  // - d(Z(u,v)/dv = a*(-sin(v)+(1/2+1/2*tan(1/2*v)^2)/tan(1/2*v))
  //
  // Let Du = (dy/du, dy/du, dy/du)
  //
  // Let Dv = (dx/dv, dy/dv, dz/dv)
  //
  // Then the normal n = Du X Dv
  //
  // Warning: This function may fail if tan(1/2*v) = 0;
  //
  // This function performs the mapping fn(u,v)->(x,y,x), returning it
  // as Pt. It also returns the partial derivatives Du and Dv.
  // Pt = (x, y, z), Du = (dx/du, dy/du, dz/du), Dv = (dx/dv, dy/dv, dz/dv)
  void Evaluate(double uvw[3], double Pt[3], double Duvw[9]);

  // Description:
  // Calculate a user defined scalar using one or all of uvw,Pt,Duvw.
  //
  // uvw are the parameters with Pt being the the cartesian point, 
  // Duvw are the derivatives of this point with respect to u, v and w.
  // Pt, Duvw are obtained from Evaluate().
  //
  // This function is only called if the ScalarMode has the value
  // vtkParametricTriangulator::userDefined
  //
  // If the user does not need to calculate a scalar, then the 
  // instantiated function should return zero. 
  //
  double EvaluateScalar(double uvw[3], double Pt[3], double Duvw[9]);

protected:
  vtkParametricDini();
  ~vtkParametricDini();

  // Variables
  double A;
  double B;

private:
  vtkParametricDini(const vtkParametricDini&);  // Not implemented.
  void operator=(const vtkParametricDini&);  // Not implemented.
};

#endif
