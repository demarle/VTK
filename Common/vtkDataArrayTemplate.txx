/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataArrayTemplate.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtkDataArrayTemplate_txx
#define __vtkDataArrayTemplate_txx

#include "vtkDataArrayTemplate.h"

// We do not provide a definition for the copy constructor or
// operator=.  Block the warning.
#ifdef _MSC_VER
# pragma warning (disable: 4661)
#endif

//----------------------------------------------------------------------------
template <class T>
vtkDataArrayTemplate<T>::vtkDataArrayTemplate(vtkIdType numComp):
  vtkDataArray(numComp)
{
  this->Array = 0;
  this->TupleSize = 3;
  this->Tuple = new float[this->TupleSize]; //used for conversion
  this->SaveUserArray = 0;
}

//----------------------------------------------------------------------------
template <class T>
vtkDataArrayTemplate<T>::~vtkDataArrayTemplate()
{
  if ((this->Array) && (!this->SaveUserArray))
    {
    delete [] this->Array;
    }
  delete [] this->Tuple;
}

//----------------------------------------------------------------------------
// This method lets the user specify data to be held by the array.  The
// array argument is a pointer to the data.  size is the size of
// the array supplied by the user.  Set save to 1 to keep the class
// from deleting the array when it cleans up or reallocates memory.
// The class uses the actual array provided; it does not copy the data
// from the suppled array.
template <class T>
void vtkDataArrayTemplate<T>::SetArray(T* array, vtkIdType size, int save)
{
  if ((this->Array) && (!this->SaveUserArray))
    {
    vtkDebugMacro (<< "Deleting the array...");
    delete [] this->Array;
    }
  else
    {
    vtkDebugMacro (<<"Warning, array not deleted, but will point to new array.");
    }

  vtkDebugMacro(<<"Setting array to: " << array);

  this->Array = array;
  this->Size = size;
  this->MaxId = size-1;
  this->SaveUserArray = save;
}

//----------------------------------------------------------------------------
// Allocate memory for this array. Delete old storage only if necessary.
template <class T>
int vtkDataArrayTemplate<T>::Allocate(const vtkIdType sz, const vtkIdType)
{
  if(sz > this->Size)
    {
    if(this->Array && !this->SaveUserArray)
      {
      delete [] this->Array;
      }

    this->Size = ( sz > 0 ? sz : 1);
    this->Array = new T[this->Size];
    if(!this->Array)
      {
      return 0;
      }
    this->SaveUserArray = 0;
    }

  this->MaxId = -1;

  return 1;
}

//----------------------------------------------------------------------------
// Release storage and reset array to initial state.
template <class T>
void vtkDataArrayTemplate<T>::Initialize()
{
  if(this->Array && !this->SaveUserArray)
    {
    delete [] this->Array;
    }
  this->Array = 0;
  this->Size = 0;
  this->MaxId = -1;
  this->SaveUserArray = 0;
}

//----------------------------------------------------------------------------
// Deep copy of another double array.
template <class T>
void vtkDataArrayTemplate<T>::DeepCopy(vtkDataArray* fa)
{
  // Do nothing on a NULL input.
  if(!fa)
    {
    return;
    }

  // Avoid self-copy.
  if(this == fa)
    {
    return;
    }

  // If data type does not match, do copy with conversion.
  if(fa->GetDataType() != this->GetDataType())
    {
    this->Superclass::DeepCopy(fa);
    return;
    }

  // Free our previous memory.
  if(this->Array && !this->SaveUserArray)
    {
    delete [] this->Array;
    }

  // Copy the given array into new memory.
  this->NumberOfComponents = fa->GetNumberOfComponents();
  this->MaxId = fa->GetMaxId();
  this->Size = fa->GetSize();
  this->SaveUserArray = 0;
  this->Array = new T[this->Size];
  memcpy(this->Array, fa->GetVoidPointer(0), this->Size*sizeof(T));
}

//----------------------------------------------------------------------------
template <class T>
void vtkDataArrayTemplate<T>::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if(this->Array)
    {
    os << indent << "Array: " << this->Array << "\n";
    }
  else
    {
    os << indent << "Array: (null)\n";
    }
}

