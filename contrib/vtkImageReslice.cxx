/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageReslice.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to David G Gobbi who developed this class.

Copyright (c) 1993-1999 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <limits.h>
#include <float.h>
#include <math.h>
#include "vtkImageCache.h"
#include "vtkImageReslice.h"
#include "vtkMath.h"
#include <stdio.h>

//----------------------------------------------------------------------------
vtkImageReslice::vtkImageReslice()
{
  this->OutputSpacing[0] = 1;
  this->OutputSpacing[1] = 1;
  this->OutputSpacing[2] = 1;

  this->OutputOrigin[0] = FLT_MAX; // flag to set defaults later
  this->OutputExtent[0] = INT_MAX; // ditto

  this->Wrap = 0; // don't wrap

  this->Interpolate = 0; // nearest-neighbor interpolation by default
  this->InterpolationMode = VTK_RESLICE_LINEAR;
  this->Optimization = 1; // optimizations seem to finally be stable...
  this->BackgroundLevel = 0;

  this->ResliceTransform = NULL;
}

//----------------------------------------------------------------------------
vtkImageReslice::~vtkImageReslice()
{
  this->SetResliceTransform(NULL);
}

//----------------------------------------------------------------------------
void vtkImageReslice::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageFilter::PrintSelf(os,indent);

  if (this->ResliceTransform)
    {
    os << indent << "ResliceTransform: " << this->ResliceTransform << "\n";
    this->ResliceTransform->PrintSelf(os,indent.GetNextIndent());
    }
  os << indent << "OutputSpacing: " << this->OutputSpacing[0] << " " <<
    this->OutputSpacing[1] << " " << this->OutputSpacing[2] << "\n";
  os << indent << "OutputOrigin: " << this->OutputOrigin[0] << " " <<
    this->OutputOrigin[1] << " " << this->OutputOrigin[2] << "\n";
  os << indent << "OutputExtent: " << this->OutputExtent[0] << " " <<
    this->OutputExtent[1] << " " << this->OutputExtent[2] << " " <<
    this->OutputExtent[3] << " " << this->OutputExtent[4] << " " <<
    this->OutputExtent[5] << "\n";
  os << indent << "Wrap: " << (this->Wrap ? "On\n":"Off\n");
  os << indent << "Interpolate: " << (this->Interpolate ? "On\n":"Off\n");
  os << indent << "InterpolationMode: " 
     << this->GetInterpolationModeAsString() << "\n";
  os << indent << "Optimization: " << (this->Optimization ? "On\n":"Off\n");
  os << indent << "BackgroundLevel: " << this->BackgroundLevel << "\n";
}

//----------------------------------------------------------------------------
// The transform matrix supplied by the user converts output coordinates
// to input coordinates.  
// To speed up the pixel lookup, the following function provides a
// matrix which converts output pixel indices to input pixel indices.

void vtkImageReslice::ComputeIndexMatrix(vtkMatrix4x4 *matrix)
{
  int i;
  float inOrigin[3];
  float inSpacing[3];
  float outOrigin[3];
  float outSpacing[3];

  this->GetInput()->GetSpacing(inSpacing);
  this->GetInput()->GetOrigin(inOrigin);
  this->GetOutput()->GetSpacing(outSpacing);
  this->GetOutput()->GetOrigin(outOrigin);  
  
  vtkTransform *transform = vtkTransform::New();
  vtkMatrix4x4 *inMatrix = vtkMatrix4x4::New();
  vtkMatrix4x4 *outMatrix = vtkMatrix4x4::New();

  if (this->GetResliceTransform())
    {
    transform->SetMatrix(*(this->GetResliceTransform()->GetMatrixPointer()));
    }
  
  // the outMatrix takes OutputData indices to OutputData coordinates,
  // the inMatrix takes InputData coordinates to InputData indices

  for (i = 0; i < 3; i++) 
    {
    inMatrix->Element[i][i] = 1/inSpacing[i];
    inMatrix->Element[i][3] = -inOrigin[i]/inSpacing[i];
    outMatrix->Element[i][i] = outSpacing[i];
    outMatrix->Element[i][3] = outOrigin[i];
    }
  
  transform->PreMultiply();
  transform->Concatenate(outMatrix);
  transform->PostMultiply();
  transform->Concatenate(inMatrix);

  transform->GetMatrix(matrix);
  
  transform->Delete();
  inMatrix->Delete();
  outMatrix->Delete();
}

//----------------------------------------------------------------------------
static void ComputeRequiredInputUpdateExtentOptimized(vtkImageReslice *self,
						      int inExt[6], 
						      int outExt[6]);

void vtkImageReslice::ComputeRequiredInputUpdateExtent(int inExt[6], 
						       int outExt[6])
{
  if (this->GetOptimization())
    {
    ComputeRequiredInputUpdateExtentOptimized(this,inExt,outExt);
    return;
    }

  int i,j,k;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  float point[4];

  // convert matrix from world coordinates to pixel indices
  this->ComputeIndexMatrix(matrix);

  for (i = 0; i < 3; i++)
    {
    inExt[2*i] = INT_MAX;
    inExt[2*i+1] = INT_MIN;
    }

  // check the coordinates of the 8 corners of the output extent
  for (i = 0; i < 8; i++)  
    {
    // get output coords
    point[0] = outExt[i%2];
    point[1] = outExt[2+(i/2)%2];
    point[2] = outExt[4+(i/4)%2];
    point[3] = 1.0;

    // convert to input coords
    matrix->MultiplyPoint(point,point);

    point[0] /= point[3];
    point[1] /= point[3];
    point[2] /= point[3];
    point[3] = 1.0;

    // set 
    if (this->GetInterpolate()
	* this->GetInterpolationMode() != VTK_RESLICE_NEAREST)
      {
      int extra = (this->GetInterpolationMode() == VTK_RESLICE_CUBIC); 
      for (j = 0; j < 3; j++) 
	{
	k = int(floor(point[j]))-extra;
	if (k < inExt[2*j]) 
	  {
	  inExt[2*j] = k;
	  } 
	k = int(ceil(point[j]))+extra;	
	if (k > inExt[2*j+1])
	  {
	  inExt[2*j+1] = k;
	  }
	}
      }
    else
      {
      for (j = 0; j < 3; j++) 
	{
	k = int(floor(point[j] + 0.5));
	if (k < inExt[2*j])
	  { 
	  inExt[2*j] = k;
	  } 
	if (k > inExt[2*j+1]) 
	  {
	  inExt[2*j+1] = k;
	  }
	}
      }
    }
  matrix->Delete();
}

