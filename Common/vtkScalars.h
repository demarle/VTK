/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkScalars.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkScalars - represent and manipulate scalar data
// .SECTION Description
// vtkScalars provides an interface to scalar data.  The data model for
// vtkScalars is an array accessible by point or cell id.  Scalar data is
// represented by a subclass of vtkDataArray, and may be any any type of
// data, although generic operations on scalar data is performed using
// floating point values.
//
// Scalars typically provide a single value per point. However, there are
// types of scalars that have multiple values or components. (For example,
// scalars that represent (RGB) color information, etc.) In this case,
// the ActiveComponent instance variable is used to indicate which
// component value is to be used for scalar data.
//
// Because of the close relationship between scalars and colors, scalars 
// also maintain an internal lookup table. If provided, this table is used 
// to map scalars into colors, rather than the lookup table that the vtkMapper
// objects are associated with.
//
// .SECTION See Also
// vtkDataArray vtkAttributeData vtkPointData vtkCellData

#ifndef __vtkScalars_h
#define __vtkScalars_h

#include "vtkAttributeData.h"

class vtkIdList;
class vtkScalars;
class vtkLookupTable;
class vtkUnsignedCharArray;
class vtkScalarsToColors;

class VTK_COMMON_EXPORT vtkScalars : public vtkAttributeData
{
public:
  static vtkScalars *New();
  static vtkScalars *New(int dataType, int numComp=1);

  vtkTypeMacro(vtkScalars,vtkAttributeData);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the data for this object. The tuple dimension must be consistent with
  // the object.
  void SetData(vtkDataArray *);

  // Description:
  // Create a copy of this object.
  vtkAttributeData *MakeObject();

  // Description:
  // Specify the number of scalars for this object to hold. Make sure
  // that you set the number of components in texture first.
  void SetNumberOfScalars(vtkIdType number)
    {this->Data->SetNumberOfTuples(number);};

  // Description:
  // Return number of scalars in the array.
  vtkIdType GetNumberOfScalars() {return this->Data->GetNumberOfTuples();};

  // Description:
  // Return the scalar value as a float for a specific id.
  float GetScalar(vtkIdType id);

  // Description:
  // Insert Scalar into object. No range checking performed (fast!).
  // Make sure you use SetNumberOfScalars() to allocate memory prior
  // to using SetScalar().
  void SetScalar(vtkIdType id, float s);

  // Description:
  // Insert Scalar into object. Range checking performed and memory
  // allocated as necessary.
  void InsertScalar(vtkIdType id, float s);

  // Description:
  // Insert Scalar at end of array and return its location (id) in the array.
  vtkIdType InsertNextScalar(float s);

  // Description:
  // Specify/Get the number of components in scalar data.
  void SetNumberOfComponents(int num);
  int GetNumberOfComponents() {return this->Data->GetNumberOfComponents();}

  // Description:
  // Set/Get the active scalar component. This ivar specifies which
  // value (or component) to use with multivalued scalars.
  vtkSetMacro(ActiveComponent, int);
  vtkGetMacro(ActiveComponent, int);

  // Special computational methods.

  // Description:
  // Determine (rmin,rmax) range of scalar values.
  void ComputeRange();


  // Description:
  // Return the range of scalar values. Data returned as pointer to float array
  // of length 2.
  float *GetRange();


  // Description:
  // Return the range of scalar values. Range copied into array provided.
  void GetRange(float range[2]);

  // Description:
  // These methods return the Min and Max possible range of the native
  // data type. For example if a vtkScalars consists of unsigned char
  // data these will return (0,255). 
  void GetDataTypeRange(double range[2]);
  double GetDataTypeMin();
  double GetDataTypeMax();
  
  // Description:
  // Create default lookup table. Generally used to create one when none
  // is available.
  virtual void CreateDefaultLookupTable();