//----------------------------------------------------------------------------
// Protected function does "reallocate"
template <class T>
T* vtkDataArrayTemplate<T>::ResizeAndExtend(const vtkIdType sz)
{
  T* newArray;
  vtkIdType newSize;

  if(sz > this->Size)
    {
    // Requested size is bigger than current size.  Allocate enough
    // memory to fit the requested size and be more than double the
    // currently allocated memory.
    newSize = this->Size + sz;
    }
  else if (sz == this->Size)
    {
    // Requested size is equal to current size.  Do nothing.
    return this->Array;
    }
  else
    {
    // Requested size is smaller than current size.  Squeeze the
    // memory.
    newSize = sz;
    }

  if(newSize <= 0)
    {
    this->Initialize();
    return 0;
    }

  newArray = new T[newSize];
  if(!newSize)
    {
    vtkErrorMacro(<< "Cannot allocate memory\n");
    return 0;
    }

  if(this->Array)
    {
    memcpy(newArray, this->Array,
           (sz < this->Size ? sz : this->Size) * sizeof(T));
    if(!this->SaveUserArray)
      {
      delete [] this->Array;
      }
    }

  if(newSize < this->Size)
    {
    this->MaxId = newSize-1;
    }
  this->Size = newSize;
  this->Array = newArray;
  this->SaveUserArray = 0;

  return this->Array;
}

//----------------------------------------------------------------------------
template <class T>
void vtkDataArrayTemplate<T>::Resize(const vtkIdType sz)
{
  T* newArray;
  vtkIdType newSize = sz*NumberOfComponents;

  if(newSize == this->Size)
    {
    return;
    }

  if(newSize <= 0)
    {
    this->Initialize();
    return;
    }

  newArray = new T[newSize];
  if(!newArray)
    {
    vtkErrorMacro(<< "Cannot allocate memory\n");
    return;
    }

  if(this->Array)
    {
    memcpy(newArray, this->Array,
           (newSize < this->Size ? newSize : this->Size) * sizeof(T));
    if(!this->SaveUserArray)
      {
      delete[] this->Array;
      }
    }

  if(newSize < this->Size)
    {
    this->MaxId = newSize-1;
    }
  this->Size = newSize;
  this->Array = newArray;
  this->SaveUserArray = 0;
}

//----------------------------------------------------------------------------
// Set the number of n-tuples in the array.
template <class T>
void vtkDataArrayTemplate<T>::SetNumberOfTuples(const vtkIdType number)
{
  this->SetNumberOfValues(number*this->NumberOfComponents);
}

//----------------------------------------------------------------------------
// Get a pointer to a tuple at the ith location. This is a dangerous method
// (it is not thread safe since a pointer is returned).
template <class T>
float* vtkDataArrayTemplate<T>::GetTuple(const vtkIdType i)
{
#if 1
  if(this->TupleSize < this->NumberOfComponents)
    {
    this->TupleSize = this->NumberOfComponents;
    delete [] this->Tuple;
    this->Tuple = new float[this->TupleSize];
    }

  T* t = this->Array + this->NumberOfComponents*i;
  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    this->Tuple[j] = static_cast<float>(t[j]);
    }
  return this->Tuple;
#else
  // Use this version along with purify or valgrind to detect code
  // that saves the pointer returned by GetTuple.
  T* t = this->Array + this->NumberOfComponents*i;
  float* newTuple = new float[this->NumberOfComponents];
  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    newTuple[j] = static_cast<float>(t[j]);
    }
  delete [] this->Tuple;
  this->Tuple = newTuple;
  return this->Tuple;
#endif
}

//----------------------------------------------------------------------------
// Copy the tuple value into a user-provided array.
template <class T>
void vtkDataArrayTemplate<T>::GetTuple(const vtkIdType i, float* tuple)
{
  T* t = this->Array + this->NumberOfComponents*i;
  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    tuple[j] = static_cast<float>(t[j]);
    }
}

template <class T>
void vtkDataArrayTemplate<T>::GetTuple(const vtkIdType i, double* tuple)
{
  T* t = this->Array + this->NumberOfComponents*i;
  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    tuple[j] = static_cast<double>(t[j]);
    }
}

//----------------------------------------------------------------------------
// Set the tuple value at the ith location in the array.
template <class T>
void vtkDataArrayTemplate<T>::SetTuple(const vtkIdType i, const float* tuple)
{
  vtkIdType loc = i * this->NumberOfComponents;
  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    this->Array[loc+j] = static_cast<T>(tuple[j]);
    }
}

