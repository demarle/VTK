/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTransform.cxx
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
#include <stdlib.h>
#include "vtkTransform.h"
#include "vtkMath.h"

// Constructs a transform and sets the following defaults
// preMultiplyFlag = 1 stackSize = 10. It then
// creates an identity matrix as the top matrix on the stack.
vtkTransform::vtkTransform ()
{
  this->PreMultiplyFlag = 1;
  this->StackSize = 10;
  this->Stack = new vtkMatrix4x4 *[this->StackSize];
  *this->Stack = vtkMatrix4x4::New();
  this->StackBottom = this->Stack;
  this->Identity();

  this->Point[0] = this->Point[1] = this->Point[2] = this->Point[3] = 0.0;
  this->Orientation[0] = this->Orientation[1] = this->Orientation[2] = 0.0;
}

// Copy constructor. Creates an instance of vtkTransform and then
// copies its instance variables from the values in t. 
vtkTransform::vtkTransform (const vtkTransform& t)
{
  int i, n;

  this->PreMultiplyFlag = t.PreMultiplyFlag;
  this->StackSize = t.StackSize;
  this->StackBottom = new vtkMatrix4x4 *[this->StackSize];

  // now copy each matrix in the stack
  for ( n=t.Stack-t.StackBottom+1, i=0; i < n; i++ )
    {
    this->StackBottom[i] = new vtkMatrix4x4(*(t.Stack[i]));
    (this->StackBottom[i])->DeepCopy(t.Stack[i]);
    }

  this->Stack = this->StackBottom + (n - 1);
}

vtkTransform& vtkTransform::operator=(const vtkTransform& t)
{
  int i, n;

  this->PreMultiplyFlag = t.PreMultiplyFlag;
  this->StackSize = t.StackSize;
  // free old memory
  if (this->StackBottom)
    {
    for (n=this->Stack-this->StackBottom+1, i=0; i < n; i++)
      {
      this->StackBottom[i]->Delete();
      }
    delete [] this->StackBottom;
    } 
  this->StackBottom = new vtkMatrix4x4 *[this->StackSize];
  for ( n=t.Stack-t.StackBottom+1, i=0; i < n; i++ )
    {
    this->StackBottom[i] = vtkMatrix4x4::New();
    (this->StackBottom[i])->DeepCopy(t.Stack[i]);
    }
  this->Stack = this->StackBottom + (n - 1);

  for ( i=0; i < 3; i++)
    {
    this->Point[i] = t.Point[i];
    this->Orientation[i] = t.Orientation[i];
    }

  this->Modified();
  return *this;
}

// Deletes the transformation on the top of the stack and sets the top 
// to the next transformation on the stack.
void vtkTransform::Pop ()
{
  // if we're at the bottom of the stack, don't pop
  if (this->Stack == this->StackBottom)
    {
    return;
    }

  // free the stack matrix storage
  (*this->Stack)->Delete();
  *this->Stack = NULL;

  // update the stack
  this->Stack--;

  this->Modified ();
}

// Sets the internal state of the transform to
// post multiply. All subsequent matrix
// operations will occur after those already represented
// in the current transformation matrix.
void vtkTransform::PostMultiply ()
{
  if (this->PreMultiplyFlag != 0) 
    {
    this->PreMultiplyFlag = 0;
    this->Modified ();
    }
}

// Sets the internal state of the transform to
// pre multiply. All subsequent matrix
// operations will occur before those already represented
// in the current transformation matrix.
void vtkTransform::PreMultiply ()
{
  if (this->PreMultiplyFlag != 1) 
    {
    this->PreMultiplyFlag = 1;
    this->Modified ();
    }
}

