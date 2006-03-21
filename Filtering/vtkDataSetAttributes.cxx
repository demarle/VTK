/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataSetAttributes.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDataSetAttributes.h"

#include "vtkArrayIteratorIncludes.h"
#include "vtkCell.h"
#include "vtkMath.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkLongArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkDataSetAttributes, "1.13");
vtkStandardNewMacro(vtkDataSetAttributes);

//--------------------------------------------------------------------------
const char vtkDataSetAttributes
::AttributeNames[vtkDataSetAttributes::NUM_ATTRIBUTES][10] =
{ "Scalars",
  "Vectors",
  "Normals",
  "TCoords",
  "Tensors",
  "GlobalIds" };

const char vtkDataSetAttributes
::LongAttributeNames[vtkDataSetAttributes::NUM_ATTRIBUTES][35] =
{ "vtkDataSetAttributes::SCALARS",
  "vtkDataSetAttributes::VECTORS",
  "vtkDataSetAttributes::NORMALS",
  "vtkDataSetAttributes::TCOORDS",
  "vtkDataSetAttributes::TENSORS",
  "vtkDataSetAttributes::GLOBALIDS" };

//--------------------------------------------------------------------------
// Construct object with copying turned on for all data.
vtkDataSetAttributes::vtkDataSetAttributes()
{
  int attributeType;
  for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
    {
    this->AttributeIndices[attributeType] = -1;
    this->CopyAttributeFlags[COPYTUPLE][attributeType] = 1;
    this->CopyAttributeFlags[INTERPOLATE][attributeType] = 1;
    this->CopyAttributeFlags[PASSDATA][attributeType] = 1;
    }

  //Global IDs should not be interpolated because they are labels, not "numbers"
  //Global IDs should not be copied either, unless doing so preserves meaning.
  //Passing through is ussually OK because it is 1:1.
  this->CopyAttributeFlags[COPYTUPLE][GLOBALIDS] = 0;
  this->CopyAttributeFlags[INTERPOLATE][GLOBALIDS] = 0;

  this->TargetIndices=0;

}

//--------------------------------------------------------------------------
// Destructor for the vtkDataSetAttributes objects.
vtkDataSetAttributes::~vtkDataSetAttributes()
{
  this->Initialize();
  delete[] this->TargetIndices;
  this->TargetIndices = 0;
}

//--------------------------------------------------------------------------
// Turn on copying of all data.
void vtkDataSetAttributes::CopyAllOn(int ctype)
{
  this->vtkFieldData::CopyAllOn();
  this->SetCopyScalars(1, ctype);
  this->SetCopyVectors(1, ctype);
  this->SetCopyNormals(1, ctype);
  this->SetCopyTCoords(1, ctype);
  this->SetCopyTensors(1, ctype);
  this->SetCopyGlobalIds(1, ctype);
}

//--------------------------------------------------------------------------
// Turn off copying of all data.
void vtkDataSetAttributes::CopyAllOff(int ctype)
{
  this->vtkFieldData::CopyAllOff();
  this->SetCopyScalars(0, ctype);
  this->SetCopyVectors(0, ctype);
  this->SetCopyNormals(0, ctype);
  this->SetCopyTCoords(0, ctype);
  this->SetCopyTensors(0, ctype);
  this->SetCopyGlobalIds(0, ctype);
}

//--------------------------------------------------------------------------
// Deep copy of data (i.e., create new data arrays and
// copy from input data). Note that attribute data is
// not copied.
void vtkDataSetAttributes::DeepCopy(vtkFieldData *fd)
{

  this->Initialize(); //free up memory

  vtkDataSetAttributes* dsa = vtkDataSetAttributes::SafeDownCast(fd);
  // If the source is a vtkDataSetAttributes
  if (dsa)
    {
    int numArrays = fd->GetNumberOfArrays();
    int attributeType, i;
    vtkAbstractArray *data, *newData;

    // Allocate space for numArrays
    this->AllocateArrays(numArrays);
    for (i=0; i < numArrays; i++ )
      {
      data = fd->GetAbstractArray(i);
      newData = data->NewInstance(); //instantiate same type of object
      newData->DeepCopy(data);
      newData->SetName(data->GetName());
      if ((attributeType=dsa->IsArrayAnAttribute(i)) != -1)
        {
        // If this array is an attribute in the source, make it so
        // in the target as well.
        this->SetAttribute(vtkDataArray::SafeDownCast(newData), attributeType);
        }
      else
        {
        this->AddArray(newData);
        }
      newData->Delete();
      }
    // Copy the copy flags
    for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
      {
      this->CopyAttributeFlags[COPYTUPLE][attributeType] = 
        dsa->CopyAttributeFlags[COPYTUPLE][attributeType];
      this->CopyAttributeFlags[INTERPOLATE][attributeType] = 
        dsa->CopyAttributeFlags[INTERPOLATE][attributeType];
      this->CopyAttributeFlags[PASSDATA][attributeType] = 
        dsa->CopyAttributeFlags[PASSDATA][attributeType];
      }
    this->CopyFlags(dsa);
    }
  // If the source is field data, do a field data copy
  else
    {
    this->vtkFieldData::DeepCopy(fd);
    }
          
}

//--------------------------------------------------------------------------
// Shallow copy of data (i.e., use reference counting).
void vtkDataSetAttributes::ShallowCopy(vtkFieldData *fd)
{
  this->Initialize(); //free up memory

  vtkDataSetAttributes* dsa = vtkDataSetAttributes::SafeDownCast(fd);
  // If the source is a vtkDataSetAttributes
  if (dsa)
    {
    int numArrays = fd->GetNumberOfArrays();
    int attributeType, i;

    // Allocate space for numArrays
    this->AllocateArrays(numArrays);
    this->NumberOfActiveArrays = 0;
    for (i=0; i < numArrays; i++ )
      {
      this->NumberOfActiveArrays++;
      this->SetArray(i, fd->GetAbstractArray(i));
      if ((attributeType=dsa->IsArrayAnAttribute(i)) != -1)
        {
        // If this array is an attribute in the source, make it so
        // in the target as well.
        this->SetActiveAttribute(i, attributeType);
        }
      }
    // Copy the copy flags
    for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
      {
      this->CopyAttributeFlags[COPYTUPLE][attributeType] = 
        dsa->CopyAttributeFlags[COPYTUPLE][attributeType];
      this->CopyAttributeFlags[INTERPOLATE][attributeType] = 
        dsa->CopyAttributeFlags[INTERPOLATE][attributeType];
      this->CopyAttributeFlags[PASSDATA][attributeType] = 
        dsa->CopyAttributeFlags[PASSDATA][attributeType];
      }
    this->CopyFlags(dsa);
    }
  // If the source is field data, do a field data copy
  else
    {
    this->vtkFieldData::ShallowCopy(fd);
    }
}

