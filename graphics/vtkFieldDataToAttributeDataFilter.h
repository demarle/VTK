/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkFieldDataToAttributeDataFilter.h
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
// .NAME vtkFieldDataToAttributeDataFilter - map field data to dataset attribute data
// .SECTION Description
// vtkFieldDataToAttributeDataFilter is a class that maps field data into
// dataset attributes. The input to this filter is any type of dataset and
// the output is the same dataset (geometry/topology) with new attribute data
// (attribute data is passed through if not replaced during filter
// execution).
//
// To use this filter you must specify which field data from the input
// dataset to use. There are three possibilities: the cell field data, the
// point field data, or the field data associated with the data object
// superclass. Then you specify which attribute data to create: either cell
// attribute data or point attribute data.  Finally, you must define how to
// construct the various attribute data types (e.g., scalars, vectors,
// normals, etc.) from the arrays and the components of the arrays from the
// field data. This is done by associating components in the input field with
// components making up the attribute data. For example, you would specify a
// scalar with three components (RGB) by assigning components from the field
// for the R, then G, then B values of the scalars.  You may also have to
// specify component ranges (for each R-G-B) to make sure that the number of
// R, G, and B values is the same. Also, you may want to normalize the
// components which helps distribute the data uniformly.
//
// This filter is often used in conjunction with
// vtkDataObjectToDataSetFilter.  vtkDataObjectToDataSetFilter filter
// generates dataset topology and geometry and passes its input field data
// along to its output. Then this filter is used to generate the attribute
// data to go along with the daraset.

// .SECTION Caveats
// Make sure that the data you extract is consistent. That is, if you have N
// points, extract N point attributes (scalars, vectors, etc.). 

// .SECTION See Also
// vtkFieldData vtkDataSet vtkDataObjectToDataSetFilter
// vtkDataSetAttributes vtkDataArray

#ifndef __vtkFieldDataToAttributeDataFilter_h
#define __vtkFieldDataToAttributeDataFilter_h

#include "vtkDataSetToDataSetFilter.h"

#define VTK_DATA_OBJECT_FIELD 0
#define VTK_POINT_DATA_FIELD 1
#define VTK_CELL_DATA_FIELD 2

#define VTK_CELL_DATA 0
#define VTK_POINT_DATA 1