//----------------------------------------------------------------------------
void vtkImageReslice::ExecuteImageInformation() 
{
  int i,j;
  float inPoint[4], outPoint[4];
  float inOrigin[3],maxOut[3],minOut[3],tmp;
  float *inSpacing;

  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  int *inExt;

  inExt = this->Input->GetWholeExtent();
  inSpacing = this->Input->GetSpacing();
  this->Input->GetOrigin(inOrigin);
  
  if (this->ResliceTransform)
    {  
    this->ResliceTransform->GetMatrix(matrix);

    // because vtkMatrix4x4::Inverse() doesn't cut it,
    // use vtkMath::InvertMatrix()
    double mat1data[4][4];
    double mat2data[4][4];
    double *mat1[4];
    double *mat2[4];
    
    for (i = 0; i < 4; i++)
      {
      mat1[i] = mat1data[i];
      mat2[i] = mat2data[i];
      for (j = 0; j < 4; j++)
	{  // InvertMatrix transposes the matrix (?)
	mat1[i][j] = matrix->GetElement(i,j);
	}
      }
 
    if (vtkMath::InvertMatrix(mat1,mat2,4) == 0)
      {
      vtkErrorMacro(<< "ExecuteImageInformation: ResliceTransform not \
invertible");
      }

    for (i = 0; i < 4; i++)
      {
      for (j = 0; j < 4; j++)
	{
	matrix->SetElement(i,j,mat2[j][i]);
	}
      }
    }

  // default extent covers entire input extent
  if (this->OutputExtent[0] == INT_MAX)
    {
    for (i = 0; i < 3; i++)
      {
      minOut[i] = FLT_MAX;
      maxOut[i] = -FLT_MAX;
      }
    
    for (i = 0; i < 8; i++)
      {
      inPoint[0] = inOrigin[0] + inExt[i%2]*inSpacing[0];
      inPoint[1] = inOrigin[1] + inExt[2+(i/2)%2]*inSpacing[1];
      inPoint[2] = inOrigin[2] + inExt[4+(i/4)%2]*inSpacing[2];
      inPoint[3] = 1;
      
      matrix->MultiplyPoint(inPoint,outPoint);
      
      for (j = 0; j < 3; j++) 
	{
	tmp = outPoint[j]/outPoint[3];
	if (tmp > maxOut[j])
	  {
	  maxOut[j] = tmp;
	  }
	if (tmp < minOut[j])
	  {
	  minOut[j] = tmp;
	  }
	}
      }
    
    for (i = 0; i < 3; i++)
      {
      this->OutputExtent[2*i] = inExt[2*i];
      this->OutputExtent[2*i+1] = inExt[2*i]+
	int(ceil((maxOut[i]-minOut[i]+1)/this->OutputSpacing[i])) - 1;
      }    
    }

  // default origin places centre of output over centre of input
  if (this->OutputOrigin[0] == FLT_MAX)
    {
    for (i = 0; i < 3; i++)
      {
      inPoint[i] = inOrigin[i] + inSpacing[i]*(inExt[2*i]+inExt[2*i+1])*0.5;
      }
    inPoint[3] = 1;

    matrix->MultiplyPoint(inPoint,outPoint);

    for (i = 0; i < 3; i++)
      {
      this->OutputOrigin[i] = outPoint[i]/outPoint[3] 
	- this->OutputSpacing[i]
	*(this->OutputExtent[2*i]+this->OutputExtent[2*i+1])*0.5;
      }
    }
  
  this->Output->SetWholeExtent(this->OutputExtent);
  this->Output->SetSpacing(this->OutputSpacing);
  this->Output->SetOrigin(this->OutputOrigin);
  matrix->Delete();
}

// Do trilinear interpolation of the input data 'inPtr' of extent 'inExt'
// at the 'point'.  The result is placed at 'outPtr'.  
// If the lookup data is beyond the extent 'inExt', set 'outPtr' to
// the background color 'background'.  
// The number of scalar components in the data is 'numscalars'
template <class T>
static int vtkTrilinearInterpolation(float *point, T *inPtr, T *outPtr,
				     T *background, int numscalars, 
				     int inExt[6], int inInc[3])
{
  int i;

  // the +1/-1 avoids round-to-zero truncation between -1 and 0,
  // and is cheaper than doing floor()
  int floorX = int(point[0]+1)-1;
  int floorY = int(point[1]+1)-1;
  int floorZ = int(point[2]+1)-1;

  float fx = point[0]-floorX;
  float fy = point[1]-floorY;
  float fz = point[2]-floorZ;

  int inIdX = floorX-inExt[0];
  int inIdY = floorY-inExt[2];
  int inIdZ = floorZ-inExt[4];

  // the doInterpX,Y,Z variables are 0 if interpolation
  // does not have to be done in the specified direction,
  // i.e. if the x, y or z lookup indices have no fractional
  // component. 
  int doInterpX = (fx != 0);
  int doInterpY = (fy != 0);
  int doInterpZ = (fz != 0);
  
  if (inIdX < 0 || inIdX+doInterpX > inExt[1]-inExt[0]
      || inIdY < 0 || inIdY+doInterpY > inExt[3]-inExt[2]
      || inIdZ < 0 || inIdZ+doInterpZ > inExt[5]-inExt[4] )
    {// out of bounds: clear to background color 
    if (background)
      {
      for (i = 0; i < numscalars; i++) 
        *outPtr++ = *background++;
      }
    return 0;
    }
  else 
    {// do trilinear interpolation
    int factX = inIdX*inInc[0];
    int factY = inIdY*inInc[1];
    int factZ = inIdZ*inInc[2];

    int factX1 = (inIdX+doInterpX)*inInc[0];
    int factY1 = (inIdY+doInterpY)*inInc[1];
    int factZ1 = (inIdZ+doInterpZ)*inInc[2];
    
    int i000 = factX+factY+factZ;
    int i001 = factX+factY+factZ1;
    int i010 = factX+factY1+factZ;
    int i011 = factX+factY1+factZ1;
    int i100 = factX1+factY+factZ;
    int i101 = factX1+factY+factZ1;
    int i110 = factX1+factY1+factZ;
    int i111 = factX1+factY1+factZ1;

    float rx = 1 - fx;
    float ry = 1 - fy;
    float rz = 1 - fz;
      
    float ryrz = ry*rz;
    float ryfz = ry*fz;
    float fyrz = fy*rz;
    float fyfz = fy*fz;

    for (i = 0; i < numscalars; i++) 
      {
      *outPtr++ = 
	T(rx*(ryrz*inPtr[i000]+ryfz*inPtr[i001]+
	      fyrz*inPtr[i010]+fyfz*inPtr[i011])
	  + fx*(ryrz*inPtr[i100]+ryfz*inPtr[i101]+
		fyrz*inPtr[i110]+fyfz*inPtr[i111]));
      inPtr++;
      }
    return 1;
    }
}			  

static inline int vtkInterpolateWrap(int num, int range)
{
  if ((num %= range) < 0)
    num += range; // required for some % implementations 
  return num;
}

