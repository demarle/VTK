/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProp3D.cxx
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
#include <stdlib.h>
#include <math.h>

#include "vtkProp3D.h"

typedef double (*SqMatPtr)[4];

// Construct with the following defaults: origin(0,0,0) 
// position=(0,0,0) and orientation=(0,0,0). No user defined 
// matrix and no texture map.
vtkProp3D::vtkProp3D()
{
  this->Origin[0] = 0.0;
  this->Origin[1] = 0.0;
  this->Origin[2] = 0.0;

  this->Position[0] = 0.0;
  this->Position[1] = 0.0;
  this->Position[2] = 0.0;

  this->Orientation[0] = 0.0;
  this->Orientation[1] = 0.0;
  this->Orientation[2] = 0.0;

  this->Scale[0] = 1.0;
  this->Scale[1] = 1.0;
  this->Scale[2] = 1.0;

  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -1.0;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = 1.0;

  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;

  this->UserMatrix = NULL;
  this->UserTransform = NULL;
  this->Matrix = vtkMatrix4x4::New();
  this->Transform = vtkTransform::New();
}

vtkProp3D::~vtkProp3D()
{
  this->Matrix->Delete();
  this->Transform->Delete();
  if (this->UserMatrix)
    {
    this->UserMatrix->UnRegister(this);
    this->UserMatrix = NULL;
    }
  if (this->UserTransform)
    {
    this->UserTransform->UnRegister(this);
    this->UserTransform = NULL;
    }
}

// Shallow copy of an Prop3D.
void vtkProp3D::ShallowCopy(vtkProp3D *Prop3D)
{
  int i;

  this->vtkProp::ShallowCopy(Prop3D);

  for (i=0; i < 3; i++) 
    {
    this->Origin[i] = Prop3D->Origin[i];
    this->Position[i] = Prop3D->Position[i];
    this->Orientation[i] = Prop3D->Orientation[i];
    this->Center[i] = Prop3D->Center[i];
    this->Scale[i] = Prop3D->Scale[i];
    }
  this->Transform->DeepCopy(Prop3D->Transform);
  
  for (i=0; i < 6; i++)
    {
    this->Bounds[i] = Prop3D->Bounds[i];
    }

  this->SetUserTransform(Prop3D->UserTransform);
  this->SetUserMatrix(Prop3D->UserMatrix);
}

// Incrementally change the position of the Prop3D.
void vtkProp3D::AddPosition (float deltaX,float deltaY,float deltaZ)
{
  float position[3];

  position[0] = this->Position[0] + deltaX;
  position[1] = this->Position[1] + deltaY;
  position[2] = this->Position[2] + deltaZ;
  
  this->SetPosition(position);
}

void vtkProp3D::AddPosition (float deltaPosition[3])
{
  this->AddPosition (deltaPosition[0], deltaPosition[1], deltaPosition[2]);
}

// Sets the orientation of the Prop3D.  Orientation is specified as
// X,Y and Z rotations in that order, but they are performed as
// RotateZ, RotateX, and finally RotateY.
void vtkProp3D::SetOrientation (float x,float y,float z)
{
  if (x == this->Orientation[0] && y == this->Orientation[1] 
      && z == this->Orientation[2])
    {
    return;
    }
    
  // store the coordinates
  this->Orientation[0] = x;
  this->Orientation[1] = y;
  this->Orientation[2] = z;

  vtkDebugMacro(<< " Orientation set to ( " 
                << this->Orientation[0] << ", "
                << this->Orientation[1] << ", " 
                << this->Orientation[2] << ")\n");

  this->Transform->Identity();
  this->Transform->PreMultiply ();
  this->Transform->RotateZ(this->Orientation[2]);
  this->Transform->RotateX(this->Orientation[0]);
  this->Transform->RotateY(this->Orientation[1]);

  this->Modified();
}
void vtkProp3D::SetOrientation(float a[3])
{
  this->SetOrientation(a[0],a[1],a[2]);
}