class VTK_EXPORT vtkFieldDataToAttributeDataFilter : public vtkDataSetToDataSetFilter
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);
  const char *GetClassName() {return "vtkFieldDataToAttributeDataFilter";};

  // Description:
  // Construct object with input field set to the data object field, and the
  // output attribute data set to generate point data.
  static vtkFieldDataToAttributeDataFilter *New() 
    {return new vtkFieldDataToAttributeDataFilter;};

  // Description:
  // Specify which field data to use to generate the output attribute
  // data. There are three choices: the field data associated with the
  // vtkDataObject superclass; the point field attribute data; and the cell
  // field attribute data.
  vtkSetMacro(InputField,int);
  vtkGetMacro(InputField,int);
  void SetInputFieldToDataObjectField() 
    {this->SetInputField(VTK_DATA_OBJECT_FIELD);};
  void SetInputFieldToPointDataField() 
    {this->SetInputField(VTK_POINT_DATA_FIELD);};
  void SetInputFieldToCellDataField() 
    {this->SetInputField(VTK_CELL_DATA_FIELD);};
  
  // Description:
  // Specify which attribute data to output: point or cell data attributes.
  vtkSetMacro(OutputAttributeData,int);
  vtkGetMacro(OutputAttributeData,int);
  void SetOutputAttributeDataToCellData() 
    {this->SetOutputAttributeData(VTK_CELL_DATA);};
  void SetOutputAttributeDataToPointData() 
    {this->SetOutputAttributeData(VTK_POINT_DATA);};

  // Description:
  // Define the component(s) of the field to be used for the scalar
  // components.  Note that the parameter comp must lie between (0,4). To
  // define the field component to use you specify an array name and the
  // component in that array. The (min,max) values are the range of data in
  // the component you wish to extract.
  void SetScalarComponent(int comp, char *arrayName, int arrayComp,
			  int min, int max, int normalize);
  void SetScalarComponent(int comp, char *arrayName, int arrayComp)
    {this->SetScalarComponent(comp, arrayName, arrayComp, -1, -1, this->DefaultNormalize);};
  const char *GetScalarComponentArrayName(int comp);
  int GetScalarComponentArrayComponent(int comp);
  int GetScalarComponentMinRange(int comp);
  int GetScalarComponentMaxRange(int comp);
  int GetScalarComponentNormalizeFlag(int comp);
  
  // Description:
  // Define the component(s) of the field to be used for the vector
  // components.  Note that the parameter comp must lie between (0,3). To
  // define the field component to use you specify an array name and the
  // component in that array. The (min,max) values are the range of data in
  // the component you wish to extract.
  void SetVectorComponent(int comp, char *arrayName, int arrayComp,
			  int min, int max, int normalize);
  void SetVectorComponent(int comp, char *arrayName, int arrayComp)
    {this->SetVectorComponent(comp, arrayName, arrayComp, -1, -1, this->DefaultNormalize);};
  const char *GetVectorComponentArrayName(int comp);
  int GetVectorComponentArrayComponent(int comp);
  int GetVectorComponentMinRange(int comp);
  int GetVectorComponentMaxRange(int comp);
  int GetVectorComponentNormalizeFlag(int comp);
  
  // Description:
  // Define the component(s) of the field to be used for the normal
  // components.  Note that the parameter comp must lie between (0,3). To
  // define the field component to use you specify an array name and the
  // component in that array. The (min,max) values are the range of data in
  // the component you wish to extract.
  void SetNormalComponent(int comp, char *arrayName, int arrayComp,
			  int min, int max, int normalize);
  void SetNormalComponent(int comp, char *arrayName, int arrayComp)
    {this->SetNormalComponent(comp, arrayName, arrayComp, -1, -1, this->DefaultNormalize);};
  const char *GetNormalComponentArrayName(int comp);
  int GetNormalComponentArrayComponent(int comp);
  int GetNormalComponentMinRange(int comp);
  int GetNormalComponentMaxRange(int comp);
  int GetNormalComponentNormalizeFlag(int comp);
  
  // Description:
  // Define the components of the field to be used for the tensor
  // components.  Note that the parameter comp must lie between (0,9). To
  // define the field component to use you specify an array name and the
  // component in that array. The (min,max) values are the range of data in
  // the component you wish to extract.
  void SetTensorComponent(int comp, char *arrayName, int arrayComp,
			  int min, int max, int normalize);
  void SetTensorComponent(int comp, char *arrayName, int arrayComp)
    {this->SetTensorComponent(comp, arrayName, arrayComp, -1, -1, this->DefaultNormalize);};
  const char *GetTensorComponentArrayName(int comp);
  int GetTensorComponentArrayComponent(int comp);
  int GetTensorComponentMinRange(int comp);
  int GetTensorComponentMaxRange(int comp);
  int GetTensorComponentNormalizeFlag(int comp);
  
  // Description:
  // Define the components of the field to be used for the cell texture coord
  // components.  Note that the parameter comp must lie between (0,9). To
  // define the field component to use you specify an array name and the
  // component in that array. The (min,max) values are the range of data in
  // the component you wish to extract.
  void SetTCoordComponent(int comp, char *arrayName, int arrayComp,
			  int min, int max, int normalize);
  void SetTCoordComponent(int comp, char *arrayName, int arrayComp)
    {this->SetTCoordComponent(comp, arrayName, arrayComp, -1, -1, this->DefaultNormalize);};
  const char *GetTCoordComponentArrayName(int comp);
  int GetTCoordComponentArrayComponent(int comp);
  int GetTCoordComponentMinRange(int comp);
  int GetTCoordComponentMaxRange(int comp);
  int GetTCoordComponentNormalizeFlag(int comp);
  
  // Description:
  // Set the default Normalize() flag for those methods setting a default
  // Normalize value (e.g., SetScalarComponents).
  vtkSetMacro(DefaultNormalize,int);
  vtkGetMacro(DefaultNormalize,int);
  vtkBooleanMacro(DefaultNormalize,int);

  // Helper functions, made public to support other classes

  // Description:
  // Given an array of names of arrays in field data, return the common type
  // for these arrays. For example, if a vector is constructed of the three
  // type (char,int,float), the return type is float.
  static int GetComponentsType(int numComp, vtkDataArray **arrays);

  // Description:
  // Construct a portion of a data array (the comp portion) from another data
  // array and its component. The variables min and max control the range of
  // the data to use from the other data array; normalize is a flag that when
  // set will normalize the data between (0,1).
  static int ConstructArray(vtkDataArray *da, int comp, vtkDataArray *frray,
		     int fieldComp, int min, int max, int normalize);

  // Description:
  // Return an array of a particular name from field data and do error checking.
  static vtkDataArray *GetFieldArray(vtkFieldData *fd, char *name, int comp);
  
  // Description:
  // Specify an array name for one of the components.
  static void SetArrayName(vtkObject *self, char* &name, char *newName);
  
  // Description:
  // Update the maximum and minimum component range values. Returns a flag
  // indicating whether the range was updated.
  static int UpdateComponentRange(vtkDataArray *da, int compRange[2]);