//--------------------------------------------------------------------------
// Initialize all of the object's data to NULL
void vtkDataSetAttributes::InitializeFields()
{
  this->vtkFieldData::InitializeFields();

  int attributeType;
  for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
    {
    this->AttributeIndices[attributeType] = -1;
    this->CopyAttributeFlags[COPYTUPLE][attributeType] = 1;
    this->CopyAttributeFlags[INTERPOLATE][attributeType] = 1;
    this->CopyAttributeFlags[PASSDATA][attributeType] = 1;
    }
  this->CopyAttributeFlags[COPYTUPLE][GLOBALIDS] = 0;
  this->CopyAttributeFlags[INTERPOLATE][GLOBALIDS] = 0;
  //this->CopyAttributeFlags[PASSDATA][GLOBALIDS] = 1; here just for clarity
}

//--------------------------------------------------------------------------
// Initialize all of the object's data to NULL
void vtkDataSetAttributes::Initialize()
{
//
// We don't modify ourselves because the "ReleaseData" methods depend upon
// no modification when initialized.
//

// Call superclass' Initialize()
  this->vtkFieldData::Initialize();
//
// Free up any memory
// And don't forget to reset the attribute copy flags.
  int attributeType;
  for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
    {
    this->AttributeIndices[attributeType] = -1;
    this->CopyAttributeFlags[COPYTUPLE][attributeType] = 1;
    this->CopyAttributeFlags[INTERPOLATE][attributeType] = 1;
    this->CopyAttributeFlags[PASSDATA][attributeType] = 1;
    }
  this->CopyAttributeFlags[COPYTUPLE][GLOBALIDS] = 0;
  this->CopyAttributeFlags[INTERPOLATE][GLOBALIDS] = 0;
  //this->CopyAttributeFlags[PASSDATA][GLOBALIDS] = 1; here just for clarity
}

//--------------------------------------------------------------------------
// This method is used to determine which arrays
// will be copied to this object
vtkFieldData::BasicIterator  vtkDataSetAttributes::ComputeRequiredArrays(
  vtkDataSetAttributes* pd, int ctype)
{
  if ((ctype < COPYTUPLE) || (ctype > PASSDATA))
    {
    vtkErrorMacro("Must call compute required with COPYTUPLE, INTERPOLATE or PASSDATA");
    ctype = COPYTUPLE;
    }

  // We need to do some juggling to find the number of arrays
  // which will be passed.

  // First, find the number of arrays to be copied because they
  // are in the list of _fields_ to be copied (and the actual data
  // pointer is non-NULL). Also, we keep those indices in a list.
  int* copyFlags = new int[pd->GetNumberOfArrays()];
  int index, i, numArrays = 0;
  for(i=0; i<pd->GetNumberOfArrays(); i++)
    {
    const char* arrayName = pd->GetArrayName(i);
    // If there is no blocker for the given array
    // and both CopyAllOff and CopyOn for that array are not true
    if ( (this->GetFlag(arrayName) != 0) &&
         !(this->DoCopyAllOff && (this->GetFlag(arrayName) != 1)) &&
         pd->GetAbstractArray(i))
      {
      copyFlags[numArrays] = i;
      numArrays++;
      }
    }

  // Next, we check the arrays to be copied because they are one of
  // the _attributes_ to be copied (and the data array in non-NULL). 
  // We make sure that we don't count anything twice.
  int alreadyCopied;
  int attributeType, j;
  for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
    {
    index = pd->AttributeIndices[attributeType];
    int flag = this->GetFlag(pd->GetArrayName(index));
    // If this attribute is to be copied
    if (this->CopyAttributeFlags[ctype][attributeType] && flag)
      {
      // Find out if it is also in the list of fields to be copied
      // Since attributes can only be vtkDataArray, we use GetArray() call.
      if (pd->GetArray(index))
        {
        alreadyCopied = 0;
        for(i=0; i<numArrays; i++)
          {
          if ( index == copyFlags[i] )
            {
            alreadyCopied = 1;
            }
          }
        // If not, increment the number of arrays to be copied.
        if (!alreadyCopied)
          {
          copyFlags[numArrays] = index;
          numArrays++;
          }
        }
      }
    // If it is not to be copied and it is in the list (from the
    // previous pass), remove it
    else
      {
      for(i=0; i<numArrays; i++)
        {
        if ( index == copyFlags[i] )
          {
          for(j=i; j<numArrays-1; j++)
            {
            copyFlags[j] = copyFlags[j+1];
            }
          numArrays--;
          i--;
          }
        }
      }
    }
  vtkFieldData::BasicIterator it(copyFlags, numArrays);
  delete[] copyFlags;
  return it;
}

//--------------------------------------------------------------------------
// Pass entire arrays of input data through to output. Obey the "copy"
// flags.
void vtkDataSetAttributes::PassData(vtkFieldData* fd)
{
  if (!fd)
    {
    return;
    }

  vtkDataSetAttributes* dsa = vtkDataSetAttributes::SafeDownCast(fd);

  if (dsa)
    {
    // Create an iterator to iterate over the fields which will
    // be passed, i.e. fields which are either:
    // 1> in the list of _fields_ to be copied or
    // 2> in the list of _attributes_ to be copied.
    // Note that NULL data arrays are not copied

    vtkFieldData::BasicIterator it = this->ComputeRequiredArrays(dsa, PASSDATA);
    
    if ( it.GetListSize() > this->NumberOfArrays )
      {
      this->AllocateArrays(it.GetListSize());
      }
    if (it.GetListSize() == 0)
      {
      return;
      }
    
    // Since we are replacing, remove old attributes
    int attributeType; //will change//
    for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
      {
      if (this->CopyAttributeFlags[PASSDATA][attributeType])
        {
        this->RemoveArray(this->AttributeIndices[attributeType]);
        this->AttributeIndices[attributeType] = -1;
        }
      }
    
    int i, arrayIndex;
    for(i=it.BeginIndex(); !it.End(); i=it.NextIndex())
      {
      arrayIndex = this->AddArray(dsa->GetAbstractArray(i));
      // If necessary, make the array an attribute
      if ( ((attributeType = dsa->IsArrayAnAttribute(i)) != -1 ) && 
           this->CopyAttributeFlags[PASSDATA][attributeType] )
        {
        this->SetActiveAttribute(arrayIndex, attributeType);
        }
      }
    }
  else
    {
    this->vtkFieldData::PassData(fd);
    }
}




