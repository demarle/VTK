/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkArrayCalculator.cxx
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
#include "vtkArrayCalculator.h"
#include "vtkFunctionParser.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkArrayCalculator* vtkArrayCalculator::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkArrayCalculator");
  if(ret)
    {
    return (vtkArrayCalculator*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkArrayCalculator;
}

vtkArrayCalculator::vtkArrayCalculator()
{
  this->FunctionParser = vtkFunctionParser::New();
  
  this->Function = NULL;
  this->ResultArrayName = NULL;
  this->ScalarArrayNames = NULL;
  this->VectorArrayNames = NULL;
  this->NumberOfScalarArrays = 0;
  this->NumberOfVectorArrays = 0;
  this->AttributeMode = VTK_ATTRIBUTE_MODE_DEFAULT;
  this->SelectedScalarComponents = NULL;
  this->SelectedVectorComponents = NULL;
}

vtkArrayCalculator::~vtkArrayCalculator()
{
  int i;
  
  this->FunctionParser->Delete();
  this->FunctionParser = NULL;
  
  if (this->Function)
    {
    delete [] this->Function;
    this->Function = NULL;
    }

  if (this->ResultArrayName)
    {
    delete [] this->ResultArrayName;
    this->ResultArrayName = NULL;
    }
  
  if (this->ScalarArrayNames)
    {
    for (i = 0; i < this->NumberOfScalarArrays; i++)
      {
      delete [] this->ScalarArrayNames[i];
      this->ScalarArrayNames[i] = NULL;
      }
    delete [] this->ScalarArrayNames;
    this->ScalarArrayNames = NULL;
    }

  if (this->VectorArrayNames)
    {
    for (i = 0; i < this->NumberOfVectorArrays; i++)
      {
      delete [] this->VectorArrayNames[i];
      this->VectorArrayNames[i] = NULL;
      }
    delete [] this->VectorArrayNames;
    this->VectorArrayNames = NULL;
    }

  if (this->SelectedScalarComponents)
    {
    delete [] this->SelectedScalarComponents;
    this->SelectedScalarComponents = NULL;
    }
  
  if (this->SelectedVectorComponents)
    {
    for (i = 0; i < this->NumberOfVectorArrays; i++)
      {
      delete [] this->SelectedVectorComponents[i];
      this->SelectedVectorComponents[i] = NULL;
      }
    delete [] this->SelectedVectorComponents;
    this->SelectedVectorComponents = NULL;
    }
}

void vtkArrayCalculator::Execute()
{
  int resultType; // 0 for scalar, 1 for vector
  int attributeDataType; // 0 for point data, 1 for cell data
  int i, j;
  
  vtkDataSet* input = this->GetInput();
  vtkDataSet* output = this->GetOutput();
  vtkPointData* inPD = input->GetPointData();
  vtkCellData* inCD = input->GetCellData();
  vtkFieldData* inFD;
  vtkDataArray* currentArray;
  vtkDoubleArray* resultArray;
  int numPts = input->GetNumberOfPoints();
  
  if (this->AttributeMode == VTK_ATTRIBUTE_MODE_DEFAULT)
    {
    if (inPD->GetFieldData())
      {
      inFD = inPD->GetFieldData();
      attributeDataType = 0;
      }
    else if (inCD->GetFieldData())
      {
      inFD = inCD->GetFieldData();
      attributeDataType = 1;
      }
    else
      {
      vtkErrorMacro("No field data to operate on.");
      return;
      }
    }
  else if (this->AttributeMode == VTK_ATTRIBUTE_MODE_USE_POINT_DATA)
    {
    if (inPD->GetFieldData())
      {
      inFD = inPD->GetFieldData();
      attributeDataType = 0;
      }
    else
      {
      vtkErrorMacro("No point field data to operate on.");
      return;
      }
    }
  else
    {
    if (inCD->GetFieldData())
      {
      inFD = inCD->GetFieldData();
      attributeDataType = 1;
      }
    else
      {
      vtkErrorMacro("No cell field data to operate on.");
      return;
      }
    }
  
  for (i = 0; i < this->NumberOfScalarArrays; i++)
    {
    currentArray = inFD->GetArray(this->ScalarArrayNames[i]);
    if (currentArray)
      {
      if (currentArray->GetNumberOfComponents() >
          this->SelectedScalarComponents[i])
        {
        this->FunctionParser->
          SetScalarVariableValue(this->ScalarArrayNames[i],
                                 currentArray->GetComponent(i, this->SelectedScalarComponents[i]));
        }
      else
        {
        vtkErrorMacro("Array " << this->ScalarArrayNames[i]
                      << " does not contain the selected component.");
        return;
        }
      }
    else
      {
      vtkErrorMacro("Invalid array name: " << this->ScalarArrayNames[i]);
      return;
      }
    }

  for (i = 0; i < this->NumberOfVectorArrays; i++)
    {
    currentArray = inFD->GetArray(this->VectorArrayNames[i]);
    if (currentArray)
      {
      if ((currentArray->GetNumberOfComponents() >
           this->SelectedVectorComponents[i][0]) &&
          (currentArray->GetNumberOfComponents() >
           this->SelectedVectorComponents[i][1]) &&
          (currentArray->GetNumberOfComponents() >
           this->SelectedVectorComponents[i][2]))
        {
        this->FunctionParser->
          SetVectorVariableValue(this->VectorArrayNames[i],
                                 currentArray->GetComponent(i, this->SelectedVectorComponents[i][0]),
                                 currentArray->GetComponent(i, this->SelectedVectorComponents[i][1]),
                                 currentArray->GetComponent(i, this->SelectedVectorComponents[i][2]));
        }
      else
        {
        vtkErrorMacro("Array " << this->VectorArrayNames[i]
                      << " does not contain one of the selected components.");
        return;
        }
      }
    else
      {
      vtkErrorMacro("Invalid array name: " << this->ScalarArrayNames[i]);
      return;
      }
    }

  if (this->FunctionParser->IsScalarResult())
    {
    resultType = 0;
    }
  else if (this->FunctionParser->IsVectorResult())
    {
    resultType = 1;
    }
  else
    {
    // Error occurred in vtkFunctionParser.
    return;
    }

  resultArray = vtkDoubleArray::New();
  if (resultType == 0)
    {
    resultArray->Allocate(numPts);
    resultArray->SetNumberOfComponents(1);
    resultArray->SetNumberOfTuples(numPts);
    resultArray->SetComponent(0, 0, this->FunctionParser->GetScalarResult());
    }
  else
    {
    resultArray->Allocate(numPts * 3);
    resultArray->SetNumberOfComponents(3);
    resultArray->SetNumberOfTuples(numPts);
    resultArray->SetTuple(0, this->FunctionParser->GetVectorResult());
    }
  
  for (i = 1; i < numPts; i++)
    {
    for (j = 0; j < this->NumberOfScalarArrays; j++)
      {
      currentArray = inFD->GetArray(this->ScalarArrayNames[j]);
      this->FunctionParser->
        SetScalarVariableValue(j, currentArray->GetComponent(i, this->SelectedScalarComponents[j]));
      }
    for (j = 0; j < this->NumberOfVectorArrays; j++)
      {
      currentArray = inFD->GetArray(this->VectorArrayNames[j]);
      this->FunctionParser->
        SetVectorVariableValue(j, currentArray->GetComponent(i, this->SelectedVectorComponents[j][0]),
                               currentArray->GetComponent(i, this->SelectedVectorComponents[j][1]),
                               currentArray->GetComponent(i, this->SelectedVectorComponents[j][2]));      
      }
    if (resultType == 0)
      {
      resultArray->SetComponent(i, 0, this->FunctionParser->GetScalarResult());
      }
    else
      {
      resultArray->SetTuple(i, this->FunctionParser->GetVectorResult());
      }
    }
  
  output->CopyStructure(input);
  output->GetPointData()->PassData(inPD);
  output->GetCellData()->PassData(inCD);
  
  if (attributeDataType == 0)
    {
    output->GetPointData()->GetFieldData()->AddReplaceArray(resultArray, this->ResultArrayName);
    }
  else
    {
    output->GetCellData()->GetFieldData()->AddReplaceArray(resultArray, this->ResultArrayName);
    }
  resultArray->Delete();
  resultArray = NULL;
}

void vtkArrayCalculator::SetFunction(const char* function)
{
  if (this->Function && function &&
      strcmp(this->Function, function) == 0)
    {
    return;
    }
  
  if (this->Function)
    {
    delete [] this->Function;
    this->Function = NULL;
    }
  
  if (function)
    {
    this->Function = new char[strlen(function)+1];
    strcpy(this->Function, function);
    this->FunctionParser->SetFunction(this->Function);
    }
}

void vtkArrayCalculator::AddScalarArrayName(const char* arrayName,
                                            int component)
{
  if (!arrayName)
    {
    return;
    }
  
  int i;
  char** tempNames = new char *[this->NumberOfScalarArrays];
  int* tempComponents = new int[this->NumberOfScalarArrays];
  
  for (i = 0; i < this->NumberOfScalarArrays; i++)
    {
    tempNames[i] = new char[strlen(this->ScalarArrayNames[i]) + 1];
    strcpy(tempNames[i], this->ScalarArrayNames[i]);
    delete [] this->ScalarArrayNames[i];
    this->ScalarArrayNames[i] = NULL;
    tempComponents[i] = this->SelectedScalarComponents[i];
    }
  if (this->ScalarArrayNames)
    {
    delete [] this->ScalarArrayNames;
    this->ScalarArrayNames = NULL;
    }
  if (this->SelectedScalarComponents)
    {
    delete [] this->SelectedScalarComponents;
    this->SelectedScalarComponents = NULL;
    }
  
  this->ScalarArrayNames = new char *[this->NumberOfScalarArrays + 1];
  this->SelectedScalarComponents = new int[this->NumberOfScalarArrays + 1];
  
  for (i = 0; i < this->NumberOfScalarArrays; i++)
    {
    this->ScalarArrayNames[i] = new char[strlen(tempNames[i]) + 1];
    strcpy(this->ScalarArrayNames[i], tempNames[i]);
    delete [] tempNames[i];
    tempNames[i] = NULL;
    this->SelectedScalarComponents[i] = tempComponents[i];
    }
  delete [] tempNames;
  tempNames = NULL;
  delete [] tempComponents;
  tempComponents = NULL;

  this->ScalarArrayNames[i] = new char[strlen(arrayName) + 1];
  strcpy(this->ScalarArrayNames[i], arrayName);
  this->SelectedScalarComponents[i] = component;
  
  this->NumberOfScalarArrays++;
}

void vtkArrayCalculator::AddVectorArrayName(const char* arrayName,
                                            int component0, int component1,
                                            int component2)
{
  if (!arrayName)
    {
    return;
    }
  
  int i;
  char** tempNames = new char *[this->NumberOfVectorArrays];
  int** tempComponents = new int *[this->NumberOfVectorArrays];
  
  for (i = 0; i < this->NumberOfVectorArrays; i++)
    {
    tempNames[i] = new char[strlen(this->VectorArrayNames[i]) + 1];
    strcpy(tempNames[i], this->VectorArrayNames[i]);
    delete [] this->VectorArrayNames[i];
    this->VectorArrayNames[i] = NULL;
    tempComponents[i] = new int[3];
    tempComponents[i][0] = this->SelectedVectorComponents[i][0];
    tempComponents[i][1] = this->SelectedVectorComponents[i][1];
    tempComponents[i][2] = this->SelectedVectorComponents[i][2];
    delete [] this->SelectedVectorComponents[i];
    this->SelectedVectorComponents = NULL;
    }
  
  if (this->VectorArrayNames)
    {
    delete [] this->VectorArrayNames;
    this->VectorArrayNames = NULL;
    }
  if (this->SelectedVectorComponents)
    {
    delete [] this->SelectedVectorComponents;
    this->SelectedVectorComponents = NULL;
    }
  
  this->VectorArrayNames = new char *[this->NumberOfVectorArrays + 1];
  this->SelectedVectorComponents = new int *[this->NumberOfVectorArrays + 1];
  
  for (i = 0; i < this->NumberOfVectorArrays; i++)
    {
    this->VectorArrayNames[i] = new char[strlen(tempNames[i]) + 1];
    strcpy(this->VectorArrayNames[i], tempNames[i]);
    delete [] tempNames[i];
    tempNames[i] = NULL;
    this->SelectedVectorComponents[i] = new int[3];
    this->SelectedVectorComponents[i][0] = component0;
    this->SelectedVectorComponents[i][1] = component1;
    this->SelectedVectorComponents[i][2] = component2;
    delete [] tempComponents[i];
    tempComponents[i] = NULL;
    }
  delete [] tempNames;
  tempNames = NULL;
  delete [] tempComponents;
  tempComponents = NULL;

  this->VectorArrayNames[i] = new char[strlen(arrayName) + 1];
  strcpy(this->VectorArrayNames[i], arrayName);
  this->SelectedVectorComponents[i] = new int[3];
  this->SelectedVectorComponents[i][0] = component0;
  this->SelectedVectorComponents[i][1] = component1;
  this->SelectedVectorComponents[i][2] = component2;
  
  this->NumberOfVectorArrays++;
}

const char* vtkArrayCalculator::GetAttributeModeAsString()
{
  if ( this->AttributeMode == VTK_ATTRIBUTE_MODE_DEFAULT )
    {
    return "Default";
    }
  else if ( this->AttributeMode == VTK_ATTRIBUTE_MODE_USE_POINT_DATA )
    {
    return "UsePointData";
    }
  else 
    {
    return "UseCellData";
    }
}

void vtkArrayCalculator::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "Function: " 
     << (this->Function ? this->Function : "(none)") << endl;
  os << indent << "ResultArrayName: "
     << (this->ResultArrayName ? this->ResultArrayName : "(none)") << endl;
  os << indent << "AttributeMode: " << this->GetAttributeModeAsString() << endl;
}