template <class T>
static int vtkTrilinearInterpolationWrap(float *point, T *inPtr, T *outPtr,
					 T *background, int numscalars, 
					 int inExt[6], int inInc[3])
{
  int i;

  int floorX = int(point[0]+1)-1;
  int floorY = int(point[1]+1)-1;
  int floorZ = int(point[2]+1)-1;

  float fx = point[0]-floorX;
  float fy = point[1]-floorY;
  float fz = point[2]-floorZ;

  // this corrects for differences between int() and floor()
  if (fx < 0)
    fx = point[0] - (--floorX);
  if (fy < 0)
    fy = point[1] - (--floorY);
  if (fz < 0)
    fz = point[2] - (--floorZ);

  int inIdX = floorX-inExt[0];
  int inIdY = floorY-inExt[2];
  int inIdZ = floorZ-inExt[4];

  int inExtX = inExt[1]-inExt[0]+1;
  int inExtY = inExt[3]-inExt[2]+1;
  int inExtZ = inExt[5]-inExt[4]+1;

  int factX = vtkInterpolateWrap(inIdX,inExtX)*inInc[0];
  int factY = vtkInterpolateWrap(inIdY,inExtY)*inInc[1];
  int factZ = vtkInterpolateWrap(inIdZ,inExtZ)*inInc[2];

  int factX1 = vtkInterpolateWrap(inIdX+1,inExtX)*inInc[0];
  int factY1 = vtkInterpolateWrap(inIdY+1,inExtY)*inInc[1];
  int factZ1 = vtkInterpolateWrap(inIdZ+1,inExtZ)*inInc[2];

  int i000 = factX+factY+factZ;
  int i001 = factX+factY+factZ1;
  int i010 = factX+factY1+factZ;
  int i011 = factX+factY1+factZ1;
  int i100 = factX1+factY+factZ;
  int i101 = factX1+factY+factZ1;
  int i110 = factX1+factY1+factZ;
  int i111 = factX1+factY1+factZ1;

  float rx = 1 - fx;
  float ry = 1 - fy;
  float rz = 1 - fz;
  
  float ryrz = ry*rz;
  float ryfz = ry*fz;
  float fyrz = fy*rz;
  float fyfz = fy*fz;

  for (i = 0; i < numscalars; i++) 
    {
    *outPtr++ = 
      T(rx*(ryrz*inPtr[i000]+ryfz*inPtr[i001]+
	    fyrz*inPtr[i010]+fyfz*inPtr[i011])
	+ fx*(ryrz*inPtr[i100]+ryfz*inPtr[i101]+
	      fyrz*inPtr[i110]+fyfz*inPtr[i111]));
    inPtr++;
    }
  return 1;
}			  

// Do nearest-neighbor interpolation of the input data 'inPtr' of extent 
// 'inExt' at the 'point'.  The result is placed at 'outPtr'.  
// If the lookup data is beyond the extent 'inExt', set 'outPtr' to
// the background color 'background'.  
// The number of scalar components in the data is 'numscalars'

template <class T>
static int vtkNearestNeighborInterpolation(float *point, T *inPtr, T *outPtr,
                                           T *background, int numscalars, 
                                           int inExt[6], int inInc[3])
{
  int i;
  int inIdX = int(point[0]+1.5)-inExt[0]-1;
  int inIdY = int(point[1]+1.5)-inExt[2]-1;
  int inIdZ = int(point[2]+1.5)-inExt[4]-1;
  
  if (inIdX < 0 || inIdX > inExt[1]-inExt[0]
      || inIdY < 0 || inIdY > inExt[3]-inExt[2]
      || inIdZ < 0 || inIdZ > inExt[5]-inExt[4] )
    {
    if (background)
      {
      for (i = 0; i < numscalars; i++)
	*outPtr++ = *background++;
      }
    return 0;
    }
  else 
    {
    inPtr += inIdX*inInc[0]+inIdY*inInc[1]+inIdZ*inInc[2];
    for (i = 0; i < numscalars; i++)
      {
      *outPtr++ = *inPtr++;
      }
    return 1;
    }
} 

template <class T>
static int vtkNearestNeighborInterpolationWrap(float *point, T *inPtr, 
					       T *outPtr,
					       T *background, int numscalars, 
					       int inExt[6], int inInc[3])
{
  int i;

  // round-to-zero vs. round-to-neg-infinity strikes again
  float vX = point[0]+1.5;
  float vY = point[1]+1.5;
  float vZ = point[2]+1.5;

  int floorX = int(vX)-1;
  int floorY = int(vY)-1;
  int floorZ = int(vZ)-1;

  if (vX < floorX+1) floorX--;
  if (vY < floorY+1) floorY--;
  if (vZ < floorZ+1) floorZ--;

  int inIdX = vtkInterpolateWrap(floorX-inExt[0],
				 inExt[1]-inExt[0]+1);
  int inIdY = vtkInterpolateWrap(floorY-inExt[2],
				 inExt[3]-inExt[2]+1);
  int inIdZ = vtkInterpolateWrap(floorZ-inExt[4],
				 inExt[5]-inExt[4]+1);
  
  inPtr += inIdX*inInc[0]+inIdY*inInc[1]+inIdZ*inInc[2];
  for (i = 0; i < numscalars; i++)
    {
    *outPtr++ = *inPtr++;
    }
  return 1; 
} 

// clamping functions for each type

static inline void vtkResliceClamp(double val, unsigned char& clamp)
{
  if (val < VTK_UNSIGNED_CHAR_MIN)
    { 
    val = VTK_UNSIGNED_CHAR_MIN;
    }
  if (val > VTK_UNSIGNED_CHAR_MAX)
    { 
    val = VTK_UNSIGNED_CHAR_MAX;
    }
  clamp = (unsigned char)(val);
}

static inline void vtkResliceClamp(double val, short& clamp)
{
  if (val < VTK_SHORT_MIN)
    { 
    val = VTK_SHORT_MIN;
    }
  if (val > VTK_SHORT_MAX)
    { 
    val = VTK_SHORT_MAX;
    }
  clamp = short(val);
}

static inline void vtkResliceClamp(double val, unsigned short& clamp)
{
  if (val < VTK_UNSIGNED_SHORT_MIN)
    { 
    val = VTK_UNSIGNED_SHORT_MIN;
    }
  if (val > VTK_UNSIGNED_SHORT_MAX)
    { 
    val = VTK_UNSIGNED_SHORT_MAX;
    }
  clamp = (unsigned short)(val);
}

static inline void vtkResliceClamp(double val, int& clamp)
{
  if (val < VTK_INT_MIN) 
    {
    val = VTK_INT_MIN;
    }
  if (val > VTK_INT_MAX) 
    {
    val = VTK_INT_MAX;
    }
  clamp = int(val);
}

static inline void vtkResliceClamp(double val, float& clamp)
{
  if (val < VTK_FLOAT_MIN)
    { 
    val = VTK_FLOAT_MIN;
    }
  if (val > VTK_FLOAT_MAX) 
    {
    val = VTK_FLOAT_MAX;
    }
  clamp = float(val);
}