protected:
  vtkFieldDataToAttributeDataFilter();
  ~vtkFieldDataToAttributeDataFilter();

  void Execute(); //generate output data

  int InputField;
  int OutputAttributeData;

  int NumberOfScalarComponents; //the number of components to fill-in
  char *ScalarArrays[4]; //the name of the arrays used to construct the scalar
  int ScalarArrayComponents[4]; //the components of the arrays used to construct
  int ScalarComponentRange[4][2]; //the range of the components to use
  int ScalarNormalize[4]; //flags control normalization
  
  char *VectorArrays[3]; //the name of the arrays used to construct the vectors
  int VectorArrayComponents[3]; //the components of the arrays used to construct
  int VectorComponentRange[3][2]; //the range of the components to use
  int VectorNormalize[3]; //flags control normalization
  
  char *NormalArrays[3]; //the name of the arrays used to construct the normals
  int NormalArrayComponents[3]; //the components of the arrays used to construct
  int NormalComponentRange[3][2]; //the range of the components to use
  int NormalNormalize[3]; //flags control normalization
  
  char *TensorArrays[9]; //the name of the arrays used to construct the tensors
  int TensorArrayComponents[9]; //the components of the arrays used to construct
  int TensorComponentRange[9][2]; //the range of the components to use
  int TensorNormalize[9]; //flags control normalization
  
  int NumberOfTCoordComponents; //the number of components to fill-in
  char *TCoordArrays[3]; //the name of the arrays used to construct the tcoords
  int TCoordArrayComponents[3]; //the components of the arrays used to construct
  int TCoordComponentRange[3][2]; //the range of the components to use
  int TCoordNormalize[3]; //flags control normalization
  
  int DefaultNormalize;

  void ConstructScalars(int num, vtkFieldData *fd, vtkDataSetAttributes *attr, 
			int componentRange[4][2], char *arrays[4], 
			int arrayComponents[4],	int normalize[4], int numComp);
  void ConstructVectors(int num, vtkFieldData *fd, vtkDataSetAttributes *attr, 
			int componentRange[3][2], char *arrays[3], 
			int arrayComponents[3], int normalize[3]);
  void ConstructNormals(int num, vtkFieldData *fd, vtkDataSetAttributes *attr, 
			int componentRange[3][2], char *arrays[3], 
			int arrayComponents[3], int normalize[3]);
  void ConstructTCoords(int num, vtkFieldData *fd, vtkDataSetAttributes *attr, 
			int componentRange[3][2], char *arrays[3], 
			int arrayComponents[3],	int normalize[3], int numComp);
  void ConstructTensors(int num, vtkFieldData *fd, vtkDataSetAttributes *attr, 
			int componentRange[9][2], char *arrays[9], 
			int arrayComponents[9],	int normalize[9]);
  void ConstructFieldData(int num, vtkDataSetAttributes *attr);
  
};

#endif