  // Description:
  // Set/get the lookup table associated with this scalar data, if any.
  void SetLookupTable(vtkLookupTable *lut);
  vtkLookupTable* GetLookupTable();

  // Description:
  // Given a list of point ids, return an array of scalar values.
  void GetScalars(vtkIdList *ptIds, vtkScalars *fv);

  // Description:
  // Get the scalar values for the range of points ids specified 
  // (i.e., p1->p2 inclusive). You must insure that the vtkScalars has 
  // been previously allocated with enough space to hold the data.
  void GetScalars(vtkIdType p1, vtkIdType p2, vtkScalars *fs);
  
  // Description:
  // Initialize the traversal of the scalar data to generate colors 
  // (typically used during the rendering process). The method takes
  // an alpha opacity value and returns a flag indicating whether alpha
  // blending will occur. Also takes a lookup table used to map the 
  // scalar data. The color mode parameter controls how the scalar data 
  // is mapped to colors (see vtkMapper::ColorMode methods for a definition).
  int InitColorTraversal(float alpha, vtkScalarsToColors *lut, 
			 int colorMode=0);
  
  // Description:
  // Get the color value at a particular id. Returns a pointer to a 4-byte
  // array of rgba. Make sure you call InitColorTraversal() before
  // invoking this method. (This method is obsolete; use the 
  // vtkMapper::MapScalars() method.)
  unsigned char *GetColor(vtkIdType id) 
    {
    VTK_LEGACY_METHOD("GetColor", "4.0");
    return (this->*(this->CurrentColorFunction))(id);
    }

protected:
  vtkScalars();
  ~vtkScalars();

  float Range[8];
  vtkTimeStamp ComputeTime;

  // following stuff is used for converting scalars to colors
  float CurrentAlpha;
  vtkScalarsToColors *CurrentLookupTable;
  //BTX
  unsigned char *(vtkScalars::*CurrentColorFunction)(vtkIdType id);
  unsigned char *PassRGBA(vtkIdType id);
  unsigned char *PassRGB(vtkIdType id);
  unsigned char *PassIA(vtkIdType id);
  unsigned char *PassI(vtkIdType id);
  unsigned char *CompositeRGBA(vtkIdType id);
  unsigned char *CompositeIA(vtkIdType id);
  unsigned char *CompositeMapThroughLookupTable(vtkIdType id);
  unsigned char *MapThroughLookupTable(vtkIdType id);
  unsigned char *Luminance(vtkIdType id);
  vtkUnsignedCharArray *Colors;
  unsigned char RGBA[4];
  int ActiveComponent;
  //ETX
private:
  vtkScalars(const vtkScalars&);  // Not implemented.
  void operator=(const vtkScalars&);  // Not implemented.
};

inline vtkAttributeData *vtkScalars::MakeObject()
{
  return vtkScalars::New(this->GetDataType(),this->GetNumberOfComponents());
}

inline void vtkScalars::SetNumberOfComponents(int num)
{
  num = (num < 1 ? 1 : num);
  this->Data->SetNumberOfComponents(num);
}

inline float vtkScalars::GetScalar(vtkIdType id)
{
  return this->Data->GetComponent(id,this->GetActiveComponent());
}

inline void vtkScalars::SetScalar(vtkIdType id, float s)
{
  this->Data->SetComponent(id,this->GetActiveComponent(),s);
}

inline void vtkScalars::InsertScalar(vtkIdType id, float s)
{
  this->Data->InsertComponent(id,this->GetActiveComponent(),s);
}

inline vtkIdType vtkScalars::InsertNextScalar(float s)
{
  int tupleSize = this->Data->GetNumberOfComponents();
  vtkIdType id=(this->Data->GetMaxId() + tupleSize)/tupleSize;
  this->Data->InsertComponent(id,this->GetActiveComponent(),s);
  return id;
}

// These include files are placed here so that if Scalars.h is included 
// all other classes necessary for compilation are also included. 
#include "vtkIdList.h"

#endif