// Returns the orientation of the Prop3D as s vector of X,Y and Z rotation.
// The ordering in which these rotations must be done to generate the 
// same matrix is RotateZ, RotateX, and finally RotateY. See also 
// SetOrientation.
float *vtkProp3D::GetOrientation ()
{
  float   *orientation;

  // return the orientation of the transformation matrix
  orientation = this->Transform->GetOrientation();
  this->Orientation[0] = orientation[0];
  this->Orientation[1] = orientation[1];
  this->Orientation[2] = orientation[2];

  vtkDebugMacro(<< " Returning Orientation of ( " <<  this->Orientation[0] 
  << ", " << this->Orientation[1] << ", " << this->Orientation[2] << ")\n");

  return this->Orientation;
} // vtkProp3D::Getorientation 

// Returns the WXYZ orientation of the Prop3D. 
float *vtkProp3D::GetOrientationWXYZ()
{
  return this->Transform->GetOrientationWXYZ();
}

// Add to the current orientation. See SetOrientation and GetOrientation for 
// more details. This basically does a GetOrientation, adds the passed in
// arguments, and then calls SetOrientation.
void vtkProp3D::AddOrientation (float a1,float a2,float a3)
{
  float *orient;

  orient = this->GetOrientation();
  this->SetOrientation(orient[0] + a1,
		       orient[1] + a2,
		       orient[2] + a3);
} 
void vtkProp3D::AddOrientation(float a[3])
{
  this->AddOrientation(a[0],a[1],a[2]);
}

// Rotate the Prop3D in degrees about the X axis using the right hand rule. The
// axis is the Prop3D's X axis, which can change as other rotations are performed.
// To rotate about the world X axis use RotateWXYZ (angle, 1, 0, 0). This rotation
// is applied before all others in the current transformation matrix.
void vtkProp3D::RotateX (float angle)
{
  this->Transform->PreMultiply ();
  this->Transform->RotateX(angle);
  this->Modified();
} 

// Rotate the Prop3D in degrees about the Y axis using the right hand rule. The
// axis is the Prop3D's Y axis, which can change as other rotations are performed.
// To rotate about the world Y axis use RotateWXYZ (angle, 0, 1, 0). This rotation
// is applied before all others in the current transformation matrix.
void vtkProp3D::RotateY (float angle)
{
  this->Transform->PreMultiply ();
  this->Transform->RotateY(angle);
  this->Modified();
} 

// Rotate the Prop3D in degrees about the Z axis using the right hand rule. The
// axis is the Prop3D's Z axis, which can change as other rotations are performed.
// To rotate about the world Z axis use RotateWXYZ (angle, 0, 0, 1). This rotation
// is applied before all others in the current transformation matrix.

void vtkProp3D::RotateZ (float angle)
{
  this->Transform->PreMultiply ();
  this->Transform->RotateZ(angle);
  this->Modified();
} 

// Rotate the Prop3D in degrees about an arbitrary axis specified by the 
// last three arguments. The axis is specified in world coordinates. To
// rotate an about its model axes, use RotateX, RotateY, RotateZ.
void vtkProp3D::RotateWXYZ (float degree, float x, float y, float z)
{
  this->Transform->PostMultiply();  
  this->Transform->RotateWXYZ(degree,x,y,z);
  this->Transform->PreMultiply();  
  this->Modified();
}

void vtkProp3D::GetMatrix(vtkMatrix4x4 *result)
{
  this->GetMatrix(&result->Element[0][0]);
  result->Modified();
}

