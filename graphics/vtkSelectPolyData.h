/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSelectPolyData.h
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
// .NAME vtkSelectPolyData - select portion of polygonal mesh; generate selection scalars
// .SECTION Description
// vtkSelectPolyData is a filter that selects polygonal data based on
// defining a "loop" and indicating the region inside of the loop. The
// mesh within the loop consists of complete cells (the cells are not
// cut). Alternatively, this filter can be used to generate scalars.
// These scalar values, which are a distance measure to the loop, can
// be used to clip, contour. or extract data (i.e., anything that an
// implicit function can do). 
//
// The loop is defined by an array of x-y-z point coordinates.
// (Coordinates should be in the same coordinate space as the input
// polygonal data.) The loop can be concave and non-planar, but not
// self-intersecting. The input to the filter is a polygonal mesh
// (only surface primitives such as triangle strips and polygons); the
// output is either a) a portion of the original mesh laying within
// the selection loop (GenerateSelectionScalarsOff); or b) the same
// polygonal mesh with the addition of scalar values
// (GenerateSelectionScalarsOn).
//
// The algorithm works as follows. For each point coordinate in the
// loop, the closest point in the mesh is found. The result is a loop
// of closest point ids from the mesh. Then, the edges in the mesh
// connecting the closest points (and laying along the lines forming
// the loop) are found. A greedy edge tracking procedure is used as
// follows. At the current point, the mesh edge oriented in the
// direction of and whose end point is closest to the line is
// chosen. The edge is followed to the new end point, and the
// procedure is repeated. This process continues until the entire loop
// has been created. 
// 
// To determine what portion of the mesh is inside and outside of the
// loop, three options are possible. 1) the smallest connected region,
// 2) the largest connected region, and 3) the connected region
// closest to a user specified point. (Set the ivar SelectionMode.)
// 
// Once the loop is computed as above, the GenerateSelectionScalars
// controls the output of the filter. If on, then scalar values are
// generated based on distance to the loop lines. Otherwise, the cells
// laying inside the selection loop are output. By default, the mesh
// lying within the loop is output; however, if InsideOut is on, then
// the portion of the mesh lying outside of the loop is output.
//
// The filter can be configured to generate the unselected portions of
// the mesh as output by setting GenerateUnselectedOutput. Use the
// method GetUnselectedOutput to access this output. (Note: this flag
// is pertinent only when GenerateSelectionScalars is off.)

// .SECTION Caveats
// Make sure that the points you pick are on a connected surface. If
// not, then the filter will generate an empty or partial result. Also,
// self-intersecting loops will generate unpredictable results.
//
// During processing of the data, non-triangular cells are converted to
// triangles if GenerateSelectionScalars is off.

// .SECTION See Also
// vtkImplicitSelectionLoop

#ifndef __vtkSelectPolyData_h
#define __vtkSelectPolyData_h

#include "vtkPolyDataToPolyDataFilter.h"

#define VTK_INSIDE_SMALLEST_REGION 0
#define VTK_INSIDE_LARGEST_REGION 1
#define VTK_INSIDE_CLOSEST_POINT_REGION 2

class vtkCharArray;

class VTK_EXPORT vtkSelectPolyData : public vtkPolyDataToPolyDataFilter
{
public:
  // Description:
  // Instantiate object with InsideOut turned off, and 
  // GenerateSelectionScalars turned off. The unselected output
  // is not generated, and the inside mode is the smallest region.
  vtkSelectPolyData();
  ~vtkSelectPolyData();
  static vtkSelectPolyData *New() {return new vtkSelectPolyData;};
  const char *GetClassName() {return "vtkSelectPolyData";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the flag to control beahvior of the filter. If
  // GenerateSelectionScalars is on, then the output of the filter
  // is the same as the input, except that scalars are generated.
  // If off, the filter outputs the cells laying inside the loop, and
  // does not generate scalars.
  vtkSetMacro(GenerateSelectionScalars,int);
  vtkGetMacro(GenerateSelectionScalars,int);
  vtkBooleanMacro(GenerateSelectionScalars,int);

  // Description:
  // Set/Get the InsideOut flag. When off, the mesh within the loop is
  // extracted. When on, the mesh outside the loop is extracted.
  vtkSetMacro(InsideOut,int);
  vtkGetMacro(InsideOut,int);
  vtkBooleanMacro(InsideOut,int);

  // Description:
  // Set/Get the array of point coordinates defining the loop. There must
  // be at least three points used to define a loop.
  vtkSetReferenceCountedObjectMacro(Loop,vtkPoints);
  vtkGetObjectMacro(Loop,vtkPoints);

  // Description:
  // Control how inside/outside of loop is defined.
  vtkSetClampMacro(SelectionMode,int,
             VTK_INSIDE_SMALLEST_REGION,VTK_INSIDE_CLOSEST_POINT_REGION);
  vtkGetMacro(SelectionMode,int);
  void SetSelectionModeToSmallestRegion()
    {this->SetSelectionMode(VTK_INSIDE_SMALLEST_REGION);};
  void SetSelectionModeToLargestRegion()
    {this->SetSelectionMode(VTK_INSIDE_LARGEST_REGION);};
  void SetSelectionModeToClosestPointRegion()
    {this->SetSelectionMode(VTK_INSIDE_CLOSEST_POINT_REGION);};
  char *GetSelectionModeAsString();

  // Description:
  // Control whether a second output is generated. The second output
  // contains the polygonal data that's not been selected.
  vtkSetMacro(GenerateUnselectedOutput,int);
  vtkGetMacro(GenerateUnselectedOutput,int);
  vtkBooleanMacro(GenerateUnselectedOutput,int);

  // Description:
  // Return output that hasn't been selected (if GenreateUnselectedOutput is
  // enabled).
  vtkPolyData *GetUnselectedOutput() {return this->UnselectedOutput;};

  // Description:
  // Return the (mesh) edges of the selection region.
  vtkPolyData *GetSelectionEdges() {return this->SelectionEdges;};

  // Overload GetMTime() because we depend on Loop
  unsigned long int GetMTime();

protected:
  void Execute();

  int GenerateSelectionScalars;
  int InsideOut;
  vtkPoints *Loop;
  int SelectionMode;
  float ClosestPoint[3];
  int GenerateUnselectedOutput;
  vtkPolyData *UnselectedOutput;
  vtkPolyData *SelectionEdges;

private:
  vtkPolyData *Mesh;
  void GetPointNeighbors (int ptId, vtkIdList *nei);

};

// Description:
// Return the method of determining in/out of loop as a string.
inline char *vtkSelectPolyData::GetSelectionModeAsString(void)
{
  if ( this->SelectionMode == VTK_INSIDE_SMALLEST_REGION ) 
    {
    return "InsideSmallestRegion";
    }
  else if ( this->SelectionMode == VTK_INSIDE_LARGEST_REGION ) 
    {
    return "InsideLargestRegion";
    }
  else 
    {
    return "InsideClosestPointRegion";
    }
}

#endif


