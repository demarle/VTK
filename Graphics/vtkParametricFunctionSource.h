/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParametricFunctionSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParametricFunctionSource - tessellate parametric functions
// .SECTION Description
// This class tessellates parametric functions. The user must specify how
// many points in the parametric coordinate directions are required, and 
// mode to use to generate scalars.
//
// .SECTION Thanks
// Andrew Maclean a.maclean@cas.edu.au for creating and contributing the
// class.
//
// .SECTION See Also
// vtkParametricFunction
//
// Implementations of derived classes implementing non-orentable surfaces:
// vtkParametricBoy vtkParametricCrossCap vtkParametricFig8Klein
// vtkParametricKlein vtkParametricMobius vtkParametricRoman
//
// Implementations of derived classes implementing orientable surfaces:
// vtkParametricConicSpiral vtkParametricDini vtkParametricEnneper
// vtkParametricRandomHills vtkParametricSuperEllipsoid 
// vtkParametricSuperToroid vtkParametricTorus 
//
#ifndef __vtkParametricFunctionSource_h
#define __vtkParametricFunctionSource_h

#include "vtkPolyDataAlgorithm.h"

class vtkCellArray;
class vtkParametricFunction;

class VTK_GRAPHICS_EXPORT vtkParametricFunctionSource : public vtkPolyDataAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkParametricFunctionSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a new instance with (50,50,50) points in the (u-v-w) directions.
  static vtkParametricFunctionSource *New();

  // Description:
  // Specify the parametric function to use to generate the tessellation.
  virtual void SetParametricFunction(vtkParametricFunction*);
  vtkGetObjectMacro(ParametricFunction,vtkParametricFunction);

  // Description:
  // Set/Get the number of tessellant points in the u-direction.
  vtkSetMacro(NumberOfUPoints,int);
  vtkGetMacro(NumberOfUPoints,int);

  // Description:
  // Set/Get the number of tessellant points in the v-direction.
  vtkSetMacro(NumberOfVPoints,int);
  vtkGetMacro(NumberOfVPoints,int);

  // Description:
  // Set/Get the number of tessellant points in the w-direction.
  vtkSetMacro(NumberOfWPoints,int);
  vtkGetMacro(NumberOfWPoints,int);

  //BTX
  // Description:
  // Enumerate the supported scalar generation modes.
  // <pre>
  // SCALAR_NONE, (default) scalars are not generated.
  // SCALAR_U, the scalar is set to the u-value. 
  // SCALAR_V, the scalar is set to the v-value.
  // SCALAR_U0, the scalar is set to 1 if u = (u_max - u_min)/2 = u_avg, 0 otherwise.
  // SCALAR_V0, the scalar is set to 1 if v = (v_max - v_min)/2 = v_avg, 0 otherwise.
  // SCALAR_U0V0, the scalar is 
  //   set to 1 if u == u_avg, 2 if v == v_avg, 3 if u = u_avg && v = v_avg, 0 otherwise.
  // SCALAR_MODULUS, the scalar is set to (sqrt(u*u+v*v)), this is measured relative to (u_avg,v_avg).
  // SCALAR_PHASE, the scalar is set to (atan2(v,u)) (in degrees, 0 to 360), this is measured relative to (u_avg,v_avg).
  // SCALAR_QUADRANT, the scalar is set to 1, 2, 3 or 4 
  //   depending upon the quadrant of the point (u,v).
  // SCALAR_X, the scalar is set to the x-value. 
  // SCALAR_Y, the scalar is set to the y-value. 
  // SCALAR_Z, the scalar is set to the z-value. 
  // SCALAR_DISTANCE, the scalar is set to (sqrt(x*x+y*y+z*z)). I.e. distance from the origin.
  // SCALAR_USER_DEFINED, the scalar is set to the value returned from EvaluateScalar().
  // </pre>
  enum SCALAR_MODE { SCALAR_NONE = 0, 
    SCALAR_U, SCALAR_V, 
    SCALAR_U0, SCALAR_V0, SCALAR_U0V0,
    SCALAR_MODULUS, SCALAR_PHASE, SCALAR_QUADRANT,
    SCALAR_X, SCALAR_Y, SCALAR_Z, SCALAR_DISTANCE,
    SCALAR_FUNCTION_DEFINED };
  //ETX

  // Description:
  // Get/Set the mode used for the scalar data.  The options are:
  // SCALAR_NONE, (default) scalars are not generated.
  // SCALAR_U, the scalar is set to the u-value. 
  // SCALAR_V, the scalar is set to the v-value.
  // SCALAR_U0, the scalar is set to 1 if u = (u_max - u_min)/2 = u_avg, 0 otherwise.
  // SCALAR_V0, the scalar is set to 1 if v = (v_max - v_min)/2 = v_avg, 0 otherwise.
  // SCALAR_U0V0, the scalar is 
  //   set to 1 if u == u_avg, 2 if v == v_avg, 3 if u = u_avg && v = v_avg, 0 otherwise.
  // SCALAR_MODULUS, the scalar is set to (sqrt(u*u+v*v)), this is measured relative to (u_avg,v_avg).
  // SCALAR_PHASE, the scalar is set to (atan2(v,u)) (in degrees, 0 to 360), this is measured relative to (u_avg,v_avg).
  // SCALAR_QUADRANT, the scalar is set to 1, 2, 3 or 4 
  //   depending upon the quadrant of the point (u,v).
  // SCALAR_X, the scalar is set to the x-value. 
  // SCALAR_Y, the scalar is set to the y-value. 
  // SCALAR_Z, the scalar is set to the z-value. 
  // SCALAR_DISTANCE, the scalar is set to (sqrt(x*x+y*y+z*z)). I.e. distance from the origin.
  // SCALAR_FUNCTION_DEFINED, the scalar is set to the value returned from EvaluateScalar().
  vtkSetClampMacro(ScalarMode, int, SCALAR_NONE, SCALAR_FUNCTION_DEFINED);
  vtkGetMacro(ScalarMode, int);
  void SetScalarModeToNone( void ) {this->SetScalarMode(SCALAR_NONE);}
  void SetScalarModeToU( void ) {this->SetScalarMode(SCALAR_U);}
  void SetScalarModeToV( void ) {this->SetScalarMode(SCALAR_V);}
  void SetScalarModeToU0( void ) {this->SetScalarMode(SCALAR_U0);}
  void SetScalarModeToV0( void ) {this->SetScalarMode(SCALAR_V0);}
  void SetScalarModeToU0V0( void ) {this->SetScalarMode(SCALAR_U0V0);}
  void SetScalarModeToModulus( void ) {this->SetScalarMode(SCALAR_MODULUS);}
  void SetScalarModeToPhase( void ) {this->SetScalarMode(SCALAR_PHASE);}
  void SetScalarModeToQuadrant( void ) {this->SetScalarMode(SCALAR_QUADRANT);} 
  void SetScalarModeToX( void ) {this->SetScalarMode(SCALAR_X);}
  void SetScalarModeToY( void ) {this->SetScalarMode(SCALAR_Y);}
  void SetScalarModeToZ( void ) {this->SetScalarMode(SCALAR_Z);}
  void SetScalarModeToDistance( void ) {this->SetScalarMode(SCALAR_DISTANCE);}
  void SetScalarModeToFunctionDefined( void ) {this->SetScalarMode(SCALAR_FUNCTION_DEFINED);}

protected:
  vtkParametricFunctionSource();
  virtual ~vtkParametricFunctionSource();

  // Usual data generation method
  int RequestData(vtkInformation *info, vtkInformationVector **input,
                  vtkInformationVector *output);

  // Variables
  vtkParametricFunction *ParametricFunction;
  
  int NumberOfUPoints;
  int NumberOfVPoints;
  int NumberOfWPoints;
  int ScalarMode;

private:
  // Description:
  // Generate triangle strips from an ordered set of points.
  //
  // Given a parametrization f(u,v)->(x,y,z), this function generates 
  // a vtkCellAarray of point IDs over the range MinimumU <= u < MaximumU 
  // and MinimumV <= v < MaximumV.
  //
  // Before using this function, ensure that: NumberOfUPoints,
  // NumberOfVPoints, MinimumU, MaximumU, MinimumV, MaximumV, JoinU, JoinV,
  // TwistU, TwistV, ordering are set appropriately for the parametric function.
  //
  void MakeTriangleStrips ( vtkCellArray * strips, int PtsU, int PtsV );
  
  vtkParametricFunctionSource(const vtkParametricFunctionSource&);  // Not implemented.
  void operator=(const vtkParametricFunctionSource&);  // Not implemented.

};

#endif