void vtkProp3D::GetMatrix(double result[16])
{
  // check whether or not need to rebuild the matrix
  if ( this->GetMTime() > this->MatrixMTime )
    {
    this->GetOrientation();
    this->Transform->Push();  
    this->Transform->Identity();  
    this->Transform->PostMultiply();  
    
    // shift back to actor's origin
    this->Transform->Translate(-this->Origin[0],
                              -this->Origin[1],
                              -this->Origin[2]);

    // scale
    this->Transform->Scale(this->Scale[0],
                          this->Scale[1],
                          this->Scale[2]);
    
    // rotate
    this->Transform->RotateY(this->Orientation[1]);
    this->Transform->RotateX(this->Orientation[0]);
    this->Transform->RotateZ(this->Orientation[2]);
    
    // move back from origin and translate
    this->Transform->Translate(this->Origin[0] + this->Position[0],
                              this->Origin[1] + this->Position[1],
                              this->Origin[2] + this->Position[2]);

    // apply user defined transform if it is present
    if (this->UserTransform)
      {
      this->UserTransform->Update();
      this->Transform->Concatenate(this->UserTransform->GetMatrixPointer());
      }

    // apply user defined matrix last if there is one 
    if (this->UserMatrix)
      {
      this->Transform->Concatenate(this->UserMatrix);
      }

    this->Transform->PreMultiply();  
    this->Transform->GetMatrix(this->Matrix);
    this->MatrixMTime.Modified();
    this->Transform->Pop();  
    }
  vtkMatrix4x4::DeepCopy(result,this->Matrix);
} 

// Return a reference to the Prop3D's 4x4 composite matrix.
vtkMatrix4x4 *vtkProp3D::GetMatrixPointer()
{
  this->GetMatrix(this->Matrix);
  return this->Matrix;
} 


// Get the bounds for this Prop3D as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
void vtkProp3D::GetBounds(float bounds[6])
{
  this->GetBounds();
  for (int i=0; i<6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}

// Get the center of the bounding box in world coordinates.
float *vtkProp3D::GetCenter()
{
  this->GetBounds();
  this->Center[0] = (this->Bounds[1] + this->Bounds[0])/2.0;
  this->Center[1] = (this->Bounds[3] + this->Bounds[2])/2.0;
  this->Center[2] = (this->Bounds[5] + this->Bounds[4])/2.0;
  
  return this->Center;
}

// Get the length of the diagonal of the bounding box.
float vtkProp3D::GetLength()
{
  double diff, l=0.0;
  int i;

  this->GetBounds();
  for (i=0; i<3; i++)
    {
    diff = this->Bounds[2*i+1] - this->Bounds[2*i];
    l += diff * diff;
    }
 
  return (float)sqrt(l);
}

// Get the Prop3D's x range in world coordinates.
float *vtkProp3D::GetXRange()
{
  this->GetBounds();
  return this->Bounds;
}

// Get the Prop3D's y range in world coordinates.
float *vtkProp3D::GetYRange()
{
  this->GetBounds();
  return &(this->Bounds[2]);
}

// Get the Prop3D's z range in world coordinates.
float *vtkProp3D::GetZRange()
{
  this->GetBounds();
  return &(this->Bounds[4]);
}

void vtkProp3D::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  vtkProp::PrintSelf(os,indent);

  os << indent << "Position: (" << this->Position[0] << ", " 
     << this->Position[1] << ", " << this->Position[2] << ")\n";

  os << indent << "Orientation: (" << this->Orientation[0] << ", " 
     << this->Orientation[1] << ", " << this->Orientation[2] << ")\n";

  os << indent << "Origin: (" << this->Origin[0] << ", " 
     << this->Origin[1] << ", " << this->Origin[2] << ")\n";

  os << indent << "Scale: (" << this->Scale[0] << ", " 
     << this->Scale[1] << ", " << this->Scale[2] << ")\n";

  os << indent << "UserTransform: ";
  if (this->UserTransform)
    {
    os << this->UserTransform << "\n"; 
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << "UserMatrix: ";
  if (this->UserMatrix)
    {
    os << this->UserMatrix << "\n"; 
    }
  else
    {
    os << "(none)\n";
    }
}

