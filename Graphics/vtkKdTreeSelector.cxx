/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkKdTreeSelector.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/

#include "vtkKdTreeSelector.h"

#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkKdTree.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkSelection.h"

vtkCxxRevisionMacro(vtkKdTreeSelector, "1.7");
vtkStandardNewMacro(vtkKdTreeSelector);

vtkKdTreeSelector::vtkKdTreeSelector()
{
  this->SelectionBounds[0] = 0.0;
  this->SelectionBounds[1] = -1.0;
  this->SelectionBounds[2] = 0.0;
  this->SelectionBounds[3] = -1.0;
  this->SelectionBounds[4] = VTK_DOUBLE_MIN;
  this->SelectionBounds[5] = VTK_DOUBLE_MAX;
  this->KdTree = 0;
  this->BuildKdTreeFromInput = true;
  this->SelectionFieldName = 0;
  this->SingleSelection = false;
  this->SingleSelectionThreshold = 1.0;
  this->SelectionAttribute = -1;
}

vtkKdTreeSelector::~vtkKdTreeSelector()
{
  this->SetKdTree(0);
  this->SetSelectionFieldName(0);
}

void vtkKdTreeSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "KdTree: " << (this->KdTree ? "" : "(null)") << endl;
  if (this->KdTree)
    {
    this->KdTree->PrintSelf(os, indent.GetNextIndent());
    }
  os << indent << "SelectionFieldName: "
    << (this->SelectionFieldName ? this->SelectionFieldName : "(null)") << endl;
  os << indent << "BuildKdTreeFromInput: " 
    << (this->BuildKdTreeFromInput ? "on" : "off") << endl;
  os << indent << "SelectionBounds: " << endl;
  os << indent << "  xmin, xmax = (" << this->SelectionBounds[0]
    << "," << this->SelectionBounds[1] << ")" << endl;
  os << indent << "  ymin, ymax = (" << this->SelectionBounds[2]
    << "," << this->SelectionBounds[3] << ")" << endl;
  os << indent << "  zmin, zmax = (" << this->SelectionBounds[4]
    << "," << this->SelectionBounds[5] << ")" << endl;
  os << indent << "SingleSelection: "
    << (this->SingleSelection ? "on" : "off") << endl;
  os << indent << "SingleSelectionThreshold: " << this->SingleSelectionThreshold << endl;
  os << indent << "SelectionAttribute: " << this->SelectionAttribute << endl;
}

void vtkKdTreeSelector::SetKdTree(vtkKdTree* arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this
                << "): setting KdTree to " << arg );
  if (this->KdTree != arg)
    {
    vtkKdTree* tempSGMacroVar = this->KdTree;
    this->KdTree = arg;
    if (this->KdTree != NULL) 
      { 
      this->BuildKdTreeFromInput = false;
      this->KdTree->Register(this); 
      }
    else
      {
      this->BuildKdTreeFromInput = true;
      }
    if (tempSGMacroVar != NULL)
      {
      tempSGMacroVar->UnRegister(this);
      }
    this->Modified();
    }
}

unsigned long vtkKdTreeSelector::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();
  if (this->KdTree != NULL)
    {
    unsigned long time = this->KdTree->GetMTime();
    mTime = (time > mTime ? time : mTime);
    }
  return mTime;
}

int vtkKdTreeSelector::RequestData(
  vtkInformation* vtkNotUsed(request), 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  vtkAbstractArray* field = NULL;

  if (this->BuildKdTreeFromInput)
    {
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    if (inInfo == NULL)
      {
      vtkErrorMacro("No input, but building kd-tree from input");
      return 0;
      }
    vtkPointSet* input = vtkPointSet::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));
    if (input == NULL)
      {
      vtkErrorMacro("Input is NULL");
      return 0;
      }

    // If no points, there is nothing to do
    if (input->GetPoints() == NULL || input->GetNumberOfPoints() == 0)
      {
      return 1;
      }

    // Construct the kd-tree if we need to
    if (this->KdTree == NULL || this->KdTree->GetMTime() < input->GetMTime())
      {
      if (this->KdTree == NULL)
        {
        this->KdTree = vtkKdTree::New();
        }
      this->KdTree->Initialize();
      this->KdTree->BuildLocatorFromPoints(input->GetPoints());
      }
    
    // Look for selection field
    if (this->SelectionAttribute == vtkDataSetAttributes::GLOBALIDS ||
        this->SelectionAttribute == vtkDataSetAttributes::PEDIGREEIDS)
      {
      field = input->GetPointData()->GetAbstractAttribute(this->SelectionAttribute);
      if (field == NULL)
        {
        vtkErrorMacro("Could not find attribute " << this->SelectionAttribute);
        return 0;
        }
      }
    if (this->SelectionFieldName)
      {
      field = input->GetPointData()->GetAbstractArray(this->SelectionFieldName);
      if (field == NULL)
        {
        vtkErrorMacro("SelectionFieldName field not found");
        return 0;
        }
      }
    }

  // If no kd-tree, there is nothing to do
  if (this->KdTree == NULL)
    {
    return 1;
    }

  // Use the kd-tree to find the selected points
  vtkIdTypeArray* ids = vtkIdTypeArray::New();
  if (this->SingleSelection)
    {
    double center[3];
    for (int c = 0; c < 3; c++)
      {
      center[c] = (this->SelectionBounds[2*c] + this->SelectionBounds[2*c+1]) / 2.0;
      }
    double dist;
    vtkIdType closestToCenter = this->KdTree->FindClosestPoint(center, dist);
    if (dist < this->SingleSelectionThreshold)
      {
      ids->InsertNextValue(closestToCenter);
      }
    }
  else
    {
    this->KdTree->FindPointsInArea(this->SelectionBounds, ids);
    }

  // Fill the selection with the found ids
  vtkSelection* output = vtkSelection::GetData(outputVector);
  output->GetProperties()->Set(output->FIELD_TYPE(), vtkSelection::POINT);
  if (field)
    {
    vtkAbstractArray* arr = vtkAbstractArray::CreateArray(field->GetDataType());
    arr->SetName(field->GetName());
    for (vtkIdType i = 0; i < ids->GetNumberOfTuples(); i++)
      {
      arr->InsertNextTuple(ids->GetValue(i), field);
      }
    if (this->SelectionAttribute == vtkDataSetAttributes::GLOBALIDS ||
        this->SelectionAttribute == vtkDataSetAttributes::PEDIGREEIDS)
      {
      if (this->SelectionAttribute == vtkDataSetAttributes::GLOBALIDS)
        {
        output->GetProperties()->Set(output->CONTENT_TYPE(), vtkSelection::GLOBALIDS);
        }
      else if (this->SelectionAttribute == vtkDataSetAttributes::PEDIGREEIDS)
        {
        output->GetProperties()->Set(output->CONTENT_TYPE(), vtkSelection::PEDIGREEIDS);
        }
      }
    else
      {
      output->GetProperties()->Set(output->CONTENT_TYPE(), vtkSelection::VALUES);
      }
    output->SetSelectionList(arr);
    arr->Delete();
    }
  else
    {
    output->GetProperties()->Set(output->CONTENT_TYPE(), vtkSelection::INDICES);
    output->SetSelectionList(ids);
    }

  // Clean up
  ids->Delete();

  return 1;
}

int vtkKdTreeSelector::FillInputPortInformation(
  int vtkNotUsed(port), 
  vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

