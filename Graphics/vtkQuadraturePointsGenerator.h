/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQuadraturePointsGenerator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkQuadraturePointsGenerator
// .SECTION Description
//
// Create a vtkPolyData on its output containing the vertices
// for the quadrature points for one of the vtkDataArrays present
// on its input vtkUnstructuredGrid. If the input data set has
// has FieldData generated by vtkQuadraturePointInterpolator then
// this will be set as point data. Note: Point sets are generated
// per field array. This is because each field array may contain
// its own dictionary.
//
// .SECTION See also
// vtkQuadraturePointInterpolator, vtkQuadratureSchemeDefinition, vtkInformationQuadratureSchemeDefinitionVectorKey

#ifndef vtkQuadraturePointsGenerator_h
#define vtkQuadraturePointsGenerator_h

#include "vtkPolyDataAlgorithm.h"

class vtkPolyData;
class vtkUnstructuredGrid;
class vtkInformation;
class vtkInformationVector;

class VTK_GRAPHICS_EXPORT vtkQuadraturePointsGenerator : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkQuadraturePointsGenerator,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkQuadraturePointsGenerator *New();

protected:
  int FillInputPortInformation(int port, vtkInformation *info);

  int RequestData(vtkInformation *req, vtkInformationVector **input, vtkInformationVector *output);

  // Description:
  // Generate the point set .
  int Generate(vtkUnstructuredGrid *usgIn,
                vtkIdTypeArray* offsets,
                vtkPolyData *pdOut);

  int GenerateField(vtkUnstructuredGrid *usgIn,
                vtkDataArray* data,
                vtkIdTypeArray* offsets,
                vtkPolyData* pdOut);

  vtkQuadraturePointsGenerator();
  virtual ~vtkQuadraturePointsGenerator();
private:
  vtkQuadraturePointsGenerator(const vtkQuadraturePointsGenerator &); // Not implemented
  void operator=(const vtkQuadraturePointsGenerator &); // Not implemented
};

#endif
