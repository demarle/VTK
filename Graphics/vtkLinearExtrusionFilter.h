/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLinearExtrusionFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLinearExtrusionFilter - sweep polygonal data creating a "skirt" from free edges and lines, and lines from vertices
// .SECTION Description
// vtkLinearExtrusionFilter is a modeling filter. It takes polygonal data as 
// input and generates polygonal data on output. The input dataset is swept
// according to some extrusion function and creates new polygonal primitives.
// These primitives form a "skirt" or swept surface. For example, sweeping a
// line results in a quadrilateral, and sweeping a triangle creates a "wedge".
//
// There are a number of control parameters for this filter. You can 
// control whether the sweep of a 2D object (i.e., polygon or triangle strip) 
// is capped with the generating geometry via the "Capping" ivar. Also, you
// can extrude in the direction of a user specified vector, towards a point,
// or in the direction of vertex normals (normals must be provided - use 
// vtkPolyDataNormals if necessary). The amount of extrusion is controlled by
// the "ScaleFactor" instance variable.
//
// The skirt is generated by locating certain topological features. Free 
// edges (edges of polygons or triangle strips only used by one polygon or
// triangle strips) generate surfaces. This is true also of lines or 
// polylines. Vertices generate lines.
//
// This filter can be used to create 3D fonts, 3D irregular bar charts,
// or to model 2 1/2D objects like punched plates. It also can be used to 
// create solid objects from 2D polygonal meshes.

// .SECTION Caveats
// Some polygonal objects have no free edges (e.g., sphere). When swept,
// this will result in two separate surfaces if capping is on, or no surface
// if capping is off.

// .SECTION See Also
// vtkRotationalExtrusionFilter

#ifndef __vtkLinearExtrusionFilter_h
#define __vtkLinearExtrusionFilter_h

#include "vtkPolyDataAlgorithm.h"

class vtkDataArray;

#define VTK_VECTOR_EXTRUSION 1
#define VTK_NORMAL_EXTRUSION 2
#define VTK_POINT_EXTRUSION 3

class VTK_GRAPHICS_EXPORT vtkLinearExtrusionFilter : public vtkPolyDataAlgorithm 
{
public:
  vtkTypeMacro(vtkLinearExtrusionFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create object with normal extrusion type, capping on, scale factor=1.0,
  // vector (0,0,1), and extrusion point (0,0,0).
  static vtkLinearExtrusionFilter *New();

  // Description:
  // Set/Get the type of extrusion.
  vtkSetClampMacro(ExtrusionType,int,VTK_VECTOR_EXTRUSION,VTK_POINT_EXTRUSION);
  vtkGetMacro(ExtrusionType,int);
  void SetExtrusionTypeToVectorExtrusion()
    {this->SetExtrusionType(VTK_VECTOR_EXTRUSION);};
  void SetExtrusionTypeToNormalExtrusion()
    {this->SetExtrusionType(VTK_NORMAL_EXTRUSION);};
  void SetExtrusionTypeToPointExtrusion()
    {this->SetExtrusionType(VTK_POINT_EXTRUSION);};

  // Description:
  // Turn on/off the capping of the skirt.
  vtkSetMacro(Capping,int);
  vtkGetMacro(Capping,int);
  vtkBooleanMacro(Capping,int);

  // Description:
  // Set/Get extrusion scale factor,
  vtkSetMacro(ScaleFactor,double);
  vtkGetMacro(ScaleFactor,double);

  // Description:
  // Set/Get extrusion vector. Only needs to be set if VectorExtrusion is
  // turned on.
  vtkSetVector3Macro(Vector,double);
  vtkGetVectorMacro(Vector,double,3);

  // Description:
  // Set/Get extrusion point. Only needs to be set if PointExtrusion is
  // turned on. This is the point towards which extrusion occurs.
  vtkSetVector3Macro(ExtrusionPoint,double);
  vtkGetVectorMacro(ExtrusionPoint,double,3);

protected:
  vtkLinearExtrusionFilter();
  ~vtkLinearExtrusionFilter() {};

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int ExtrusionType;
  int Capping;
  double ScaleFactor;
  double Vector[3];
  double ExtrusionPoint[3];

  //BTX
  double *(vtkLinearExtrusionFilter::*ExtrudePoint)(double x[3], vtkIdType id, 
                                                   vtkDataArray *normals);
  double *ViaNormal(double x[3], vtkIdType id, vtkDataArray *normals);
  double *ViaVector(double x[3], vtkIdType id, vtkDataArray *normals=0);
  double *ViaPoint(double x[3], vtkIdType id, vtkDataArray *normals=0);
  //ETX
 
private:
  vtkLinearExtrusionFilter(const vtkLinearExtrusionFilter&);  // Not implemented.
  void operator=(const vtkLinearExtrusionFilter&);  // Not implemented.
};

#endif