// Do tricubic interpolation of the input data 'inPtr' of extent 'inExt'
// at the 'point'.  The result is placed at 'outPtr'.  
// If any of the lookup data is beyond the extent 'inExt', try the
// trilinear interpolant.
// The number of scalar components in the data is 'numscalars'

// set up the lookup indices and the interpolation coefficients

void vtkImageResliceSetInterpCoeffs(float F[4],int *l, int *m, float f, 
		     int interpMode)
{   
  float fp1,fm1,fm2;

  switch (interpMode)
    {
    case 7:     // cubic interpolation
      *l = 0; *m = 4; 
      fp1 = f+1; fm1 = f-1; fm2 = fm1-1;
      F[0] = -f*fm1*fm2/6;
      F[1] = fp1*fm1*fm2/2;
      F[2] = -fp1*f*fm2/2;
      F[3] = fp1*f*fm1/6;
      break;
    case 0:     // no interpolation
    case 2:
    case 4:
    case 6:
      *l = 1; *m = 2; 
      F[1] = 1;
      break;    // linear interpolation
    case 1:
      *l = 1; *m = 3;
      F[1] = 1-f;
      F[2] = f;
      break;
    case 3:     // quadratic interpolation
      *l = 1; *m = 4; 
      fm1 = f-1; fm2 = fm1-1;
      F[1] = fm1*fm2/2;
      F[2] = -f*fm2;
      F[3] = f*fm1/2;
      break;
    case 5:     // quadratic interpolation
      *l = 0; *m = 3; 
      fp1 = f+1; fm1 = f-1; 
      F[0] = f*fm1/2;
      F[1] = -fp1*fm1;
      F[2] = fp1*f/2;
      break;
    }
}

template <class T>
static int vtkTricubicInterpolation(float *point, T *inPtr, T *outPtr,
				    T *background, int numscalars, 
				    int inExt[6], int inInc[3])
{
  int i;
  int factX[4],factY[4],factZ[4];

  // the +1/-1 avoids round-to-zero truncation between -1 and 0,
  // and is cheaper than doing floor()
  int floorX = int(point[0]+1)-1;
  int floorY = int(point[1]+1)-1;
  int floorZ = int(point[2]+1)-1;

  float fx = point[0]-floorX;
  float fy = point[1]-floorY;
  float fz = point[2]-floorZ;

  int inIdX = floorX-inExt[0];
  int inIdY = floorY-inExt[2];
  int inIdZ = floorZ-inExt[4];

  // the doInterpX,Y,Z variables are 0 if interpolation
  // does not have to be done in the specified direction,
  // i.e. if the x, y or z lookup indices have no fractional
  // component.   
  int doInterpX = (fx != 0);
  int doInterpY = (fy != 0);
  int doInterpZ = (fz != 0);

  // check whether we can do cubic interpolation, quadratic, linear, or none
  // in each of the three directions
  if (inIdX < 0 || inIdX+doInterpX > inExt[1]-inExt[0] ||
      inIdY < 0 || inIdY+doInterpY > inExt[3]-inExt[2] ||
      inIdZ < 0 || inIdZ+doInterpZ > inExt[5]-inExt[4])
    {// out of bounds: clear to background color
    if (background)
      {
      for (i = 0; i < numscalars; i++) 
	*outPtr++ = *background++;
      }
    return 0;
    }
  else 
    {// do tricubic interpolation
    float fX[4],fY[4],fZ[4];
    double vX,vY,vZ,val;
    T *inPtr1, *inPtr2;
    int j,k,l,jl,jm,kl,km,ll,lm;
    
    for (i = 0; i < 4; i++)
    {
      factX[i] = (inIdX-1+i)*inInc[0];
      factY[i] = (inIdY-1+i)*inInc[1];
      factZ[i] = (inIdZ-1+i)*inInc[2];
    }

    // depending on whether we are at the edge of the 
    // input extent, choose the appropriate interpolation
    // method to use

    int interpModeX = ((inIdX > 0) << 2) + 
                      ((inIdX+2 <= inExt[1]-inExt[0]) << 1) +
                      doInterpX;
    int interpModeY = ((inIdY > 0) << 2) + 
                      ((inIdY+2 <= inExt[3]-inExt[2]) << 1) +
                      doInterpY;
    int interpModeZ = ((inIdZ > 0) << 2) + 
	              ((inIdZ+2 <= inExt[5]-inExt[4]) << 1) +
		      doInterpZ;

    vtkImageResliceSetInterpCoeffs(fX,&ll,&lm,fx,interpModeX);
    vtkImageResliceSetInterpCoeffs(fY,&kl,&km,fy,interpModeY);
    vtkImageResliceSetInterpCoeffs(fZ,&jl,&jm,fz,interpModeZ);

    // Finally, here is the tricubic interpolation
    // (or cubic-cubic-linear, or cubic-nearest-cubic, etc)
    for (i = 0; i < numscalars; i++)
      {
      val = 0;
      for (j = jl; j < jm; j++)
	{
	inPtr1 = inPtr + factZ[j];
	vZ = 0;
	for (k = kl; k < km; k++)
	  {
	  inPtr2 = inPtr1 + factY[k];
	  vY = 0;
	  for (l = ll; l < lm; l++)
	    {
	    vY += *(inPtr2+factX[l]) * fX[l]; 
	    }
	  vZ += vY*fY[k]; 
	  }
	val += vZ*fZ[j];
	}
      vtkResliceClamp(val,*outPtr++); // clamp to limits of type
      inPtr++;
      }
    return 1;
    }
}		  

