/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageImport.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder,ill Lorensen.

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
#include "vtkImageImport.h"

//----------------------------------------------------------------------------
vtkImageImport::vtkImageImport()
{
  this->Region = vtkImageRegion::New();
}

//----------------------------------------------------------------------------
vtkImageImport::~vtkImageImport()
{
  this->Region->Delete();
}

//----------------------------------------------------------------------------
void vtkImageImport::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageSource::PrintSelf(os,indent);
  os << indent << "ScalarType: " 
     << vtkImageScalarTypeNameMacro(this->ScalarType) << "\n";
  os << indent << "Region: (" << this->Region << ")\n";
  this->Region->PrintSelf(os,indent.GetNextIndent());
  
}



//----------------------------------------------------------------------------
void vtkImageImport::SetAxes(int dim, int *axes)
{
  this->Region->ReleaseData();
  this->Region->SetAxes(dim, axes);
  // Keep a local copy for macros
  this->Region->GetAxes(VTK_IMAGE_DIMENSIONS, this->Axes);
}

//----------------------------------------------------------------------------
void vtkImageImport::SetExtent(int dim, int *extent)
{
  this->Region->ReleaseData();
  this->Region->SetExtent(dim, extent);
  // Keep a local copy for macros
  this->Region->GetExtent(VTK_IMAGE_DIMENSIONS, this->Extent);
}

//----------------------------------------------------------------------------
void vtkImageImport::SetPointer(void *ptr)
{
  int length;
  void *regionPtr;
  vtkImageData *data;
  
  
  this->Region->ReleaseData();
  // Compute the length of the memory segment.
  switch (this->ScalarType)
    {
    case VTK_FLOAT:
      length = sizeof(float);
      break;
    case VTK_INT:
      length = sizeof(int);
      break;
    case VTK_SHORT:
      length = sizeof(short);
      break;
    case VTK_UNSIGNED_SHORT:
      length = sizeof(unsigned short);
      break;
    case VTK_UNSIGNED_CHAR:
      length = sizeof(unsigned char);
      break;
    default:
      vtkErrorMacro("Cannot determine ScalarType");
      return;
    }
  length *= this->Region->GetVolume();
  
  
  // make sure the underlying data object is set correctly
  data = vtkImageData::New();
  data->SetScalarType(this->ScalarType);
  data->SetAxes(VTK_IMAGE_DIMENSIONS, this->Axes);
  data->SetExtent(VTK_IMAGE_DIMENSIONS, this->Extent);

  this->Region->SetScalarType(this->ScalarType);
  this->Region->SetAxes(VTK_IMAGE_DIMENSIONS, this->Axes);
  this->Region->SetExtent(VTK_IMAGE_DIMENSIONS, this->Extent);
  this->Region->SetData(data);
  
  // copy the memory
  vtkDebugMacro("SetPointer: Copying " << length << " bytes");
  regionPtr = this->Region->GetScalarPointer();
  memcpy(regionPtr, ptr, length);
  
  // data is reference counted
  //data->Delete();
  
}

