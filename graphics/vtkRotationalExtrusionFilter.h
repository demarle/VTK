/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRotationalExtrusionFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkRotationalExtrusionFilter - sweep polygonal data creating "skirt" from free edges and lines, and lines from vertices
// .SECTION Description
// vtkRotationalExtrusionFilter is a modelling filter. It takes polygonal 
// data as input and generates polygonal data on output. The input dataset 
// is swept around the z-axis to create new polygonal primitives. These 
// primitives form a "skirt" or swept surface. For example, sweeping a
// line results in a cylindrical shell, and sweeping a circle creates a 
// torus.
//
// There are a number of control parameters for this filter. You can 
// control whether the sweep of a 2D object (i.e., polygon or triangle 
// strip) is capped with the generating geometry via the "Capping" instance
// variable. Also, you can control the angle of rotation, and whether 
// translation along the z-axis is performed along with the rotation.
// (Translation is useful for creating "springs".) You also can adjust 
// the radius of the generating geometry using the "DeltaRotation" instance 
// variable.
//
// The skirt is generated by locating certain topological features. Free 
// edges (edges of polygons or triangle strips only used by one polygon or
// triangle strips) generate surfaces. This is true also of lines or 
// polylines. Vertices generate lines.
//
// This filter can be used to model axisymmetric objects like cylinders,
// bottles, and wine glasses; or translational/rotational symmetric objects
// like springs or corkscrews.

// .SECTION Caveats
// If the object sweeps 360 degrees, radius does not vary, and the object 
// does not translate, capping is not performed. This is because the cap 
// is unnecessary.
//
// Some polygonal objects have no free edges (e.g., sphere). When swept,
// this will result in two separate surfaces if capping is on, or no surface
// if capping is off.

// .SECTION See Also
// vtkLinearExtrusionFilter

#ifndef __vtkRotationalExtrusionFilter_h
#define __vtkRotationalExtrusionFilter_h

#include "vtkPolyDataToPolyDataFilter.h"

class VTK_EXPORT vtkRotationalExtrusionFilter : public vtkPolyDataToPolyDataFilter 
{
public:
  const char *GetClassName() {return "vtkRotationalExtrusionFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create object with capping on, angle of 360 degrees, resolution = 12, and
  // no translation along z-axis.
  // vector (0,0,1), and point (0,0,0).
  static vtkRotationalExtrusionFilter *New() {
    return new vtkRotationalExtrusionFilter;};

  // Description:
  // Set/Get resolution of sweep operation. Resolution controls the number
  // of intermediate node points.
  vtkSetClampMacro(Resolution,int,1,VTK_LARGE_INTEGER);
  vtkGetMacro(Resolution,int);

  // Description:
  // Turn on/off the capping of the skirt.
  vtkSetMacro(Capping,int);
  vtkGetMacro(Capping,int);
  vtkBooleanMacro(Capping,int);

  // Description:
  // Set/Get angle of rotation.
  vtkSetMacro(Angle,float);
  vtkGetMacro(Angle,float);

  // Description:
  // Set/Get total amount of translation along the z-axis.
  vtkSetMacro(Translation,float);
  vtkGetMacro(Translation,float);

  // Description:
  // Set/Get change in radius during sweep process.
  vtkSetMacro(DeltaRadius,float);
  vtkGetMacro(DeltaRadius,float);

protected:
  vtkRotationalExtrusionFilter();
  ~vtkRotationalExtrusionFilter() {};
  void Execute();
  int Resolution;
  int Capping;
  float Angle;
  float Translation;
  float DeltaRadius;
};

#endif