template <class T>
static int vtkTricubicInterpolationWrap(float *point, T *inPtr, T *outPtr,
					T *background, int numscalars, 
					int inExt[6], int inInc[3])
{
  int i;
  int factX[4],factY[4],factZ[4];

  int floorX = int(point[0]+1)-1;
  int floorY = int(point[1]+1)-1;
  int floorZ = int(point[2]+1)-1;

  float fx = point[0]-floorX;
  float fy = point[1]-floorY;
  float fz = point[2]-floorZ;

  // this corrects for differences between int() and floor()
  if (fx < 0)
    fx = point[0] - (--floorX);
  if (fy < 0)
    fy = point[1] - (--floorY);
  if (fz < 0)
    fz = point[2] - (--floorZ);

  int inIdX = floorX-inExt[0];
  int inIdY = floorY-inExt[2];
  int inIdZ = floorZ-inExt[4];

  int inExtX = inExt[1]-inExt[0]+1;
  int inExtY = inExt[3]-inExt[2]+1;
  int inExtZ = inExt[5]-inExt[4]+1;

  float fX[4],fY[4],fZ[4];
  double vX,vY,vZ,val;
  T *inPtr1, *inPtr2;
  int j,k,l;

  for (i = 0; i < 4; i++)
    {
    factX[i] = vtkInterpolateWrap(inIdX-1+i,inExtX)*inInc[0];
    factY[i] = vtkInterpolateWrap(inIdY-1+i,inExtY)*inInc[1];
    factZ[i] = vtkInterpolateWrap(inIdZ-1+i,inExtZ)*inInc[2];
    }

  vtkImageResliceSetInterpCoeffs(fX,&i,&i,fx,7);
  vtkImageResliceSetInterpCoeffs(fY,&i,&i,fy,7);
  vtkImageResliceSetInterpCoeffs(fZ,&i,&i,fz,7);

  // Finally, here is the tricubic interpolation
  for (i = 0; i < numscalars; i++)
    {
    val = 0;
    for (j = 0; j < 4; j++)
      {
      inPtr1 = inPtr + factZ[j];
      vZ = 0;
      for (k = 0; k < 4; k++)
	{
	inPtr2 = inPtr1 + factY[k];
	vY = 0;
	for (l = 0; l < 4; l++)
	  {
	  vY += *(inPtr2+factX[l]) * fX[l]; 
	  }
	vZ += vY*fY[k]; 
	}
      val += vZ*fZ[j];
      }
    vtkResliceClamp(val,*outPtr++); // clamp to limits of type
    inPtr++;
    }
  return 1;
}		  

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// (this one function is pretty much the be-all and end-all of the
// filter)
template <class T>
static void vtkImageResliceExecute(vtkImageReslice *self,
				   vtkImageData *inData, T *inPtr,
				   vtkImageData *outData, T *outPtr,
				   int outExt[6], int id)
{
  int i, numscalars;
  int idX, idY, idZ;
  int outIncX, outIncY, outIncZ;
  int inIdX, inIdY, inIdZ;
  int inExt[6], inInc[3];
  unsigned long count = 0;
  unsigned long target;
  float inPoint[4],outPoint[4];
  T *background;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  int (*interpolate)(float *point, T *inPtr, T *outPtr,
		     T *background, int numscalars, 
		     int inExt[6], int inInc[3]);
  
  // find maximum input range
  inData->GetExtent(inExt);
  
  target = (unsigned long)
    ((outExt[5]-outExt[4]+1)*(outExt[3]-outExt[2]+1)/50.0);
  target++;
  
  // Get Increments to march through data 
  inData->GetIncrements(inInc);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  numscalars = inData->GetNumberOfScalarComponents();
  
  // change transform matrix so that instead of taking 
  // input coords -> output coords it takes output indices -> input indices
  self->ComputeIndexMatrix(matrix);
  
  // Color for area outside of input volume extent  
  background = new T[numscalars];
  for (i = 0; i < numscalars; i++)
    background[i] = T(self->GetBackgroundLevel());

  // Set interpolation method
  if (self->GetWrap())
    {
    switch (self->GetInterpolate() * self->GetInterpolationMode())
      {
      case VTK_RESLICE_NEAREST:
	interpolate = &vtkNearestNeighborInterpolationWrap;
	break;
      case VTK_RESLICE_LINEAR:
	interpolate = &vtkTrilinearInterpolationWrap;
	break;
      case VTK_RESLICE_CUBIC:
	interpolate = &vtkTricubicInterpolationWrap;
	break;
      }
    }
  else
    {
    switch (self->GetInterpolate() * self->GetInterpolationMode())
      {
      case VTK_RESLICE_NEAREST:
	interpolate = &vtkNearestNeighborInterpolation;
	break;
      case VTK_RESLICE_LINEAR:
	interpolate = &vtkTrilinearInterpolation;
	break;
      case VTK_RESLICE_CUBIC:
	interpolate = &vtkTricubicInterpolation;
	break;
      }
    }    

  // Loop through ouput pixels
  for (idZ = outExt[4]; idZ <= outExt[5]; idZ++)
    {
    for (idY = outExt[2]; idY <= outExt[3]; idY++)
      {
      if (!id) 
	{
	if (!(count%target)) 
	  {
	  self->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      
      for (idX = outExt[0]; idX <= outExt[1]; idX++)
	{
	outPoint[0] = idX;
	outPoint[1] = idY;
	outPoint[2] = idZ;
	outPoint[3] = 1;

	matrix->MultiplyPoint(outPoint,inPoint); // apply transform

	inPoint[0] /= inPoint[3]; // deal with w if the transform
	inPoint[1] /= inPoint[3]; //   was a perspective transform
	inPoint[2] /= inPoint[3];
	inPoint[3] = 1;
	
	interpolate(inPoint, inPtr, outPtr, background, 
		    numscalars, inExt, inInc);

	outPtr += numscalars; 
	}
      outPtr += outIncY;
      }
    outPtr += outIncZ;
    }
  delete [] background;
  matrix->Delete();
}

//----------------------------------------------------------------------------
// The remainder of this file is the 'optimized' version of the code.

//----------------------------------------------------------------------------
static void ComputeRequiredInputUpdateExtentOptimized(vtkImageReslice *self,
						      int inExt[6], 
						      int outExt[6])
{
  int i,j,k;
  int idX,idY,idZ;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  double *xAxis, *yAxis, *zAxis, *origin;
  double point[4],w;

  // convert matrix from world coordinates to pixel indices
  self->ComputeIndexMatrix(matrix);
  matrix->Transpose();
  xAxis = matrix->Element[0];
  yAxis = matrix->Element[1];
  zAxis = matrix->Element[2];
  origin = matrix->Element[3];

  for (i = 0; i < 3; i++)
    {
    inExt[2*i] = INT_MAX;
    inExt[2*i+1] = INT_MIN;
    }
  
  for (i = 0; i < 8; i++)
    {
    // calculate transform using method in vtkImageResliceExecute
    idX = outExt[i%2];
    idY = outExt[2+(i/2)%2];
    idZ = outExt[4+(i/4)%2];
    
    for (j = 0; j < 4; j++) 
      {
      point[j] = origin[j] + idZ*zAxis[j];
      point[j] = point[j] + idY*yAxis[j];
      }
    
    w = point[3] + idX*xAxis[3];
    
    if (self->GetInterpolate() 
	* self->GetInterpolationMode() != VTK_RESLICE_NEAREST)
      {
      for (j = 0; j < 3; j++) 
	{
	int extra = (self->GetInterpolationMode() == VTK_RESLICE_CUBIC); 
	k = int(floor((point[j]+idX*xAxis[j])/w))-extra;
	if (k < inExt[2*j]) inExt[2*j] = k; 
	k = int(ceil((point[j]+idX*xAxis[j])/w))+extra;
	if (k > inExt[2*j+1]) inExt[2*j+1] = k;
	}
      }
    else
      {
      for (j = 0; j < 3; j++) 
	{
	k = int(floor((point[j]+idX*xAxis[j])/w + 0.5));
	if (k < inExt[2*j]) inExt[2*j] = k; 
	if (k > inExt[2*j+1]) inExt[2*j+1] = k;
	}
      }
    }
  matrix->Delete();
}

//----------------------------------------------------------------------------
// helper functions for vtkOptimizedExecute()

// find approximate intersection of line with the plane x = x_min,
// y = y_min, or z = z_min (lower limit of data extent) 

static int intersectionLow(double *point, double *axis, int *sign,
			   int *limit, int ai, int *outExt)
{
  // approximate value of r
  int r;
  double rd = (limit[ai]*point[3]-point[ai])
    /(axis[ai]-limit[ai]*axis[3]) + 0.5;
   
  if (rd < outExt[2*ai]) 
    r = outExt[2*ai];
  else if (rd > outExt[2*ai+1])
    r = outExt[2*ai+1];
  else
    r = int(rd);
  
  // move back and forth to find the point just inside the extent
  while (int( (point[ai]+r*axis[ai])/double(point[3]+r*axis[3]) + 1.5 )-1 
	 < limit[ai])
    r += sign[ai];

  while (int( (point[ai]+(r-sign[ai])*axis[ai])
	      /double(point[3]+(r-sign[ai])*axis[3]) + 1.5 )-1 
	 >= limit[ai])
    r -= sign[ai];

  return r;
}

// same as above, but for x = x_max
static int intersectionHigh(double *point, double *axis, int *sign, 
			    int *limit, int ai, int *outExt)
{
  int r;
  double rd = (limit[ai]*point[3]-point[ai])
      /(axis[ai]-limit[ai]*axis[3]) + 0.5; 
    
  if (rd < outExt[2*ai]) 
    r = outExt[2*ai];
  else if (rd > outExt[2*ai+1])
    r = outExt[2*ai+1];
  else
    r = int(rd);
  
  while (int( (point[ai]+r*axis[ai])/double(point[3]+r*axis[3]) + 1.5 )-1 
	 > limit[ai])
    r -= sign[ai];

  while (int( (point[ai]+(r+sign[ai])*axis[ai])
	      /double(point[3]+(r+sign[ai])*axis[3]) + 1.5 )-1 
	 <= limit[ai])
    r += sign[ai];

  return r;
}

static int isBounded(double *point, double *xAxis, int *inMin, 
		     int *inMax, int ai, int r)
{
  int bi = ai+1; 
  int ci = ai+2;
  if (bi > 2) bi -= 3;  // coordinate index must be 0, 1 or 2
  if (ci > 2) ci -= 3;
  double w = point[3]+r*xAxis[3];
  int bp = int((point[bi]+r*xAxis[bi])/w + 1.5)-1;
  int cp = int((point[ci]+r*xAxis[ci])/w + 1.5)-1;
  
  return (bp >= inMin[bi] && bp <= inMax[bi] &&
	  cp >= inMin[ci] && cp <= inMax[ci]);
}

// this huge mess finds out where the current output raster
// line intersects the input volume 
int vtkImageReslice::FindExtent(int& r1, int& r2, double *point, 
                                double *xAxis, 
		      int *inMin, int *inMax, int *outExt)
{
  int i, ix, iy, iz;
  int sign[3];
  int indx1[4],indx2[4];
  double w1,w2;

  // find signs of components of x axis 
  // (this is complicated due to the homogenous coordinate)
  for (i = 0; i < 3; i++)
    {
    if (point[i]/point[3] <= (point[i]+xAxis[i])/(point[3]+xAxis[3]))
      {
      sign[i] = 1;
      }
    else 
      {
      sign[i] = -1;
      }
    } 
  
  // order components of xAxis from largest to smallest
  
  ix = 0;
  for (i = 1; i < 3; i++)
    {
    if (xAxis[i]*xAxis[i] > xAxis[ix]*xAxis[ix])
      {
      ix = i;
      }
    }
  
  iy = ((ix > 1) ? ix-2 : ix+1);
  iz = ((ix > 0) ? ix-1 : ix+2);

  if (xAxis[iz]*xAxis[iz] > xAxis[iy]*xAxis[iy])
    {
    i = iy;
    iy = iz;
    iz = i;
    }

  r1 = intersectionLow(point,xAxis,sign,inMin,ix,outExt);
  r2 = intersectionHigh(point,xAxis,sign,inMax,ix,outExt);
  
  // find points of intersections
  // first, find w
  w1 = point[3]+r1*xAxis[3];
  w2 = point[3]+r2*xAxis[3];
  
  for (i = 0; i < 3; i++)
    {
      indx1[i] = int((point[i]+r1*xAxis[i])/w1+1.5)-1;
      indx2[i] = int((point[i]+r2*xAxis[i])/w2+1.5)-1;
    }
  if (isBounded(point,xAxis,inMin,inMax,ix,r1))
    { // passed through x face, check opposing face
    if (isBounded(point,xAxis,inMin,inMax,ix,r2))
      {
      return sign[ix];
      }
    
    if (indx2[iy] < inMin[iy])
      { // check y face
      r2 = intersectionLow(point,xAxis,sign,inMin,iy,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iy,r2))
	{
	return sign[ix];
	}
      }
    else if (indx2[iy] > inMax[iy])
      { // check other y face
      r2 = intersectionHigh(point,xAxis,sign,inMax,iy,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iy,r2))
	{
	return sign[ix];
	}
      }
    
    if (indx2[iz] < inMin[iz])
      { // check z face
      r2 = intersectionLow(point,xAxis,sign,inMin,iz,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iz,r2))
	{
	return sign[ix];
	}
      }
    else if (indx2[iz] > inMax[iz])
      { // check other z face
      r2 = intersectionHigh(point,xAxis,sign,inMax,iz,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iz,r2))
	{
	return sign[ix];
	}
      }
    }
  
  if (isBounded(point,xAxis,inMin,inMax,ix,r2))
    { // passed through the opposite x face
    if (indx1[iy] < inMin[iy])
	{ // check y face
	r1 = intersectionLow(point,xAxis,sign,inMin,iy,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iy,r1))
	  {
	  return sign[ix];
	  }
	}
    else if (indx1[iy] > inMax[iy])
      { // check other y face
      r1 = intersectionHigh(point,xAxis,sign,inMax,iy,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iy,r1))
	{
	return sign[ix];
	}
      }
    
    if (indx1[iz] < inMin[iz])
      { // check z face
      r1 = intersectionLow(point,xAxis,sign,inMin,iz,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iz,r1))
	{
	return sign[ix];
	}
      }
    else if (indx1[iz] > inMax[iz])
      { // check other z face
      r1 = intersectionHigh(point,xAxis,sign,inMax,iz,outExt);
      if (isBounded(point,xAxis,inMin,inMax,iz,r1))
	{
	return sign[ix];
	}
      }
    }
  
  if ((indx1[iy] >= inMin[iy] && indx2[iy] < inMin[iy]) ||
      (indx1[iy] < inMin[iy] && indx2[iy] >= inMin[iy]))
    { // line might pass through bottom face
    r1 = intersectionLow(point,xAxis,sign,inMin,iy,outExt);
    if (isBounded(point,xAxis,inMin,inMax,iy,r1))
      {
      if ((indx1[iy] <= inMax[iy] && indx2[iy] > inMax[iy]) ||
	  (indx1[iy] > inMax[iy] && indx2[iy] <= inMax[iy]))
	{ // line might pass through top face
	r2 = intersectionHigh(point,xAxis,sign,inMax,iy,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iy,r2))
	  {
	  return sign[iy];
	  }
	}
      
      if (indx1[iz] < inMin[iz] && indx2[iy] < inMin[iy] ||
	  indx2[iz] < inMin[iz] && indx1[iy] < inMin[iy])
	{ // line might pass through in-to-screen face
	r2 = intersectionLow(point,xAxis,sign,inMin,iz,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iz,r2))
	  {
	  return sign[iy];
	  }
	}
      else if (indx1[iz] > inMax[iz] && indx2[iy] < inMin[iy] ||
	       indx2[iz] > inMax[iz] && indx1[iy] < inMin[iy])
	{ // line might pass through out-of-screen face
	r2 = intersectionHigh(point,xAxis,sign,inMax,iz,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iz,r2))
	  {
	  return sign[iy];
	  }
	} 
      }
    }
  
  if ((indx1[iy] <= inMax[iy] && indx2[iy] > inMax[iy]) ||
      (indx1[iy] > inMax[iy] && indx2[iy] <= inMax[iy]))
    { // line might pass through top face
    r2 = intersectionHigh(point,xAxis,sign,inMax,iy,outExt);
    if (isBounded(point,xAxis,inMin,inMax,iy,r2))
      {
      if (indx1[iz] < inMin[iz] && indx2[iy] > inMax[iy] ||
	  indx2[iz] < inMin[iz] && indx1[iy] > inMax[iy])
	{ // line might pass through in-to-screen face
	r1 = intersectionLow(point,xAxis,sign,inMin,iz,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iz,r1))
	  {
	  return sign[iy];
	  }
	}
      else if (indx1[iz] > inMax[iz] && indx2[iy] > inMax[iy] || 
	       indx2[iz] > inMax[iz] && indx1[iy] > inMax[iy])
	{ // line might pass through out-of-screen face
	r1 = intersectionHigh(point,xAxis,sign,inMax,iz,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iz,r1))
	  {
	  return sign[iy];
	  }
	}
      } 
    }
  
  if ((indx1[iz] >= inMin[iz] && indx2[iz] < inMin[iz]) ||
      (indx1[iz] < inMin[iz] && indx2[iz] >= inMin[iz]))
    { // line might pass through in-to-screen face
    r1 = intersectionLow(point,xAxis,sign,inMin,iz,outExt);
    if (isBounded(point,xAxis,inMin,inMax,iz,r1))
      {
      if (indx1[iz] > inMax[iz] || indx2[iz] > inMax[iz])
	{ // line might pass through out-of-screen face
	r2 = intersectionHigh(point,xAxis,sign,inMax,iz,outExt);
	if (isBounded(point,xAxis,inMin,inMax,iz,r2))
	  {
	  return sign[iz];
	  }
	}
      }
    }
  
  r1 = r2 = -1;
  return 1;
}