//----------------------------------------------------------------------------
template <class iterT>
void vtkDataSetAttributesCopyValues(
  iterT* destIter, const int* outExt, int outIncs[3], int rowLength,
  iterT* srcIter, const int* vtkNotUsed(inExt), int inIncs[3])
{
  // For vtkDataArray subclasses.
  int data_type_size = srcIter->GetArray()->GetDataTypeSize();
  unsigned char *inPtr;
  unsigned char *outPtr;
  unsigned char *inZPtr;
  unsigned char *outZPtr;
  
  // Get the starting input pointer.
  inZPtr = (unsigned char*)(srcIter->GetArray()->GetVoidPointer(0));
  // Shift to the start of the subextent.
  inZPtr += (outExt[0]-outExt[0])*inIncs[0] * data_type_size +
    (outExt[2] - outExt[2])*inIncs[1] * data_type_size +
    (outExt[4] - outExt[4])*inIncs[2] * data_type_size;

  // Get output pointer.
  outZPtr = (unsigned char*)(destIter->GetArray()->GetVoidPointer(0));

  // Loop over z axis.
  int zIdx, yIdx;
  for (zIdx = outExt[4]; zIdx <= outExt[5]; ++zIdx)
    {
    inPtr = inZPtr;
    outPtr = outZPtr;
    for (yIdx = outExt[2]; yIdx <= outExt[3]; ++yIdx)
      {
      memcpy(outPtr, inPtr, rowLength * data_type_size);
      inPtr += inIncs[1] * data_type_size;
      outPtr += outIncs[1] * data_type_size;
      }
    inZPtr += inIncs[2] * data_type_size;
    outZPtr += outIncs[2] * data_type_size;
    }
}

//----------------------------------------------------------------------------
VTK_TEMPLATE_SPECIALIZE
void vtkDataSetAttributesCopyValues(
  vtkArrayIteratorTemplate<vtkStdString>* destIter, const int* outExt, 
  int outIncs[3], int rowLength,
  vtkArrayIteratorTemplate<vtkStdString>* srcIter, 
  const int* vtkNotUsed(inExt), int inIncs[3])
{
  int inZIndex =  (outExt[0]-outExt[0])*inIncs[0] +
    (outExt[2] - outExt[2])*inIncs[1] +
    (outExt[4] - outExt[4])*inIncs[2] ;

  int outZIndex = 0;
  int inIndex;
  int outIndex;

  int zIdx, yIdx, xIdx;
  for (zIdx = outExt[4]; zIdx <= outExt[5]; ++zIdx)
    {
    inIndex = inZIndex;
    outIndex = outZIndex;
    for (yIdx = outExt[2]; yIdx <= outExt[3]; ++yIdx)
      {
      for (xIdx = 0; xIdx < rowLength; ++xIdx)
        {
        destIter->GetValue(outIndex + xIdx) = srcIter->GetValue(inIndex + xIdx);
        }
      inIndex += inIncs[1];
      outIndex += outIncs[1];
      }
    inZIndex += inIncs[2];
    outZIndex += outIncs[2];
    }
}

//----------------------------------------------------------------------------
// This is used in the imaging pipeline for copying arrays.
// CopyAllocate needs to be called before this method. 
void vtkDataSetAttributes::CopyStructuredData(vtkDataSetAttributes *fromPd,
                                          const int *inExt, const int *outExt)
{
  int i;
  
  for(i=this->RequiredArrays.BeginIndex(); !this->RequiredArrays.End(); 
      i=this->RequiredArrays.NextIndex())
    {
    vtkDataArray *inArray = vtkDataArray::SafeDownCast(fromPd->Data[i]);
    vtkDataArray *outArray = vtkDataArray::SafeDownCast(this->Data[this->TargetIndices[i]]);
    int inIncs[3];
    int outIncs[3];
    int rowLength;
    int zIdx;

    // Compute increments
    inIncs[0] = /*inArray->GetDataTypeSize() * */ inArray->GetNumberOfComponents();
    inIncs[1] = inIncs[0] * (inExt[1]-inExt[0]+1);
    inIncs[2] = inIncs[1] * (inExt[3]-inExt[2]+1);
    outIncs[0] = inIncs[0];
    outIncs[1] = outIncs[0] * (outExt[1]-outExt[0]+1);
    outIncs[2] = outIncs[1] * (outExt[3]-outExt[2]+1);
    // Length of continuous data to copy (one row).
    rowLength = (outExt[1]-outExt[0]+1)*outIncs[0];

    // Make sure the input extents match the actual array lengths.
    zIdx = (inExt[1]-inExt[0]+1)*(inExt[3]-inExt[2]+1)*(inExt[5]-inExt[4]+1);
    if (inArray->GetNumberOfTuples() != zIdx)
      {
      vtkErrorMacro("Input extent (" << inExt[0] << ", " << inExt[1] << ", " 
                    << inExt[2] << ", " << inExt[3] << ", " << inExt[4] << ", " 
                    << inExt[5] << ") does not match array length: " << zIdx);
      // Skip copying this array.
      continue;
      }
    // Make sure the output extents match the actual array lengths.
    zIdx = (outExt[1]-outExt[0]+1)*(outExt[3]-outExt[2]+1)*(outExt[5]-outExt[4]+1);
    if (outArray->GetNumberOfTuples() != zIdx)
      {
      // The "CopyAllocate" method only sets the size, not the number of tuples.
      outArray->SetNumberOfTuples(zIdx);
      }

    vtkArrayIterator* srcIter = inArray->NewIterator();
    vtkArrayIterator* destIter = outArray->NewIterator();
    
    switch (inArray->GetDataType())
      {
      vtkArrayIteratorTemplateMacro(
        vtkDataSetAttributesCopyValues(
          VTK_TT::SafeDownCast(destIter), outExt, outIncs, rowLength,
          VTK_TT::SafeDownCast(srcIter), inExt, inIncs));
      }
    srcIter->Delete();
    destIter->Delete();
    }
}