// Pushes the current transformation matrix onto the
// transformation stack.
void vtkTransform::Push ()
{
  vtkMatrix4x4 *ctm;

  ctm = *this->Stack;
  this->Stack++;
  if ((this->Stack - this->StackBottom) > this->StackSize) 
    {
    this->Stack--;
    vtkErrorMacro(<<"Exceeded matrix stack size");
    ctm->Delete();
    return;
    }

  // allocate a new matrix on the stack
  (*this->Stack) = vtkMatrix4x4::New();
  
  // set the new matrix to the previous top of stack matrix
  (*this->Stack)->DeepCopy(ctm);

  this->Modified ();
}

// Creates an x rotation matrix and concatenates it with 
// the current transformation matrix. The angle is specified
// in degrees.
void vtkTransform::RotateX ( float angle)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New();
  float radians = angle * vtkMath::DegreesToRadians();
  float cosAngle, sinAngle;

  if (angle != 0.0) 
    {
    cosAngle = cos (radians);
    sinAngle = sin (radians);

    ctm->Element[0][0] = 1.0;
    ctm->Element[1][1] =  cosAngle;
    ctm->Element[2][1] =  sinAngle;
    ctm->Element[1][2] = -sinAngle;
    ctm->Element[2][2] =  cosAngle;
    ctm->Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Creates a y rotation matrix and concatenates it with
// the current transformation matrix. The angle is specified
// in degrees.
void vtkTransform::RotateY ( float angle)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New();
  float radians = angle * vtkMath::DegreesToRadians();
  float cosAngle, sinAngle;

  if (angle != 0.0) 
    {
    cosAngle = cos (radians);
    sinAngle = sin (radians);

    ctm->Element[0][0] = cosAngle;
    ctm->Element[1][1] = 1.0;
    ctm->Element[2][0] = -sinAngle;
    ctm->Element[0][2] = sinAngle;
    ctm->Element[2][2] = cosAngle;
    ctm->Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Creates a z rotation matrix and concatenates it with
// the current transformation matrix. The angle is specified
// in degrees.
void vtkTransform::RotateZ (float angle)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New();
  float radians = angle * vtkMath::DegreesToRadians();
  float cosAngle, sinAngle;

  if (angle != 0.0) 
    {
    cosAngle = cos (radians);
    sinAngle = sin (radians);

    ctm->Element[0][0] =  cosAngle;
    ctm->Element[1][0] =  sinAngle;
    ctm->Element[0][1] = -sinAngle;
    ctm->Element[1][1] =  cosAngle;
    ctm->Element[2][2] = 1.0;
    ctm->Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Creates a matrix that rotates angle degrees about an axis
// through the origin and x, y, z. It then concatenates
// this matrix with the current transformation matrix.
void vtkTransform::RotateWXYZ ( float angle, float x, float y, float z)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New();
  float   radians;
  float   w;
  float   quat[4];
  float   sinAngle;
  float   cosAngle;

  // build a rotation matrix and concatenate it
  quat[0] = angle;
  quat[1] = x;
  quat[2] = y;
  quat[3] = z;

  // convert degrees to radians
  radians = quat[0] * vtkMath::DegreesToRadians() / 2;

  cosAngle = cos (radians);
  sinAngle = sin (radians);

  // normalize x, y, z
  if ( vtkMath::Normalize(quat+1) == 0.0 )
    {
    vtkErrorMacro(<<"Trying to rotate around zero-length axis");
    return;
    }

  w = cosAngle;
  x = quat[1] * sinAngle;
  y = quat[2] * sinAngle;
  z = quat[3] * sinAngle;

  // matrix calculation is taken from Ken Shoemake's
  // "Animation Rotation with Quaternion Curves",
  // Comput. Graphics, vol. 19, No. 3 , p. 253

  ctm->Element[0][0] = 1 - 2 * y * y - 2 * z * z;
  ctm->Element[1][1] = 1 - 2 * x * x - 2 * z * z;
  ctm->Element[2][2] = 1 - 2 * x * x - 2 * y * y;
  ctm->Element[1][0] =  2 * x * y + 2 * w * z;
  ctm->Element[2][0] =  2 * x * z - 2 * w * y;
  ctm->Element[0][1] =  2 * x * y - 2 * w * z;
  ctm->Element[2][1] =  2 * y * z + 2 * w * x;
  ctm->Element[0][2] =  2 * x * z + 2 * w * y;
  ctm->Element[1][2] =  2 * y * z - 2 * w * x;

  // concatenate with current transformation matrix
  this->Concatenate (ctm);
  ctm->Delete();
}

void vtkTransform::RotateWXYZ ( double angle, double x, double y, double z)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New();
  double   radians;
  double   w;
  double   quat[4];
  double   sinAngle;
  double   cosAngle;

  // build a rotation matrix and concatenate it
  quat[0] = angle;
  quat[1] = x;
  quat[2] = y;
  quat[3] = z;

  // convert degrees to radians
  radians = quat[0] * vtkMath::DegreesToRadians() / 2;

  cosAngle = cos (radians);
  sinAngle = sin (radians);

  // normalize x, y, z
  if ( vtkMath::Normalize(quat+1) == 0.0 )
    {
    vtkErrorMacro(<<"Trying to rotate around zero-length axis");
    return;
    }

  w = cosAngle;
  x = quat[1] * sinAngle;
  y = quat[2] * sinAngle;
  z = quat[3] * sinAngle;

  // matrix calculation is taken from Ken Shoemake's
  // "Animation Rotation with Quaternion Curves",
  // Comput. Graphics, vol. 19, No. 3 , p. 253

  ctm->Element[0][0] = 1 - 2 * y * y - 2 * z * z;
  ctm->Element[1][1] = 1 - 2 * x * x - 2 * z * z;
  ctm->Element[2][2] = 1 - 2 * x * x - 2 * y * y;
  ctm->Element[1][0] =  2 * x * y + 2 * w * z;
  ctm->Element[2][0] =  2 * x * z - 2 * w * y;
  ctm->Element[0][1] =  2 * x * y - 2 * w * z;
  ctm->Element[2][1] =  2 * y * z + 2 * w * x;
  ctm->Element[0][2] =  2 * x * z + 2 * w * y;
  ctm->Element[1][2] =  2 * y * z - 2 * w * x;

  // concatenate with current transformation matrix
  this->Concatenate (ctm);
  ctm->Delete();
}

// Scales the current transformation matrix in the x, y and z directions.
// A scale factor of zero will automatically be replaced with one.
void vtkTransform::Scale ( float x, float y, float z)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New(); //constructed as identity

  if (x != 1.0 || y != 1.0 || z != 1.0) 
    {
    ctm->Element[0][0] = x;
    ctm->Element[1][1] = y;
    ctm->Element[2][2] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Scales the current transformation matrix in the x, y and z directions.
// A scale factor of zero will automatically be replaced with one.
void vtkTransform::Scale ( double x, double y, double z)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New(); //constructed as identity

  if (x != 1.0 || y != 1.0 || z != 1.0) 
    {
    ctm->Element[0][0] = x;
    ctm->Element[1][1] = y;
    ctm->Element[2][2] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Translate the current transformation matrix by the vector {x, y, z}.
void vtkTransform::Translate ( float x, float y, float z)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New(); //constructed as identity matrix

  if (x != 0.0 || y != 0.0 || z != 0.0) 
    {
    ctm->Element[0][3] = x;
    ctm->Element[1][3] = y;
    ctm->Element[2][3] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Translate the current transformation matrix by the vector {x, y, z}.
void vtkTransform::Translate ( double x, double y, double z)
{
  vtkMatrix4x4 *ctm = vtkMatrix4x4::New(); //constructed as identity matrix

  if (x != 0.0 || y != 0.0 || z != 0.0) 
    {
    ctm->Element[0][3] = x;
    ctm->Element[1][3] = y;
    ctm->Element[2][3] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
    }
  ctm->Delete();
}

// Obtain the transpose of the current transformation matrix.
void vtkTransform::GetTranspose (vtkMatrix4x4 *transpose)
{
  vtkMatrix4x4 *temp = vtkMatrix4x4::New();
  int i, j;

  for (i = 0; i < 4; i++) 
    {
    for (j = 0; j < 4; j++) 
      {
      temp->Element[j][i] = (**this->Stack).Element[i][j];
      }
    }    
  transpose->DeepCopy (temp);
  temp->Delete();
}

// Invert the current transformation matrix.
void vtkTransform::Inverse ()
{
  (*this->Stack)->Invert(*this->Stack, *this->Stack);

  this->Modified ();
}

// Return the inverse of the current transformation matrix.
void vtkTransform::GetInverse ( vtkMatrix4x4 *inverse)
{
  inverse->Invert(*this->Stack, inverse);
}

// Get the x, y, z orientation angles from the transformation matrix.
void vtkTransform::GetOrientation(float *prx, float *pry, float *prz)
{
  float *orientation=this->GetOrientation();
  *prx = orientation[0];
  *pry = orientation[1];
  *prz = orientation[2];
}

// Get the x, y, z orientation angles from the transformation matrix as an
// array of three floating point values.
float *vtkTransform::GetOrientation ()
{
#define VTK_AXIS_EPSILON 0.001
  float	scaleX, scaleY, scaleZ;
  vtkMatrix4x4  *temp = vtkMatrix4x4::New();
  float   x,y,z;
  float   d;
  float   d1;
  float   d2;
  float   dot;
  float   alpha;
  float   phi;
  float   theta;
  float   cosPhi, sinPhi;
  float   cosTheta, sinTheta;
  float   cosAlpha, sinAlpha;
  float   x2, y2, z2;
  float   x3, y3, z3;
  float   x3p, y3p;

  // copy the matrix into local storage
  temp->DeepCopy(*this->Stack);

  // get scale factors
  this->GetScale (scaleX, scaleY, scaleZ);

  // first rotate about y axis
  x2 = temp->Element[2][0] / scaleX;
  y2 = temp->Element[2][1] / scaleY;
  z2 = temp->Element[2][2] / scaleZ;

  x3 = temp->Element[1][0] / scaleX;
  y3 = temp->Element[1][1] / scaleY;
  z3 = temp->Element[1][2] / scaleZ;

  dot = x2 * x2 + z2 * z2;
  d1 = sqrt (dot);

  if (d1 < VTK_AXIS_EPSILON) 
    {
    cosTheta = 1.0;
    sinTheta = 0.0;
    }
  else 
    {
    cosTheta = z2 / d1;
    sinTheta = x2 / d1;
    }

  theta = atan2 (sinTheta, cosTheta);

  y = -theta / vtkMath::DegreesToRadians();

  // now rotate about x axis
  dot = x2 * x2 + y2 * y2 + z2 * z2;
  d = sqrt (dot);

  if (d < VTK_AXIS_EPSILON) 
    {
    sinPhi = 0.0;
    cosPhi = 1.0;
    }
  else if (d1 < VTK_AXIS_EPSILON) 
    {
    sinPhi = y2 / d;
    cosPhi = z2 / d;
    }
  else 
    {
    sinPhi = y2 / d;
    cosPhi = ( x2 * x2 + z2 * z2) / (d1 * d);
    }

  phi = atan2 (sinPhi, cosPhi);

  x = phi / vtkMath::DegreesToRadians();

  // finally, rotate about z
  x3p = x3 * cosTheta - z3 * sinTheta;
  y3p = - sinPhi * sinTheta * x3 + cosPhi * y3 - sinPhi * cosTheta * z3;
  dot = x3p * x3p + y3p * y3p;

  d2 = sqrt (dot);
  if (d2 < VTK_AXIS_EPSILON) 
    {
    cosAlpha = 1.0;
    sinAlpha = 0.0;
    }
  else 
    {
    cosAlpha = y3p / d2;
    sinAlpha = x3p / d2;
    }

  alpha = atan2 (sinAlpha, cosAlpha);

  z = alpha / vtkMath::DegreesToRadians();

  this->Orientation[0] = x;
  this->Orientation[1] = y;
  this->Orientation[2] = z;

  temp->Delete();
  return this->Orientation;
}

// Return the x, y, z positions from the current transformation matrix.
// This is simply returning the translation component of the 4x4 matrix.
void vtkTransform::GetPosition (float *px, float *py, float *pz)
{
  *px = (**this->Stack).Element[0][3];
  *py = (**this->Stack).Element[1][3];
  *pz = (**this->Stack).Element[2][3];
}

// vtkTransform::GetOrientationWXYZ 
float *vtkTransform::GetOrientationWXYZ ()
{
  float	scaleX, scaleY, scaleZ;
  vtkTransform *temp1 = vtkTransform::New();
  vtkMatrix4x4 *temp = vtkMatrix4x4::New();
  float quat[4];
  static float WXYZ[4];
  float mag;
  
  // copy the matrix into local storage
  temp1->SetMatrix(**this->Stack);

  // get scale factors
  this->GetScale (scaleX, scaleY, scaleZ);
  temp1->Scale(1.0/scaleX,1.0/scaleY,1.0/scaleZ);
  temp->DeepCopy(*temp1->Stack);
  
  quat[0] = 0.25*(1.0 + temp->Element[0][0] + temp->Element[1][1] 
		  + temp->Element[2][2]);

  if (quat[0] > 0.0001)
    {
    quat[0] = sqrt(quat[0]);
    quat[1] = (temp->Element[2][1] - temp->Element[1][2])/(4.0*quat[0]);
    quat[2] = (temp->Element[0][2] - temp->Element[2][0])/(4.0*quat[0]);
    quat[3] = (temp->Element[1][0] - temp->Element[0][1])/(4.0*quat[0]);
    }
  else
    {
    quat[0] = 0;
    quat[1] = -0.5*(temp->Element[1][1] + temp->Element[2][2]);
    if (quat[1] > 0.0001)
      {
      quat[1] = sqrt(quat[1]);
      quat[2] = temp->Element[1][0]/(2.0*quat[1]);
      quat[3] = temp->Element[2][0]/(2.0*quat[1]);
      }
    else
      {
      quat[1] = 0;
      quat[2] = 0.5*(1.0 - temp->Element[2][2]);
      if (quat[2] > 0.0001)
	{
	quat[2] = sqrt(quat[2]);
	quat[3] = temp->Element[2][1]/(2.0*quat[2]);
	}
      else
	{
	quat[2] = 0;
	quat[3] = 1;
	}
      }
    }
  
  // calc the wxyz
  mag = sqrt(quat[1]*quat[1] + quat[2]*quat[2] + quat[3]*quat[3]);

  if (mag)
    {
    WXYZ[0] = 180.0*2.0*acos(quat[0])/3.1415926;
    WXYZ[1] = quat[1]/mag;
    WXYZ[2] = quat[2]/mag;
    WXYZ[3] = quat[3]/mag;
    }
  else
    {
    WXYZ[0] = 0;
    WXYZ[1] = 0;
    WXYZ[2] = 0;
    WXYZ[3] = 1;
    }
  
  temp1->Delete();
  temp->Delete();
  return WXYZ;
} // vtkTransform::GetOrientationWXYZ 

// Return the position from the current transformation matrix as an array
// of three floating point numbers. This is simply returning the translation 
// component of the 4x4 matrix.
float *vtkTransform::GetPosition()
{
  static float pos[3];

  pos[0] = (**this->Stack).Element[0][3];
  pos[1] = (**this->Stack).Element[1][3];
  pos[2] = (**this->Stack).Element[2][3];

  return pos;
}

// Return the x, y, z scale factors of the current transformation matrix.
void vtkTransform::GetScale (float *px, float *py, float *pz)
{
  float *scale=this->GetScale();

  *px = scale[0];
  *py = scale[1];
  *pz = scale[2];
}

// Return the x, y, z scale factors of the current transformation matrix as 
// an array of three float numbers.
float *vtkTransform::GetScale()
{
  int	i;
  static float scale[3];
  vtkMatrix4x4 *temp;

  temp = *this->Stack;

  // find scale factors

  for (i = 0; i < 3; i++) 
    {
    scale[i] = sqrt (temp->Element[0][i] * temp->Element[0][i] +
                     temp->Element[1][i] * temp->Element[1][i] +
                     temp->Element[2][i] * temp->Element[2][i]);
    }

  return scale;
}

// Returns the current transformation matrix.
vtkMatrix4x4 *vtkTransform::GetMatrixPointer()
{
  return *this->Stack;
}

// Set the current matrix directly.
void vtkTransform::SetMatrix(vtkMatrix4x4 &m)
{
  (*this->Stack)->DeepCopy(&m);
}

// Creates an identity matrix and makes it the current transformation matrix.
void vtkTransform::Identity()
{
  vtkMatrix4x4 *ctm;
  int i,j;

  ctm = *this->Stack;
  
  for (j = 0; j < 4; j++) 
    {
    for (i = 0; i < 4; i++) 
      {
      if (i == j)
	{
	ctm->Element[i][j] = 1.0;
	}
      else
	{
	ctm->Element[i][j] = 0.0;	
	}
      }
    }

  this->Modified();
}

// Concatenates the input matrix with the current transformation matrix.
// The resulting matrix becomes the new current transformation matrix.
// The setting of the PreMultiply flag determines whether the matrix
// is PreConcatenated or PostConcatenated.
void vtkTransform::Concatenate (vtkMatrix4x4 *matrix)
{
  if (this->PreMultiplyFlag) 
    {
    this->Multiply4x4 (*this->Stack, matrix, *this->Stack);
    }
  else 
    {
    this->Multiply4x4 (matrix, *this->Stack, *this->Stack);
    }
  this->Modified ();
}

// Multiplies matrices a and b and stores the result in c.
void vtkTransform::Multiply4x4(vtkMatrix4x4 *a, vtkMatrix4x4 *b, 
			       vtkMatrix4x4 *c)
{
  int i, j, k;
  vtkMatrix4x4 *result = vtkMatrix4x4::New();

  // we need the temporary matrix because of the case c=a or c=b;
  result->Zero();
  for (i = 0; i < 4; i++) 
    {
    for (k = 0; k < 4; k++) 
      {
      for (j = 0; j < 4; j++) 
        {
        result->Element[i][k] += a->Element[i][j] * b->Element[j][k];
        }
      }
    }
  c->DeepCopy(result);
  result->Delete();
}

// Transposes the current transformation matrix.
void vtkTransform::Transpose ()
{
  this->GetTranspose(*this->Stack);
  this->Modified();
}

// Returns the current transformation matrix.
void vtkTransform::GetMatrix (vtkMatrix4x4 *ctm)
{
  ctm->DeepCopy(*this->Stack);
}

vtkTransform::~vtkTransform ()
{
  int i, n;

  for (n=this->Stack-this->StackBottom+1, i=0; i < n; i++)
    {
    this->StackBottom[i]->Delete();
    }

  delete [] this->StackBottom;
}

void vtkTransform::PrintSelf (ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os, indent);

  os << indent << (this->PreMultiplyFlag ? "PreMultiply\n" : "PostMultiply\n");

  os << indent << "Point: " << "( " << 
     this->Point[0] << ", " << this->Point[1] << ", " <<
     this->Point[2] << ", " << this->Point[3] << "\n";

  os << indent << "Current Transformation:" << "\n";

  (**this->Stack).PrintSelf (os, indent.GetNextIndent());
}

// Returns the result of multiplying the currently set Point by the current 
// transformation matrix. Point is expressed in homogeneous coordinates.
// The setting of the PreMultiplyFlag will determine if the Point is
// Pre or Post multiplied.
float *vtkTransform::GetPoint()
{
  if (this->PreMultiplyFlag)
    {
    this->Stack[0]->PointMultiply(this->Point,this->Point);
    }
  else
    {
    this->Stack[0]->MultiplyPoint(this->Point,this->Point);
    }
  return this->Point;
}

double *vtkTransform::GetDoublePoint()
{
  if (this->PreMultiplyFlag)
    {
    this->Stack[0]->PointMultiply(this->DoublePoint,this->DoublePoint);
    }
  else
    {
    this->Stack[0]->MultiplyPoint(this->DoublePoint,this->DoublePoint);
    }
  return this->DoublePoint;
}

void vtkTransform::GetPoint(float p[4])
{
  float *x=this->vtkTransform::GetPoint();
  for (int i=0; i<4; i++)
    {
    p[i] = x[i];
    }
}

// Multiplies a list of points (inPts) by the current transformation matrix.
// Transformed points are appended to the output list (outPts).
void vtkTransform::MultiplyPoints(vtkPoints *inPts, vtkPoints *outPts)
{
  float newX[3];
  float *x;
  int ptId, i;
  int numPts = inPts->GetNumberOfPoints();

  for (ptId=0; ptId < numPts; ptId++)
    {
    x = inPts->GetPoint(ptId);
    for (i=0; i<3; i++)
      {
      newX[i] = (**this->Stack).Element[i][0] * x[0] +
                (**this->Stack).Element[i][1] * x[1] +
                (**this->Stack).Element[i][2] * x[2] +
                (**this->Stack).Element[i][3];
      }

    outPts->InsertNextPoint(newX);
    }
}

// Multiplies a list of vectors (inVectors) by the current transformation 
// matrix. The transformed vectors are appended to the output list 
// (outVectors). This is a special multiplication, since these are vectors. 
// It multiplies vectors by the transposed inverse of the matrix, ignoring 
// the translational components.
void vtkTransform::MultiplyVectors(vtkVectors *inVectors, 
				   vtkVectors *outVectors)
{
  float newV[3];
  float *v;
  int ptId, i;
  int numVectors = inVectors->GetNumberOfVectors();

  vtkMatrix4x4 *aMatrix = vtkMatrix4x4::New();
  
  aMatrix->DeepCopy(*this->Stack);
  aMatrix->Invert();
  aMatrix->Transpose();

  for (ptId=0; ptId < numVectors; ptId++)
    {
    v = inVectors->GetVector(ptId);
    for (i=0; i<3; i++)
      {
      newV[i] = aMatrix->Element[i][0] * v[0] +
                aMatrix->Element[i][1] * v[1] +
                aMatrix->Element[i][2] * v[2];
      }

    outVectors->InsertNextVector(newV);
    }
  aMatrix->Delete();
}


// Multiplies a list of normals (inNormals) by the current
// transformation matrix.  The transformed normals are then appended
// to the output list (outNormals).  This is a special multiplication,
// since these are normals. It multiplies the normals by the
// transposed inverse of the matrix, ignoring the translational
// components.
void vtkTransform::MultiplyNormals(vtkNormals *inNormals, vtkNormals *outNormals)
{
  float newN[3];
  float *n;
  int ptId, i;
  int numNormals = inNormals->GetNumberOfNormals();

  vtkMatrix4x4 *aMatrix = vtkMatrix4x4::New();
  
  aMatrix->DeepCopy(*this->Stack);
  aMatrix->Invert ();
  aMatrix->Transpose ();
  
  for (ptId=0; ptId < numNormals; ptId++)
    {
    n = inNormals->GetNormal(ptId);
    for (i=0; i<3; i++)
      {
      newN[i] = aMatrix->Element[i][0] * n[0] +
                aMatrix->Element[i][1] * n[1] +
                aMatrix->Element[i][2] * n[2];
      }

    vtkMath::Normalize(newN);
    outNormals->InsertNextNormal(newN);
    }
  aMatrix->Delete();
}