// The vtkOptimizedExecute() function uses an optimization which
// is conceptually simple, but complicated to implement.

// In the un-optimized version, each output voxel
// is converted into a set of look-up indices for the input data;
// then, the indices are checked to ensure they lie within the
// input data extent.

// In the optimized version below, the check is done in reverse:
// it is first determined which output voxels map to look-up indices
// within the input data extent.  Then, further calculations are
// done only for those voxels.  This means that 1) minimal work
// is done for voxels which map to regions outside fo the input
// extent (they are just set to the background color) and 2)
// the inner loops of the look-up and interpolation are
// tightened relative to the un-uptimized version. 

template <class T>
static void vtkOptimizedExecute(vtkImageReslice *self,
				vtkImageData *inData, T *inPtr,
				vtkImageData *outData, T *outPtr,
				int outExt[6], int id)
{
  int i, numscalars;
  int idX, idY, idZ;
  int outIncX, outIncY, outIncZ;
  int inIdX, inIdY, inIdZ;
  int inExt[6];
  int inMax[3], inMin[3];
  int inInc[3];
  unsigned long count = 0;
  unsigned long target;
  int r1,r2;
  double inPoint0[4];
  double inPoint1[4];
  float inPoint[4];
  double *xAxis, *yAxis, *zAxis, *origin;
  T *background, *inPtr1;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  double w;
  int (*interpolate)(float *point, T *inPtr, T *outPtr,
		     T *background, int numscalars, 
		     int inExt[6], int inInc[3]);

  // Set interpolation method
  if (self->GetWrap())
    {
    switch (self->GetInterpolate() * self->GetInterpolationMode())
      {
      case VTK_RESLICE_NEAREST:
	interpolate = &vtkNearestNeighborInterpolationWrap;
	break;
      case VTK_RESLICE_LINEAR:
	interpolate = &vtkTrilinearInterpolationWrap;
	break;
      case VTK_RESLICE_CUBIC:
	interpolate = &vtkTricubicInterpolationWrap;
	break;
      }
    }
  else
    {
    switch (self->GetInterpolate() * self->GetInterpolationMode())
      {
      case VTK_RESLICE_NEAREST:
	interpolate = &vtkNearestNeighborInterpolation;
	break;
      case VTK_RESLICE_LINEAR:
	interpolate = &vtkTrilinearInterpolation;
	break;
      case VTK_RESLICE_CUBIC:
	interpolate = &vtkTricubicInterpolation;
	break;
      }
    }    

  // find maximum input range
  self->GetInput()->GetWholeExtent(inExt);

  for (i = 0; i < 3; i++)
    {
    inMin[i] = inExt[2*i];
    inMax[i] = inExt[2*i+1];
    }
  
  target = (unsigned long)
    ((outExt[5]-outExt[4]+1)*(outExt[3]-outExt[2]+1)/50.0);
  target++;
  
  // Get Increments to march through data 
  inData->GetIncrements(inInc);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  numscalars = inData->GetNumberOfScalarComponents();

  // set up background levels
  background = new T[numscalars];
  for (i = 0; i < numscalars; i++)
    background[i] = T(self->GetBackgroundLevel());

  // change transform matrix so that instead of taking 
  // input coords -> output coords it takes output indices -> input indices
  self->ComputeIndexMatrix(matrix);
  
  // break matrix into a set of axes plus an origin
  // (this allows us to calculate the transform Incrementally)
  matrix->Transpose();
  xAxis = matrix->Element[0];
  yAxis = matrix->Element[1];
  zAxis = matrix->Element[2];
  origin = matrix->Element[3];
  
  // Loop through output pixels
  for (idZ = outExt[4]; idZ <= outExt[5]; idZ++)
    {
    inPoint0[0] = origin[0]+idZ*zAxis[0]; // incremental transform
    inPoint0[1] = origin[1]+idZ*zAxis[1]; 
    inPoint0[2] = origin[2]+idZ*zAxis[2]; 
    inPoint0[3] = origin[3]+idZ*zAxis[3]; 
    
    for (idY = outExt[2]; idY <= outExt[3]; idY++)
      {
      inPoint1[0] = inPoint0[0]+idY*yAxis[0]; // incremental transform
      inPoint1[1] = inPoint0[1]+idY*yAxis[1];
      inPoint1[2] = inPoint0[2]+idY*yAxis[2];
      inPoint1[3] = inPoint0[3]+idY*yAxis[3];
      
      if (!id) 
	{
	if (!(count%target)) 
	  {
	  self->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      
      if (self->GetWrap())
	{ // wrap-pad like behaviour
	for (idX = outExt[0]; idX <= outExt[1]; idX++)
	  {
	  w = inPoint1[3]+idX*xAxis[3];
	  inPoint[0] = (inPoint1[0]+idX*xAxis[0])/w;
	  inPoint[1] = (inPoint1[1]+idX*xAxis[1])/w;
	  inPoint[2] = (inPoint1[2]+idX*xAxis[2])/w;
	  inPoint[3] = 1;

	  interpolate(inPoint, inPtr, outPtr, background, 
		      numscalars, inExt, inInc);
	  outPtr += numscalars;
	  }
	}
      else
	{
	// find intersections of x raster line with the input extent
	if (self->FindExtent(r1,r2,inPoint1,xAxis,inMin,inMax,outExt) < 0)
	  {
	  i = r1;
	  r1 = r2;
	  r2 = i;
	  }

	// bound r1,r2 within reasonable limits
	if (r1 < outExt[0]) r1 = outExt[0];
	if (r2 > outExt[1]) r2 = outExt[1];
	if (r1 > r2) r1 = r2+1;
	if (r2 < r1) r2 = r1-1;

	// clear pixels to left of input extent
	if (numscalars == 1) // optimize for single scalar
	  {
	  for (idX = outExt[0]; idX < r1; idX++) 
	    *outPtr++ = background[0];
	  }
	else             // multiple scalars
	  {
	  for (idX = outExt[0]; idX < r1; idX++)
	    for (i = 0; i < numscalars; i++)
	      *outPtr++ = background[i];
	  }
	
	if (self->GetInterpolate() 
	    * self->GetInterpolationMode() != VTK_RESLICE_NEAREST)
	  { // Trilinear or tricubic
	  for (idX = r1; idX <= r2; idX++)
	    {
	    w = inPoint1[3]+idX*xAxis[3];
	    inPoint[0] = (inPoint1[0]+idX*xAxis[0])/w;
	    inPoint[1] = (inPoint1[1]+idX*xAxis[1])/w;
	    inPoint[2] = (inPoint1[2]+idX*xAxis[2])/w;
	    inPoint[3] = 1;
	    
	    interpolate(inPoint, inPtr, outPtr, background, 
			numscalars, inExt, inInc);
	    outPtr += numscalars;
	    }
	  }
	else
	  {  // Nearest-Neighbor, no extent checks
	  for (idX = r1; idX <= r2; idX++)
	    {
	    w = inPoint1[3]+idX*xAxis[3]; // don't forget w!  
	    // the +1.5/-1 thing avoids int() vs. floor() difference 
	    inIdX = int((inPoint1[0]+idX*xAxis[0])/w+1.5)-inExt[0]-1;
	    inIdY = int((inPoint1[1]+idX*xAxis[1])/w+1.5)-inExt[2]-1;
	    inIdZ = int((inPoint1[2]+idX*xAxis[2])/w+1.5)-inExt[4]-1;
	    
	    inPtr1 = inPtr+inIdX*inInc[0]+inIdY*inInc[1]
	      +inIdZ*inInc[2];

	    for (i = 0; i < numscalars; i++)
	      {
	      *outPtr++ = *inPtr1++;
	      }
	    }
	  }
	
  
	// clear pixels to right of input extent
	if (numscalars == 1) // optimize for single scalar
	  {
	  for (idX = r2+1; idX <= outExt[1]; idX++) 
	    *outPtr++ = background[0];
	  }
	else // multiple scalars
	  {
	  for (idX = r2+1; idX <= outExt[1]; idX++)
	    for (i = 0; i < numscalars; i++)
	      *outPtr++ = background[i];
	  }
	}

      outPtr += outIncY;
      }
    outPtr += outIncZ;
    }
  matrix->Delete();
  delete [] background;
}

//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageReslice::ThreadedExecute(vtkImageData *inData, 
				      vtkImageData *outData,
				      int outExt[6], int id)
{
  void *inPtr = inData->GetScalarPointerForExtent(inData->GetExtent());
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
  << ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
            << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  if (this->Optimization)
    {
    switch (inData->GetScalarType())
      {
      case VTK_FLOAT:
	vtkOptimizedExecute(this, inData, (float *)(inPtr), 
			       outData, (float *)(outPtr),outExt, id);
	break;
      case VTK_INT:
	vtkOptimizedExecute(this, inData, (int *)(inPtr), 
			       outData, (int *)(outPtr),outExt, id);
	break;
      case VTK_SHORT:
	vtkOptimizedExecute(this, inData, (short *)(inPtr), 
			       outData, (short *)(outPtr),outExt, id);
	break;
      case VTK_UNSIGNED_SHORT:
	vtkOptimizedExecute(this, inData, (unsigned short *)(inPtr), 
			       outData, (unsigned short *)(outPtr),outExt,id);
	break;
      case VTK_UNSIGNED_CHAR:
	vtkOptimizedExecute(this, inData, (unsigned char *)(inPtr), 
			       outData, (unsigned char *)(outPtr),outExt, id);
	break;
      default:
	vtkErrorMacro(<< "Execute: Unknown input ScalarType");
	return;
      }
    }
  else
    {
    switch (inData->GetScalarType())
      {
      case VTK_FLOAT:
	vtkImageResliceExecute(this, inData, (float *)(inPtr), 
			       outData, (float *)(outPtr),outExt, id);
	break;
      case VTK_INT:
	vtkImageResliceExecute(this, inData, (int *)(inPtr), 
			       outData, (int *)(outPtr),outExt, id);
	break;
      case VTK_SHORT:
	vtkImageResliceExecute(this, inData, (short *)(inPtr), 
			       outData, (short *)(outPtr),outExt, id);
	break;
      case VTK_UNSIGNED_SHORT:
	vtkImageResliceExecute(this, inData, (unsigned short *)(inPtr), 
			       outData, (unsigned short *)(outPtr),outExt,id);
	break;
      case VTK_UNSIGNED_CHAR:
	vtkImageResliceExecute(this, inData, (unsigned char *)(inPtr), 
			       outData, (unsigned char *)(outPtr),outExt, id);
	break;
      default:
	vtkErrorMacro(<< "Execute: Unknown input ScalarType");
	return;
      }
    }
}


