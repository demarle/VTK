/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkFloatNormals.h
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
// .NAME vtkFloatNormals - (obsolete)floating point representation of 3D normals
// .SECTION Description
// vtkFloatNormals is a concrete implementation of vtkNormals. Normals are
// represented using float values.

#ifndef __vtkFloatNormals_h
#define __vtkFloatNormals_h

#include "vtkNormals.h"
#include "vtkFloatArray.h"

class VTK_EXPORT vtkFloatNormals : public vtkNormals
{
public:
  static vtkFloatNormals *New() {return new vtkFloatNormals;};
  
  // Description:
  // Set the data type for this object.
  void SetDataType(int dataType);

  // Description:
  // Set the data for this object. Only accepts VTK_FLOAT type.
  void SetData(vtkDataArray *);

  // Description:
  // Get pointer to array of data starting at data position "id".
  float *GetPointer(const int id);

  // Description:
  // Get pointer to data array. Useful for direct writes of data. MaxId is
  // bumped by number (and memory allocated if necessary). Id is the
  // location you wish to write into; number is the number of normals to
  // write.
  float *WritePointer(const int id, const int number);
  
protected:
  vtkFloatNormals():vtkNormals(VTK_FLOAT) {};
  ~vtkFloatNormals() {};
  vtkFloatNormals(const vtkFloatNormals&) {};
  void operator=(const vtkFloatNormals&) {};

};

inline float *vtkFloatNormals::GetPointer(const int id)
{
  return ((vtkFloatArray *)this->Data)->GetPointer(3*id);
} 

inline float *vtkFloatNormals::WritePointer(const int id, const int number)
{
  return ((vtkFloatArray *)this->Data)->WritePointer(3*id,3*number);
}

inline void vtkFloatNormals::SetData(vtkDataArray *data)
{
  if ( data->GetDataType() != VTK_FLOAT )
    {
    vtkErrorMacro(<<"Float normals only accepts float data type");
    return;
    }

  vtkNormals::SetData(data);
}

inline void vtkFloatNormals::SetDataType(int type)
{
  if ( type != VTK_FLOAT )
    {
    vtkErrorMacro(<<"Float normals only accepts float data type");
    return;
    }

  vtkNormals::SetDataType(type);
}

#endif