//--------------------------------------------------------------------------
// Allocates point data for point-by-point (or cell-by-cell) copy operation.  
// If sze=0, then use the input DataSetAttributes to create (i.e., find 
// initial size of) new objects; otherwise use the sze variable.
void vtkDataSetAttributes::InternalCopyAllocate(vtkDataSetAttributes* pd, 
                                                int ctype,
                                                vtkIdType sze, vtkIdType ext)
{
  vtkAbstractArray* newAA;
  int i;

  // Create various point data depending upon input
  //
  if ( !pd )
    {
    return;
    }

  if ((ctype < COPYTUPLE) || (ctype > PASSDATA))
    {
    return;
    }

  this->RequiredArrays = this->ComputeRequiredArrays(pd, ctype);
  if (this->RequiredArrays.GetListSize() == 0)
    {
    return;
    }
  delete[] this->TargetIndices;
  this->TargetIndices = new int[pd->GetNumberOfArrays()];
  for(i=0; i<pd->GetNumberOfArrays(); i++)
    {
    this->TargetIndices[i] = -1;
    }

  vtkAbstractArray* aa=0;
  // If we are not copying on self
  if ( pd != this )
    {
    int attributeType;

    for(i=this->RequiredArrays.BeginIndex(); !this->RequiredArrays.End(); 
        i=this->RequiredArrays.NextIndex())
      {
      // Create all required arrays
      aa = pd->GetAbstractArray(i);
      newAA = aa->NewInstance();
      newAA->SetNumberOfComponents(aa->GetNumberOfComponents());
      newAA->SetName(aa->GetName());
      if ( sze > 0 )
        {
        newAA->Allocate(sze*aa->GetNumberOfComponents(),ext);
        }
      else
        {
        newAA->Allocate(aa->GetNumberOfTuples());
        }
      vtkDataArray* newDA = vtkDataArray::SafeDownCast(newAA);
      if (newDA)
        {
        vtkDataArray* da = vtkDataArray::SafeDownCast(aa);
        newDA->SetLookupTable(da->GetLookupTable());
        }
      this->TargetIndices[i] = this->AddArray(newAA);
      // If necessary, make the array an attribute
      if ( ((attributeType = pd->IsArrayAnAttribute(i)) != -1 ) && 
           this->CopyAttributeFlags[ctype][attributeType] )
        {
        this->SetActiveAttribute(this->TargetIndices[i], attributeType);
        }
      newAA->Delete();
      }
    }
  else
    {
    // If copying on self, resize the arrays and initialize
    // TargetIndices
    for(i=this->RequiredArrays.BeginIndex(); !this->RequiredArrays.End(); 
        i=this->RequiredArrays.NextIndex())
      {
      aa = pd->GetAbstractArray(i);
      aa->Resize(sze);
      this->TargetIndices[i] = i;
      }
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::RemoveArray(int index)
{
  if ( (index<0) || (index>=this->NumberOfActiveArrays))
    {
    return;
    }
  this->Superclass::RemoveArray(index);
  int attributeType;
  for(attributeType = 0; attributeType < NUM_ATTRIBUTES; attributeType++)
    {
    if (this->AttributeIndices[attributeType] == index)
      {
      this->AttributeIndices[attributeType] = -1;
      }
    else if (this->AttributeIndices[attributeType] > index)
      {
      this->AttributeIndices[attributeType]--;
      }
    }
}

//--------------------------------------------------------------------------
// Copy the attribute data from one id to another. Make sure CopyAllocate() has
// been invoked before using this method.
void vtkDataSetAttributes::CopyData(vtkDataSetAttributes* fromPd,
                                    vtkIdType fromId, vtkIdType toId)
{
  int i;
  for(i=this->RequiredArrays.BeginIndex(); !this->RequiredArrays.End(); 
      i=this->RequiredArrays.NextIndex())
    {
    this->CopyTuple(fromPd->Data[i], this->Data[this->TargetIndices[i]], 
                    fromId, toId);
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::CopyAllocate(vtkDataSetAttributes* pd,
                                        vtkIdType sze, vtkIdType ext)
{
  this->InternalCopyAllocate(pd, COPYTUPLE, sze, ext);
}

// Initialize point interpolation method.
void vtkDataSetAttributes::InterpolateAllocate(vtkDataSetAttributes* pd,
                                               vtkIdType sze, vtkIdType ext)
{
  this->InternalCopyAllocate(pd, INTERPOLATE, sze, ext);
}

//--------------------------------------------------------------------------
// Interpolate data from points and interpolation weights. Make sure that the 
// method InterpolateAllocate() has been invoked before using this method.
void vtkDataSetAttributes::InterpolatePoint(vtkDataSetAttributes *fromPd, 
                                            vtkIdType toId, vtkIdList *ptIds, 
                                            double *weights)
{
  int i;
  for(i=this->RequiredArrays.BeginIndex(); !this->RequiredArrays.End(); 
      i=this->RequiredArrays.NextIndex())
    {
    vtkAbstractArray* fromArray = this->Data[this->TargetIndices[i]];    
    //check if the destination array needs nearest neighbor interpolation
    int attributeIndex = this->IsArrayAnAttribute(this->TargetIndices[i]);
    if (attributeIndex>0 
        && 
        this->CopyAttributeFlags[INTERPOLATE][attributeIndex]==2)
      {
      //if it is, then change the interpolation weights to 0* 1.0 0*
      int sz = ptIds->GetNumberOfIds();
      double *boolweights = new double[sz];
      boolweights[0] = 1.0; //I assume sz is > 0 to avoid an if
      double max = weights[0];
      int maxpos = 0;
      int j;
      //make the largest weight 1 and all others 0
      for (j = 1; j < sz; j++)
        {
        if (weights[j] > max)
          {
          boolweights[maxpos] = 0.0;
          boolweights[j] = 1.0;
          max = weights[j];
          maxpos = j;
          }
        else
          {
          boolweights[j] = 0.0;
          }
        }      
      fromArray->InterpolateTuple(toId, ptIds, fromPd->Data[i], boolweights);
      delete [] boolweights;
      }
    else
      {
      fromArray->InterpolateTuple(toId, ptIds, fromPd->Data[i], weights);
      }      
    }
}

//--------------------------------------------------------------------------
// Interpolate data from the two points p1,p2 (forming an edge) and an 
// interpolation factor, t, along the edge. The weight ranges from (0,1), 
// with t=0 located at p1. Make sure that the method InterpolateAllocate() 
// has been invoked before using this method.
void vtkDataSetAttributes::InterpolateEdge(vtkDataSetAttributes *fromPd, 
                                           vtkIdType toId, vtkIdType p1,
                                           vtkIdType p2, double t)
{
  int i;
  for(i=this->RequiredArrays.BeginIndex(); !this->RequiredArrays.End(); 
      i=this->RequiredArrays.NextIndex())
    {
    vtkAbstractArray* fromArray = fromPd->Data[i];
    vtkAbstractArray* toArray = this->Data[this->TargetIndices[i]];

    //check if the destination array needs nearest neighbor interpolation
    int attributeIndex = this->IsArrayAnAttribute(this->TargetIndices[i]);
    if (attributeIndex>0
        && 
        this->CopyAttributeFlags[INTERPOLATE][attributeIndex]==2)
      {
      double bt = (t < 0.5) ? 0.0 : 1.0;
      toArray->InterpolateTuple(toId, p1, fromArray, p2, fromArray, bt);      
      }
    else
      {
      toArray->InterpolateTuple(toId, p1, fromArray, p2, fromArray, t);
      }     
    }
}

//--------------------------------------------------------------------------
// Interpolate data from the two points p1,p2 (forming an edge) and an 
// interpolation factor, t, along the edge. The weight ranges from (0,1), 
// with t=0 located at p1. Make sure that the method InterpolateAllocate() 
// has been invoked before using this method.
void vtkDataSetAttributes::InterpolateTime(vtkDataSetAttributes *from1,
                                           vtkDataSetAttributes *from2,
                                           vtkIdType id, double t)
{
  int attributeType;
  for(attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
    {
    // If this attribute is to be copied
    if (this->CopyAttributeFlags[INTERPOLATE][attributeType])
      {
      if (from1->GetAttribute(attributeType) && 
          from2->GetAttribute(attributeType))
        {
        vtkAbstractArray* toArray = this->GetAttribute(attributeType);
        //check if the destination array needs nearest neighbor interpolation
        if (this->CopyAttributeFlags[INTERPOLATE][attributeType]==2)
          {
          double bt = (t < 0.5) ? 0.0 : 1.0;
          toArray->InterpolateTuple(id, id, from1->GetAttribute(attributeType),
                                    id, from2->GetAttribute(attributeType), bt);
          }
        else
          {
          toArray->InterpolateTuple(id, id, from1->GetAttribute(attributeType),
                                    id, from2->GetAttribute(attributeType), t);
          }
        }
      }
    }
}

//--------------------------------------------------------------------------
// Copy a tuple of data from one data array to another. This method (and
// following ones) assume that the fromData and toData objects are of the
// same type, and have the same number of components. This is true if you
// invoke CopyAllocate() or InterpolateAllocate().
void vtkDataSetAttributes::CopyTuple(vtkAbstractArray *fromData,
                                     vtkAbstractArray *toData, vtkIdType fromId,
                                     vtkIdType toId)
{
  toData->InsertTuple(toId, fromId, fromData);
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetScalars(vtkDataArray* da) 
{ 
  return this->SetAttribute(da, SCALARS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveScalars(const char* name)
{ 
  return this->SetActiveAttribute(name, SCALARS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveAttribute(const char* name,
                                             int attributeType)
{
  int index; 
  this->GetArray(name, index);
  return this->SetActiveAttribute(index, attributeType);
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetScalars() 
{ 
  return this->GetAttribute(SCALARS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetVectors(vtkDataArray* da) 
{ 
return this->SetAttribute(da, VECTORS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveVectors(const char* name)
{ 
  return this->SetActiveAttribute(name, VECTORS); 
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetVectors() 
{ 
  return this->GetAttribute(VECTORS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetNormals(vtkDataArray* da) 
{ 
  return this->SetAttribute(da, NORMALS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveNormals(const char* name)
{ 
  return this->SetActiveAttribute(name, NORMALS); 
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetNormals() 
{ 
  return this->GetAttribute(NORMALS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetTCoords(vtkDataArray* da) 
{ 
  return this->SetAttribute(da, TCOORDS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveTCoords(const char* name)
{ 
  return this->SetActiveAttribute(name, TCOORDS); 
}
//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetTCoords() 
{ 
  return this->GetAttribute(TCOORDS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetTensors(vtkDataArray* da) 
{ 
  return this->SetAttribute(da, TENSORS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveTensors(const char* name)
{ 
  return this->SetActiveAttribute(name, TENSORS); 
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetTensors() 
{ 
  return this->GetAttribute(TENSORS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetGlobalIds(vtkDataArray* da) 
{ 
  return this->SetAttribute(da, GLOBALIDS); 
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveGlobalIds(const char* name)
{ 
  return this->SetActiveAttribute(name, GLOBALIDS); 
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetGlobalIds() 
{ 
  return this->GetAttribute(GLOBALIDS); 
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetScalars(const char* name)
{
  if (name == NULL || name[0] == '\0')
    {
    return this->GetScalars();
    }
  return this->GetArray(name);
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetVectors(const char* name)
{
  if (name == NULL || name[0] == '\0')
    {
    return this->GetVectors();
    }
  return this->GetArray(name);
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetNormals(const char* name)
{
  if (name == NULL || name[0] == '\0')
    {
    return this->GetNormals();
    }
  return this->GetArray(name);
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetTCoords(const char* name)
{
  if (name == NULL || name[0] == '\0')
    {
    return this->GetTCoords();
    }
  return this->GetArray(name);
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetTensors(const char* name)
{
  if (name == NULL || name[0] == '\0')
    {
    return this->GetTensors();
    }
  return this->GetArray(name);
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetGlobalIds(const char* name)
{
  if (name == NULL || name[0] == '\0')
    {
    return this->GetGlobalIds();
    }
  return this->GetArray(name);
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::SetActiveAttribute(int index, int attributeType)
{
  if ( (index >= 0) && (index < this->GetNumberOfArrays()))
    {
    vtkDataArray* darray = vtkDataArray::SafeDownCast(
      this->Data[index]);
    if (!darray)
      {
      vtkWarningMacro("Can not set attribute " 
        << vtkDataSetAttributes::AttributeNames[attributeType]
        << ".Only vtkDataArray subclasses can be set as active attributes.");
      return -1;
      }

    if (!this->CheckNumberOfComponents(darray, attributeType))
      {
      vtkWarningMacro("Can not set attribute " 
                      << vtkDataSetAttributes::AttributeNames[attributeType]
                      << ". Incorrect number of components.");
      return -1;
      }
    this->AttributeIndices[attributeType] = index;
    this->Modified();
    return index;
    }
  else if (index == -1)
    {
    this->AttributeIndices[attributeType] = index;
    this->Modified();
    }

  return -1;
}

//--------------------------------------------------------------------------
const int vtkDataSetAttributes
::NumberOfAttributeComponents[vtkDataSetAttributes::NUM_ATTRIBUTES] =
{ 0,
  3,
  3,
  3,
  9,
  0};

//--------------------------------------------------------------------------
// Scalars set to NOLIMIT 
const int vtkDataSetAttributes
::AttributeLimits[vtkDataSetAttributes::NUM_ATTRIBUTES] =
{ NOLIMIT,
  EXACT,
  EXACT,
  MAX,
  EXACT,
  NOLIMIT};

//--------------------------------------------------------------------------
int vtkDataSetAttributes::CheckNumberOfComponents(vtkDataArray* da,
                                                  int attributeType)
{
  int numComp = da->GetNumberOfComponents();
  
  if ( vtkDataSetAttributes::AttributeLimits[attributeType] == MAX )
    {
    if ( numComp > 
         vtkDataSetAttributes::NumberOfAttributeComponents[attributeType] )
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
  else if ( vtkDataSetAttributes::AttributeLimits[attributeType] == EXACT )
    {
    if ( numComp != 
         vtkDataSetAttributes::NumberOfAttributeComponents[attributeType] )
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
  else if ( vtkDataSetAttributes::AttributeLimits[attributeType] == NOLIMIT )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//--------------------------------------------------------------------------
vtkDataArray* vtkDataSetAttributes::GetAttribute(int attributeType)
{
  int index = this->AttributeIndices[attributeType];
  if (index == -1)
    {
    return 0;
    }
  else
    {
    return vtkDataArray::SafeDownCast(this->Data[index]);
    }
}

//--------------------------------------------------------------------------
// This method lets the user add an array and make it the current
// scalars, vectors etc... (this is determined by the attribute type
// which is an enum defined vtkDataSetAttributes)
int vtkDataSetAttributes::SetAttribute(vtkDataArray* da, int attributeType)
{
  if (da && !this->CheckNumberOfComponents(da, attributeType))
    {
    vtkWarningMacro("Can not set attribute " 
                    << vtkDataSetAttributes::AttributeNames[attributeType]
                    << ". Incorrect number of components.");
    return -1;
    }

  int currentAttribute = this->AttributeIndices[attributeType];

  // If there is an existing attribute, replace it
  if ( (currentAttribute >= 0) && 
       (currentAttribute < this->GetNumberOfArrays()) )
    {
    if (this->GetArray(currentAttribute) == da)
      {
      return currentAttribute;
      }
    this->RemoveArray(currentAttribute);
    }

  if (da)
    {
    // Add the array
    currentAttribute = this->AddArray(da);
    this->AttributeIndices[attributeType] = currentAttribute;
    }
  else
    {
    this->AttributeIndices[attributeType] = -1; //attribute of this type doesn't exist
    }
  this->Modified();
  return this->AttributeIndices[attributeType];
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  // Print the copy flags
  int i;
  os << indent << "Copy Tuple Flags: ( ";
  for (i=0; i<NUM_ATTRIBUTES; i++)
    {
    os << this->CopyAttributeFlags[COPYTUPLE][i] << " ";
    }
  os << ")" << endl;
  os << indent << "Interpolate Flags: ( ";
  for (i=0; i<NUM_ATTRIBUTES; i++)
    {
    os << this->CopyAttributeFlags[INTERPOLATE][i] << " ";
    }
  os << ")" << endl;
  os << indent << "Pass Through Flags: ( ";
  for (i=0; i<NUM_ATTRIBUTES; i++)
    {
    os << this->CopyAttributeFlags[PASSDATA][i] << " ";
    }
  os << ")" << endl;
  
  // Now print the various attributes
  vtkDataArray* da;
  int attributeType;
  for (attributeType=0; attributeType<NUM_ATTRIBUTES; attributeType++)
    {
    os << indent << vtkDataSetAttributes::AttributeNames[attributeType]
       << ": ";
    if ( (da=this->GetAttribute(attributeType)) )
      {
      os << endl;
      da->PrintSelf(os, indent.GetNextIndent());
      }
    else
      {
      os << "(none)" << endl;
      }
    }

}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::GetAttributeIndices(int* indexArray)
{
  int i;
  for(i=0; i<NUM_ATTRIBUTES; i++)
    {
    indexArray[i] = this->AttributeIndices[i];
    }
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::IsArrayAnAttribute(int idx)
{
  int i;
  for (i=0; i<NUM_ATTRIBUTES; i++)
    {
    if ( idx == this->AttributeIndices[i] )
      {
      return i;
      }
    }
  return -1;
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyAttribute (int index, int value, int ctype) 
{ 
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    { 
    int t;
    for (t = COPYTUPLE; t < vtkDataSetAttributes::ALLCOPY; t++)
      {
      if (this->CopyAttributeFlags[t][ index ] != value) 
        { 
        this->CopyAttributeFlags[t][ index ] = value; 
        this->Modified(); 
        } 
      }
    }
  else
    {
    if (this->CopyAttributeFlags[ctype][ index ] != value) 
      { 
      this->CopyAttributeFlags[ctype][ index ] = value; 
      this->Modified(); 
      } 
    }
} 

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyScalars(int i, int ctype) 
{ 
  this->SetCopyAttribute(SCALARS, i, ctype); 
}
//--------------------------------------------------------------------------
int vtkDataSetAttributes::GetCopyScalars(int ctype) { 
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    {
    return 
      this->CopyAttributeFlags[COPYTUPLE][SCALARS] && 
      this->CopyAttributeFlags[INTERPOLATE][SCALARS] && 
      this->CopyAttributeFlags[PASSDATA][SCALARS];
    }
  else
    {
    return 
      this->CopyAttributeFlags[ctype][SCALARS];
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyVectors(int i, int ctype) 
{ 
  this->SetCopyAttribute(VECTORS, i, ctype); 
}
//--------------------------------------------------------------------------
int vtkDataSetAttributes::GetCopyVectors(int ctype) 
{  
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    {
    return 
      this->CopyAttributeFlags[COPYTUPLE][VECTORS] && 
      this->CopyAttributeFlags[INTERPOLATE][VECTORS] && 
      this->CopyAttributeFlags[PASSDATA][VECTORS];
    }
  else
    {
    return 
      this->CopyAttributeFlags[ctype][VECTORS];
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyNormals(int i, int ctype) 
{ 
  this->SetCopyAttribute(NORMALS, i, ctype); 
}
//--------------------------------------------------------------------------
int vtkDataSetAttributes::GetCopyNormals(int ctype) 
{  
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    {
    return 
      this->CopyAttributeFlags[COPYTUPLE][NORMALS] && 
      this->CopyAttributeFlags[INTERPOLATE][NORMALS] && 
      this->CopyAttributeFlags[PASSDATA][NORMALS];
    }
  else
    {
    return 
      this->CopyAttributeFlags[ctype][NORMALS];
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyTCoords(int i, int ctype) 
{ 
  this->SetCopyAttribute(TCOORDS, i, ctype); 
}
//--------------------------------------------------------------------------
int vtkDataSetAttributes::GetCopyTCoords(int ctype) 
{  
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    {
    return 
      this->CopyAttributeFlags[COPYTUPLE][TCOORDS] && 
      this->CopyAttributeFlags[INTERPOLATE][TCOORDS] && 
      this->CopyAttributeFlags[PASSDATA][TCOORDS];
    }
  else
    {
    return 
      this->CopyAttributeFlags[ctype][TCOORDS];
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyTensors(int i, int ctype) 
{ 
  this->SetCopyAttribute(TENSORS, i, ctype); 
}
//--------------------------------------------------------------------------
int vtkDataSetAttributes::GetCopyTensors(int ctype) 
{ 
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    {
    return 
      this->CopyAttributeFlags[COPYTUPLE][TENSORS] && 
      this->CopyAttributeFlags[INTERPOLATE][TENSORS] && 
      this->CopyAttributeFlags[PASSDATA][TENSORS];
    }
  else
    {
    return 
      this->CopyAttributeFlags[ctype][TENSORS];
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::SetCopyGlobalIds(int i, int ctype) 
{ 
  this->SetCopyAttribute(GLOBALIDS, i, ctype); 
}
//--------------------------------------------------------------------------
int vtkDataSetAttributes::GetCopyGlobalIds(int ctype) 
{ 
  if (ctype == vtkDataSetAttributes::ALLCOPY)
    {
    return 
      this->CopyAttributeFlags[COPYTUPLE][GLOBALIDS] && 
      this->CopyAttributeFlags[INTERPOLATE][GLOBALIDS] && 
      this->CopyAttributeFlags[PASSDATA][GLOBALIDS];
    }
  else
    {
    return 
      this->CopyAttributeFlags[ctype][GLOBALIDS];
    }
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::RemoveArray(const char *name)
{
  int i;
  this->GetArray(name, i);
  this->RemoveArray(i);
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::CopyAllocate(vtkDataSetAttributes::FieldList& list, 
                                        vtkIdType sze, vtkIdType ext)
{
  vtkAbstractArray* newAA=0;
  vtkDataArray* newDA=0;
  int i;

  // Allocate attributes if any
  for (i=0; i < list.NumberOfFields; i++)
    {
    if ( list.FieldIndices[i] >= 0 )
      {
      newAA = vtkAbstractArray::CreateArray(list.FieldTypes[i]);
      newAA->SetName(list.Fields[i]);
      newAA->SetNumberOfComponents(list.FieldComponents[i]);

      if ( sze > 0 )
        {
        newAA->Allocate(sze,ext);
        }
      else
        {
        newAA->Allocate(list.NumberOfTuples,ext);
        }
      if ( (newDA = vtkDataArray::SafeDownCast(newAA)) )
        {
        newDA->SetLookupTable(list.LUT[i]);
        }

      // If attribute data, do something extra
      if ( i < NUM_ATTRIBUTES )
        {
        // since attributes can only be DataArray, newDA must be non-null.
        if ( this->CopyAttributeFlags[COPYTUPLE][i] && newDA)
          {
          list.FieldIndices[i] = this->AddArray(newDA);
          this->SetActiveAttribute(list.FieldIndices[i], i);
          }
        else
          {
          list.FieldIndices[i] = -1;
          }
        }
      else //check if this field is to be copied
        {
        if ( (this->GetFlag(list.Fields[i]) != 0) &&
             !(this->DoCopyAllOff && (this->GetFlag(list.Fields[i]) != 1)) )
          {
          list.FieldIndices[i] = this->AddArray(newDA);
          }
        else
          {
          list.FieldIndices[i] = -1;
          }
        }

      newAA->Delete(); //okay, reference counting
      }//data array defined
    }
}

//--------------------------------------------------------------------------
// Description:
// A special form of CopyData() to be used with FieldLists. Use it when you are
// copying data from a set of vtkDataSetAttributes. Make sure that you have
// called the special form of CopyAllocate that accepts FieldLists.
void vtkDataSetAttributes::CopyData(vtkDataSetAttributes::FieldList& list, 
                                    vtkDataSetAttributes* fromDSA,
                                    int idx, vtkIdType fromId, vtkIdType toId)
{
  vtkDataArray *fromDA;
  vtkDataArray *toDA;
  
  int i;
  for (i=0; i < list.NumberOfFields; i++)
    {
    if ( list.FieldIndices[i] >= 0 )
      {
      toDA = this->GetArray(list.FieldIndices[i]);
      fromDA = fromDSA->GetArray(list.DSAIndices[idx][i]);
      this->CopyTuple(fromDA, toDA, fromId, toId);
      }
    }
}

// FieldList support ---------------------------------------------------------
// To perform intersection of attribute data, use IntializeFieldList() to grab
// an initial vtkDataSetAttributes. Then use IntersectFieldList() to add (and 
// intersect) additional vtkDataSetAttributes.
void vtkDataSetAttributes::FieldList::InitializeFieldList(vtkDataSetAttributes* dsa)
{
  int i, idx;

  this->ClearFields();

  // Allocate space for the arrays plus five attributes
  this->NumberOfFields = dsa->GetNumberOfArrays() + NUM_ATTRIBUTES;
  this->Fields = new char*[this->NumberOfFields];
  this->FieldTypes = new int [this->NumberOfFields];
  this->FieldComponents = new int [this->NumberOfFields];
  this->FieldIndices = new int [this->NumberOfFields];
  this->LUT = new vtkLookupTable* [this->NumberOfFields];
  for(i=0; i < this->NumberOfFields; i++)
    {
    this->Fields[i] = 0;
    this->FieldTypes[i] = -1;
    this->FieldComponents[i] = 0;
    this->FieldIndices[i] = -1;
    }
  this->CurrentInput = 0;
  this->NumberOfTuples = 0;
  
  //there may be no data hence dsa->Data
  for(i=0; dsa->Data && i < dsa->GetNumberOfArrays(); i++) 
    {
    if ( (idx=dsa->IsArrayAnAttribute(i)) >= 0 ) //it's an attribute
      {
      this->FieldIndices[idx] = idx;
      this->SetField(idx, dsa->Data[i]);
      }
    else
      {
      this->FieldIndices[NUM_ATTRIBUTES+i] = i;
      this->SetField(NUM_ATTRIBUTES+i, dsa->Data[i]);
      }
    }
  
  // The first dataset is added to the field list
  this->IntersectFieldList(dsa);
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::FieldList::IntersectFieldList(vtkDataSetAttributes* dsa)
{
  int i;
  vtkDataArray *da;
  vtkAbstractArray* aa;
  
  // Initialize the indices for this dataset
  this->DSAIndices[this->CurrentInput] = new int [this->NumberOfFields];
  for (i=0; i < this->NumberOfFields; i++)
    {
    this->DSAIndices[this->CurrentInput][i]= -1;
    }

  // Keep a running total of the number of tuples...might be useful
  // for later allocation.
  if ( (da=dsa->GetArray(0)) ) 
    {
    this->NumberOfTuples += da->GetNumberOfTuples();
    }
  
  // Intersect the attributes
  int attributeIndices[NUM_ATTRIBUTES];
  dsa->GetAttributeIndices(attributeIndices);
  for(i=0; i < NUM_ATTRIBUTES; i++)
    {
    if ( this->FieldIndices[i] >= 0 )
      {
      da = dsa->GetAttribute(i);
      if ((da) && (da->GetDataType() == this->FieldTypes[i]) && 
          (da->GetNumberOfComponents() == this->FieldComponents[i]))
        {
        this->DSAIndices[this->CurrentInput][i] = attributeIndices[i];
        }
      else
        {
        this->FieldIndices[i] = -1; //Attribute not present
        }
      }
    }
  // Intersect the fields
  int index;
  for(i=NUM_ATTRIBUTES; i < this->NumberOfFields; i++)
    {
    if (this->FieldIndices[i] >= 0)
      {
      aa = dsa->GetAbstractArray(this->Fields[i], index);
      if ((aa) && (aa->GetDataType() == this->FieldTypes[i]) &&
          (aa->GetNumberOfComponents() == this->FieldComponents[i]))
        {
        this->DSAIndices[this->CurrentInput][i] = index;
        }
      else
        {
        this->FieldIndices[i] = -1; //Field not present
        }
      }
    }

    
  this->CurrentInput++;
}

//--------------------------------------------------------------------------
int vtkDataSetAttributes::FieldList::IsAttributePresent(int attrType)
{
  return this->FieldIndices[attrType];
}


//--------------------------------------------------------------------------
vtkDataSetAttributes::FieldList::FieldList(int numInputs)
{
  this->Fields = 0;
  this->FieldTypes = 0;
  this->FieldComponents = 0;
  this->FieldIndices = 0;
  this->NumberOfFields = 0;
  this->LUT = 0;
  this->NumberOfDSAIndices = numInputs;
  this->DSAIndices = new int*[numInputs];
  int i;
  for (i=0; i<numInputs; i++)
    {
    this->DSAIndices[i] = 0;
    }
}

//--------------------------------------------------------------------------
vtkDataSetAttributes::FieldList::~FieldList()
{
  this->ClearFields();
  delete [] this->DSAIndices;
  this->DSAIndices = 0;
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::FieldList::ClearFields()
{
  int i;
  if ( this->Fields )
    {
    for (i=0; i<this->NumberOfFields; i++)
      {
      delete [] this->Fields[i];
      this->Fields[i] = 0;
      }
    }
  if ( this->DSAIndices )
    {
    for (i=0; i<this->NumberOfDSAIndices; i++)
      {
      delete[] this->DSAIndices[i];
      this->DSAIndices[i] = 0;
      }
    }
  delete [] this->LUT;
  this->LUT = 0;
  delete [] this->Fields;
  this->Fields = 0;
  delete [] this->FieldTypes;
  this->FieldTypes = 0;
  delete [] this->FieldComponents;
  this->FieldComponents = 0;
  delete [] this->FieldIndices;
  this->FieldIndices = 0;
  
  this->NumberOfFields = 0;
  this->CurrentInput = 0;
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::FieldList::SetField(int index, 
  vtkAbstractArray *aa)
{
  const char* name=aa->GetName();
  int dataType=aa->GetDataType();
  vtkLookupTable *lut = 0;
  
  if (vtkDataArray::SafeDownCast(aa))
    {
    lut=vtkDataArray::SafeDownCast(aa)->GetLookupTable();
    }
  
  if ( this->Fields[index] )
    {
    delete [] this->Fields[index];
    this->Fields[index] = 0;
    }
  
  this->FieldTypes[index] = dataType;
  this->FieldComponents[index] = aa->GetNumberOfComponents();
  this->LUT[index] = lut;
  if (name)
    {
    int len = static_cast<int>(strlen(name));
    if (len > 0)
      {
      this->Fields[index] = new char[len+1];
      strcpy(this->Fields[index], name);
      }
    }
  else
    {
    this->Fields[index] = 0;
    }
    
}

//--------------------------------------------------------------------------
void vtkDataSetAttributes::FieldList::RemoveField(const char *name)
{
  if ( !name )
    {
    return;
    }

  int i;
  for (i=NUM_ATTRIBUTES; i < this->NumberOfFields; i++)
    {
    if ( this->Fields[i] && !strcmp(this->Fields[i],name) )
      {
      delete [] this->Fields[i];
      this->Fields[i] = 0;
      this->FieldIndices[i] = -1;
      return;
      }
    }
}


//--------------------------------------------------------------------------
const char* vtkDataSetAttributes::GetAttributeTypeAsString(int attributeType)
{
  if (attributeType < 0 || attributeType >= NUM_ATTRIBUTES)
    {
    vtkGenericWarningMacro("Bad attribute type.");
    return NULL;
    }
  return vtkDataSetAttributes::AttributeNames[attributeType];
}

//--------------------------------------------------------------------------
const char* vtkDataSetAttributes::GetLongAttributeTypeAsString(int attributeType)
{
  if (attributeType < 0 || attributeType >= NUM_ATTRIBUTES)
    {
    vtkGenericWarningMacro("Bad attribute type.");
    return NULL;
    }
  return vtkDataSetAttributes::LongAttributeNames[attributeType];
}