template <class T>
void vtkDataArrayTemplate<T>::SetTuple(const vtkIdType i, const double* tuple)
{
  vtkIdType loc = i * this->NumberOfComponents;
  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    this->Array[loc+j] = static_cast<T>(tuple[j]);
    }
}

//----------------------------------------------------------------------------
// Insert (memory allocation performed) the tuple into the ith location
// in the array.
template <class T>
void vtkDataArrayTemplate<T>::InsertTuple(const vtkIdType i,
                                          const float* tuple)
{
  T* t = this->WritePointer(i*this->NumberOfComponents,
                            this->NumberOfComponents);

  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    *t++ = static_cast<T>(*tuple++);
    }
}

template <class T>
void vtkDataArrayTemplate<T>::InsertTuple(const vtkIdType i,
                                          const double* tuple)
{
  T* t = this->WritePointer(i*this->NumberOfComponents,
                            this->NumberOfComponents);

  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    *t++ = static_cast<T>(*tuple++);
    }
}

//----------------------------------------------------------------------------
// Insert (memory allocation performed) the tuple onto the end of the array.
template <class T>
vtkIdType vtkDataArrayTemplate<T>::InsertNextTuple(const float* tuple)
{
  T* t = this->WritePointer(this->MaxId + 1, this->NumberOfComponents);

  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    *t++ = static_cast<T>(*tuple++);
    }

  return this->MaxId / this->NumberOfComponents;
}

template <class T>
vtkIdType vtkDataArrayTemplate<T>::InsertNextTuple(const double* tuple)
{
  T* t = this->WritePointer(this->MaxId + 1,this->NumberOfComponents);

  for(int j=0; j < this->NumberOfComponents; ++j)
    {
    *t++ = static_cast<T>(*tuple++);
    }

  return this->MaxId / this->NumberOfComponents;
}

//----------------------------------------------------------------------------
// Return the data component at the ith tuple and jth component location.
// Note that i<NumberOfTuples and j<NumberOfComponents.
template <class T>
float vtkDataArrayTemplate<T>::GetComponent(vtkIdType i, int j)
{
  return static_cast<float>(this->GetValue(i*this->NumberOfComponents + j));
}

//----------------------------------------------------------------------------
// Set the data component at the ith tuple and jth component location.
// Note that i<NumberOfTuples and j<NumberOfComponents. Make sure enough
// memory has been allocated (use SetNumberOfTuples() and
// SetNumberOfComponents()).
template <class T>
void vtkDataArrayTemplate<T>::SetComponent(vtkIdType i, int j,
                                           float c)
{
  this->SetValue(i*this->NumberOfComponents + j, static_cast<T>(c));
}

//----------------------------------------------------------------------------
template <class T>
void vtkDataArrayTemplate<T>::InsertComponent(vtkIdType i, int j,
                                              float c)
{
  this->InsertValue(i*this->NumberOfComponents + j, static_cast<T>(c));
}

//----------------------------------------------------------------------------
template <class T>
void vtkDataArrayTemplate<T>::SetNumberOfValues(vtkIdType number)
{
  this->Allocate(number);
  this->MaxId = number - 1;
}

//----------------------------------------------------------------------------
template <class T>
T* vtkDataArrayTemplate<T>::WritePointer(vtkIdType id,
                                         vtkIdType number)
{
  vtkIdType newSize=id+number;
  if ( newSize > this->Size )
    {
    this->ResizeAndExtend(newSize);
    }
  if ( (--newSize) > this->MaxId )
    {
    this->MaxId = newSize;
    }
  return this->Array + id;
}

//----------------------------------------------------------------------------
template <class T>
void vtkDataArrayTemplate<T>::InsertValue(vtkIdType id, T f)
{
  if ( id >= this->Size )
    {
    this->ResizeAndExtend(id+1);
    }
  this->Array[id] = f;
  if ( id > this->MaxId )
    {
    this->MaxId = id;
    }
}

//----------------------------------------------------------------------------
template <class T>
vtkIdType vtkDataArrayTemplate<T>::InsertNextValue(T f)
{
  this->InsertValue (++this->MaxId,f);
  return this->MaxId;
}

#endif
