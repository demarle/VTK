/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVoidArray.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkVoidArray - dynamic, self-adjusting array of void* pointers
// .SECTION Description
// vtkVoidArray is an array of pointers to void. It provides methods
// for insertion and retrieval of these pointers values, and will 
// automatically resize itself to hold new data.

#ifndef __vtkVoidArray_h
#define __vtkVoidArray_h

#include "vtkDataArray.h"

class VTK_EXPORT vtkVoidArray : public vtkDataArray
{
public:
  static vtkVoidArray *New();

  vtkTypeMacro(vtkVoidArray,vtkDataArray);
  void PrintSelf(vtkOstream& os, vtkIndent indent);

  // Description:
  // Allocate memory for this array. Delete old storage only if necessary.
  int Allocate(const int sz, const int ext=1000);

  // Description:
  // Release storage and reset array to initial state.
  void Initialize();

  // Description:
  // Create a similar type object
  vtkDataArray *MakeObject() {return vtkVoidArray::New();};

  // Description:
  // Get the data type.
  int GetDataType() {return VTK_VOID;};
  
  // Description:
  // Set the number of n-tuples in the array.
  void SetNumberOfTuples(const int number);

  // Description:
  // Get a pointer to a tuple at the ith location.
  float *GetTuple(const int i);

  // Description:
  // Copy the tuple value into a user-provided array.
  void GetTuple(const int i, float * tuple);
  void GetTuple(const int i, double * tuple);

  // Description:
  // Set the tuple value at the ith location in the array.
  void SetTuple(const int i, const float * tuple);
  void SetTuple(const int i, const double * tuple);

  // Description:
  // Insert (memory allocation performed) the tuple into the ith location
  // in the array.
  void InsertTuple(const int i, const float * tuple);
  void InsertTuple(const int i, const double * tuple);

  // Description:
  // Insert (memory allocation performed) the tuple onto the end of the array.
  int InsertNextTuple(const float * tuple);
  int InsertNextTuple(const double * tuple);

  // Description:
  // Resize object to just fit data requirement. Reclaims extra memory.
  void Squeeze() {this->Resize (this->MaxId+1);};

  // Description:
  // Get the data at a particular index.
  void* GetValue(const int id) {return this->Array[id];};

  // Description:
  // Specify the number of values for this object to hold. Does an
  // allocation as well as setting the MaxId ivar. Used in conjunction with
  // SetValue() method for fast insertion.
  void SetNumberOfValues(const int number);

  // Description:
  // Set the data at a particular index. Does not do range checking. Make sure
  // you use the method SetNumberOfValues() before inserting data.
  void SetValue(const int id, void *value);

  // Description:
  // Insert data at a specified position in the array.
  void InsertValue(const int id, void* p);

  // Description:
  // Insert data at the end of the array. Return its location in the array.
  int InsertNextValue(void* v);

  // Description:
  // Get the address of a particular data index. Performs no checks
  // to verify that the memory has been allocated etc.
  void** GetPointer(const int id) {return this->Array + id;}
  void *GetVoidPointer(const int id) {return this->GetPointer(id);};

  // Description:
  // Get the address of a particular data index. Make sure data is allocated
  // for the number of items requested. Set MaxId according to the number of
  // data values requested.
  void** WritePointer(const int id, const int number);
  
  // Description:
  // Deep copy of another void array.
  void DeepCopy(vtkDataArray *da);
  
  // Description:
  // For legacy compatibility. Do not use.
  void DeepCopy(vtkDataArray &da) {this->DeepCopy(&da);}
  

protected:
  vtkVoidArray();
  ~vtkVoidArray();
  vtkVoidArray(const vtkVoidArray&) {};
  void operator=(const vtkVoidArray&) {};

  void** Array;  // pointer to data
  void** Resize(const int sz);  // function to resize data

  int TupleSize; //used for data conversion
  float *Tuple;
};


inline void vtkVoidArray::SetNumberOfValues(const int number) 
{
  this->Allocate(number);
  this->MaxId = number - 1;
}

inline void vtkVoidArray::SetValue(const int id, void *value) 
{
  this->Array[id] = value;
}

inline void** vtkVoidArray::WritePointer(const int id, const int number) 
{
  int newSize=id+number;
  if ( newSize > this->Size )
    {
    this->Resize(newSize);
    }
  if ( (--newSize) > this->MaxId )
    {
    this->MaxId = newSize;
    }
  return this->Array + id;
}

inline void vtkVoidArray::InsertValue(const int id, void* p)
{
  if ( id >= this->Size )
    {
    this->Resize(id+1);
    }
  this->Array[id] = p;
  if ( id > this->MaxId )
    {
    this->MaxId = id;
    }
}

inline int vtkVoidArray::InsertNextValue(void* p)
{
  this->InsertValue (++this->MaxId,p);
  return this->MaxId;
}


#endif
