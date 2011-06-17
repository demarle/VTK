/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageResliceMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageResliceMapper.h"

#include "vtkImageSliceMapper.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkImageSlice.h"
#include "vtkImageData.h"
#include "vtkImageProperty.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkMatrix4x4.h"
#include "vtkAbstractTransform.h"
#include "vtkPlane.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageResliceToColors.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkImageResliceMapper);

//----------------------------------------------------------------------------
vtkImageResliceMapper::vtkImageResliceMapper()
{
  this->SliceMapper = vtkImageSliceMapper::New();
  this->ImageReslice = vtkImageResliceToColors::New();
  this->ResliceMatrix = vtkMatrix4x4::New();
  this->WorldToDataMatrix = vtkMatrix4x4::New();
  this->SliceToWorldMatrix = vtkMatrix4x4::New();

  this->AutoAdjustImageQuality = 1;
  this->ResampleToScreenPixels = 1;
  this->InternalResampleToScreenPixels = 1;
}

//----------------------------------------------------------------------------
vtkImageResliceMapper::~vtkImageResliceMapper()
{
  if (this->SliceMapper)
    {
    this->SliceMapper->Delete();
    }
  if (this->ImageReslice)
    {
    this->ImageReslice->Delete();
    }
  if (this->ResliceMatrix)
    {
    this->ResliceMatrix->Delete();
    }
  if (this->WorldToDataMatrix)
    {
    this->WorldToDataMatrix->Delete();
    }
  if (this->SliceToWorldMatrix)
    {
    this->SliceToWorldMatrix->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkImageResliceMapper::SetSlicePlane(vtkPlane *plane)
{
  if (this->SlicePlane == plane)
    {
    return;
    }
  if (this->SlicePlane)
    {
    this->SlicePlane->Delete();
    }
  if (!plane)
    {
    this->SlicePlane = vtkPlane::New();
    }
  else
    {
    this->SlicePlane = plane;
    plane->Register(this);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkImageResliceMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  this->SliceMapper->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------------
void vtkImageResliceMapper::Render(vtkRenderer *ren, vtkImageSlice *prop)
{
  vtkImageProperty *property = prop->GetProperty();

  this->InternalResampleToScreenPixels = this->ResampleToScreenPixels;
  if (this->AutoAdjustImageQuality &&
      this->InternalResampleToScreenPixels)
    {
    // only use image-size texture if image is smaller than render window,
    // since otherwise there is far less advantage in doing so
    int *rsize = ren->GetSize();
    int maxrsize = (rsize[0] > rsize[1] ? rsize[0] : rsize[1]);
    int *isize = this->GetInput()->GetDimensions();
    int maxisize = (isize[0] > isize[1] ? isize[0] : isize[1]);
    maxisize = (isize[2] > maxisize ? isize[2] : maxisize);
    if (maxisize <= maxrsize && maxisize <= 1024)
      {
      this->InternalResampleToScreenPixels =
        (prop->GetAllocatedRenderTime() >= 1.0);
      }
    }

  // set the matrices
  this->UpdateResliceMatrix(ren, prop);

  // update the coords for the polygon to be textured
  this->UpdatePolygonCoords(ren);

  // set the reslice spacing/origin/extent/axes
  this->UpdateResliceInformation(ren);

  // set the reslice bits related to the property
  this->UpdateResliceInterpolation(property);

  // update anything related to the image coloring
  this->UpdateColorInformation(property);

  // perform the reslicing
  this->ImageReslice->SetInput(this->GetInput());
  this->ImageReslice->UpdateWholeExtent();
 
  // apply checkerboard pattern (should have timestamps)
  if (property && property->GetCheckerboard() &&
      this->InternalResampleToScreenPixels &&
      this->SliceFacesCamera)
    {
    this->CheckerboardImage(this->ImageReslice->GetOutput(),
      ren->GetActiveCamera(), property);
    }

  // everything else is delegated
  this->SliceMapper->SetInput(this->ImageReslice->GetOutput());
  this->SliceMapper->GetDataToWorldMatrix()->DeepCopy(
    this->SliceToWorldMatrix);
  this->SliceMapper->SetSliceFacesCamera(this->SliceFacesCamera);
  this->SliceMapper->SetExactPixelMatch(this->InternalResampleToScreenPixels);
  this->SliceMapper->SetBorder( (this->Border ||
                                 this->InternalResampleToScreenPixels) );
  this->SliceMapper->SetPassColorData(true);
  this->SliceMapper->SetDisplayExtent(this->ImageReslice->GetOutputExtent());

  // render pass info for members of vtkImageStack
  this->SliceMapper->MatteEnable = this->MatteEnable;
  this->SliceMapper->ColorEnable = this->ColorEnable;
  this->SliceMapper->DepthEnable = this->DepthEnable;

  // let vtkImageSliceMapper do the rest of the work
  this->SliceMapper->Render(ren, prop);
}

//----------------------------------------------------------------------------
int vtkImageResliceMapper::ProcessRequest(
  vtkInformation* request, vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  if (request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    // Get point/normal from camera
    if (this->SliceFacesCamera || this->SliceAtFocalPoint)
      {
      vtkRenderer *ren = this->GetCurrentRenderer();

      if (ren)
        {
        vtkCamera *camera = ren->GetActiveCamera();

        if (this->SliceAtFocalPoint)
          {
          this->SlicePlane->SetOrigin(camera->GetFocalPoint());
          }
        if (this->SliceFacesCamera)
          {
          double normal[3];
          camera->GetDirectionOfProjection(normal);
          normal[0] = -normal[0];
          normal[1] = -normal[1];
          normal[2] = -normal[2];
          this->SlicePlane->SetNormal(normal);
          }
        }
      }

    // use superclass method to update other important info
    return this->Superclass::ProcessRequest(
      request, inputVector, outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
// Update the WorldToData transformation matrix, which is just the
// inverse of the vtkProp3D matrix.
void vtkImageResliceMapper::UpdateWorldToDataMatrix(vtkImageSlice *prop)
{
  // copy the matrix, but only if it has changed (we do this to
  // preserve the modified time of the matrix)
  double tmpmat[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
  if (!prop->GetIsIdentity())
    {
    vtkMatrix4x4::Invert(*prop->GetMatrix()->Element, tmpmat);
    }
  double *mat = *this->WorldToDataMatrix->Element;
  for (int i = 0; i < 16; i++)
    {
    if (mat[i] != tmpmat[i])
      {
      this->WorldToDataMatrix->DeepCopy(tmpmat);
      break;
      }
    }
}

//----------------------------------------------------------------------------
// Update the SliceToWorld transformation matrix
void vtkImageResliceMapper::UpdateSliceToWorldMatrix(vtkCamera *camera)
{
  // NOTE: This method is only called if InternalResampleToScreenPixels is On

  // Get slice plane in world coords by passing null as the prop matrix 
  double plane[4];
  this->GetSlicePlaneInDataCoords(0, plane);

  // Make sure normal is facing towards camera
  vtkMatrix4x4 *viewMatrix = camera->GetViewTransformMatrix();
  double *ndop = viewMatrix->Element[2];
  if (vtkMath::Dot(ndop, plane) < 0)
    {
    plane[0] = -plane[0];
    plane[1] = -plane[1];
    plane[2] = -plane[2];
    plane[3] = -plane[3];
    }

  // The normal is the first three elements
  double *normal = plane;

  // The last element is -dot(normal, origin)
  double dp = -plane[3];

  // Compute rotation angle between camera axis and slice normal
  double vec[3];
  vtkMath::Cross(ndop, normal, vec);
  double costheta = vtkMath::Dot(ndop, normal);
  double sintheta = vtkMath::Norm(vec);
  double theta = atan2(sintheta, costheta);
  if (sintheta != 0)
    {
    vec[0] /= sintheta;
    vec[1] /= sintheta;
    vec[2] /= sintheta;
    }
  // convert to quaternion
  costheta = cos(0.5*theta);
  sintheta = sin(0.5*theta);
  double quat[4];
  quat[0] = costheta;
  quat[1] = vec[0]*sintheta;
  quat[2] = vec[1]*sintheta;
  quat[3] = vec[2]*sintheta;
  // convert to matrix
  double mat[3][3];
  vtkMath::QuaternionToMatrix3x3(quat, mat);

  // Create a slice-to-world transform matrix
  // The columns are v1, v2, normal
  vtkMatrix4x4 *sliceToWorld = this->SliceToWorldMatrix;

  double v1[3], v2[3];
  vtkMath::Multiply3x3(mat, viewMatrix->Element[0], v1);
  vtkMath::Multiply3x3(mat, viewMatrix->Element[1], v2);

  sliceToWorld->Element[0][0] = v1[0];
  sliceToWorld->Element[1][0] = v1[1];
  sliceToWorld->Element[2][0] = v1[2];
  sliceToWorld->Element[3][0] = 0.0;

  sliceToWorld->Element[0][1] = v2[0];
  sliceToWorld->Element[1][1] = v2[1];
  sliceToWorld->Element[2][1] = v2[2];
  sliceToWorld->Element[3][1] = 0.0;

  sliceToWorld->Element[0][2] = normal[0];
  sliceToWorld->Element[1][2] = normal[1];
  sliceToWorld->Element[2][2] = normal[2];
  sliceToWorld->Element[3][2] = 0.0;

  sliceToWorld->Element[0][3] = -dp*normal[0];
  sliceToWorld->Element[1][3] = -dp*normal[1];
  sliceToWorld->Element[2][3] = dp-dp*normal[2];
  sliceToWorld->Element[3][3] = 1.0;
}

//----------------------------------------------------------------------------
// Update the reslice matrix, which is the slice-to-data matrix.
void vtkImageResliceMapper::UpdateResliceMatrix(
  vtkRenderer *ren, vtkImageSlice *prop)
{
  vtkMatrix4x4 *resliceMatrix = this->ResliceMatrix;
  vtkMatrix4x4 *propMatrix = prop->GetMatrix();

  // Get world-to-data matrix from the prop matrix
  this->UpdateWorldToDataMatrix(prop);

  // Compute SliceToWorld matrix from camera if InternalResampleToScreenPixels
  if (this->InternalResampleToScreenPixels)
    {
    this->UpdateSliceToWorldMatrix(ren->GetActiveCamera());
    vtkMatrix4x4::Multiply4x4(
      this->WorldToDataMatrix, this->SliceToWorldMatrix, this->ResliceMatrix);

    return;
    }

  // Get slice plane in world coords by passing null as the matrix
  double plane[4];
  this->GetSlicePlaneInDataCoords(0, plane);

  // Check whether normal is facing towards camera, the "ndop" is
  // the negative of the direction of projection for the camera
  vtkMatrix4x4 *viewMatrix = ren->GetActiveCamera()->GetViewTransformMatrix();
  double *ndop = viewMatrix->Element[2];
  double dotprod = vtkMath::Dot(ndop, plane);

  // Get slice plane in data coords by passing the prop matrix, flip
  // normal to face the camera 
  this->GetSlicePlaneInDataCoords(prop->GetMatrix(), plane);
  if (dotprod < 0)
    {
    plane[0] = -plane[0];
    plane[1] = -plane[1];
    plane[2] = -plane[2];
    plane[3] = -plane[3];
    }

  // Find the largest component of the normal
  int maxi = 0;
  double maxv = 0.0;
  for (int i = 0; i < 3; i++)
    {
    double tmp = plane[i]*plane[i];
    if (tmp > maxv)
      {
      maxi = i;
      maxv = tmp;
      }
    }

  // Create the corresponding axis
  double axis[3];
  axis[0] = 0.0;
  axis[1] = 0.0;
  axis[2] = 0.0;
  axis[maxi] = ((plane[maxi] < 0.0) ? -1.0 : 1.0);

  // Create two orthogonal axes
  double saxis[3], taxis[3];
  taxis[0] = 0.0;
  taxis[1] = 1.0;
  taxis[2] = 0.0;
  if (maxi == 1)
    {
    taxis[1] = 0.0;
    taxis[2] = 1.0;
    }
  vtkMath::Cross(taxis, axis, saxis);

  // The normal is the first three elements
  double *normal = plane;

  // The last element is -dot(normal, origin)
  double dp = -plane[3];

  // Compute the rotation angle between the axis and the normal
  double vec[3];
  vtkMath::Cross(axis, normal, vec);
  double costheta = vtkMath::Dot(axis, normal);
  double sintheta = vtkMath::Norm(vec);
  double theta = atan2(sintheta, costheta);
  if (sintheta != 0)
    {
    vec[0] /= sintheta;
    vec[1] /= sintheta;
    vec[2] /= sintheta;
    }
  // convert to quaternion
  costheta = cos(0.5*theta);
  sintheta = sin(0.5*theta);
  double quat[4];
  quat[0] = costheta;
  quat[1] = vec[0]*sintheta;
  quat[2] = vec[1]*sintheta;
  quat[3] = vec[2]*sintheta;
  // convert to matrix
  double mat[3][3];
  vtkMath::QuaternionToMatrix3x3(quat, mat);
  
  // Create a slice-to-data transform matrix
  // The columns are v1, v2, normal
  double v1[3], v2[3];
  vtkMath::Multiply3x3(mat, saxis, v1);
  vtkMath::Multiply3x3(mat, taxis, v2);

  resliceMatrix->Element[0][0] = v1[0];
  resliceMatrix->Element[1][0] = v1[1];
  resliceMatrix->Element[2][0] = v1[2];
  resliceMatrix->Element[3][0] = 0.0;

  resliceMatrix->Element[0][1] = v2[0];
  resliceMatrix->Element[1][1] = v2[1];
  resliceMatrix->Element[2][1] = v2[2];
  resliceMatrix->Element[3][1] = 0.0;

  resliceMatrix->Element[0][2] = normal[0];
  resliceMatrix->Element[1][2] = normal[1];
  resliceMatrix->Element[2][2] = normal[2];
  resliceMatrix->Element[3][2] = 0.0;

  resliceMatrix->Element[0][3] = dp*(propMatrix->Element[2][0] - normal[0]);
  resliceMatrix->Element[1][3] = dp*(propMatrix->Element[2][1] - normal[1]);
  resliceMatrix->Element[2][3] = dp*(propMatrix->Element[2][2] - normal[2]);
  resliceMatrix->Element[3][3] = 1.0;

  // Compute the SliceToWorldMatrix
  vtkMatrix4x4::Multiply4x4(propMatrix, resliceMatrix,
    this->SliceToWorldMatrix);
}

//----------------------------------------------------------------------------
// Do all the fancy math to set up the reslicing
void vtkImageResliceMapper::UpdateResliceInformation(vtkRenderer *ren)
{
  vtkMatrix4x4 *resliceMatrix = this->ResliceMatrix;
  vtkImageResliceToColors *reslice = this->ImageReslice;

  int extent[6];
  double spacing[3];
  double origin[3];

  // Get the view matrix
  vtkCamera *camera = ren->GetActiveCamera();
  vtkMatrix4x4 *viewMatrix = camera->GetViewTransformMatrix();

  // Get slice plane in world coords by passing null as the matrix
  double plane[4];
  this->GetSlicePlaneInDataCoords(0, plane);

  // Check whether normal is facing towards camera, the "ndop" is
  // the negative of the direction of projection for the camera
  double *ndop = viewMatrix->Element[2];
  if (vtkMath::Dot(ndop, plane) < 0)
    {
    plane[0] = -plane[0];
    plane[1] = -plane[1];
    plane[2] = -plane[2];
    plane[3] = -plane[3];
    }

  // Get the z position of the slice in slice coords
  // (requires plane to be normalized by GetSlicePlaneInDataCoords)
  double z = (plane[2] - 2.0)*plane[3];

  if (this->InternalResampleToScreenPixels)
    {
    // Get the projection matrix
    double aspect = ren->GetTiledAspectRatio();
    vtkMatrix4x4 *projMatrix = camera->GetProjectionTransformMatrix(
                                 aspect, 0, 1);

    // Compute other useful matrices
    double worldToView[16];
    double viewToWorld[16];
    double planeWorldToView[16];
    vtkMatrix4x4::Multiply4x4(
      *projMatrix->Element, *viewMatrix->Element, worldToView);
    vtkMatrix4x4::Invert(worldToView, viewToWorld);
    vtkMatrix4x4::Transpose(viewToWorld, planeWorldToView);

    double worldToSlice[16];
    double viewToSlice[16];
    vtkMatrix4x4::Invert(*this->SliceToWorldMatrix->Element, worldToSlice);
    vtkMatrix4x4::Multiply4x4(worldToSlice, viewToWorld, viewToSlice);

    // Transform the plane into view coordinates, using the transpose
    // of the inverse of the world-to-view matrix
    vtkMatrix4x4::MultiplyPoint(planeWorldToView, plane, plane);

    // Compute the bounds in slice coords
    double xmin = VTK_DOUBLE_MAX;
    double xmax = -VTK_DOUBLE_MAX;
    double ymin = VTK_DOUBLE_MAX;
    double ymax = -VTK_DOUBLE_MAX;

    for (int i = 0; i < 4; i++)
      {
      // The four corners of the view
      double x = (((i & 1) == 0) ? -1.0 : 1.0);
      double y = (((i & 2) == 0) ? -1.0 : 1.0);

      double hpoint[4];
      hpoint[0] = x;
      hpoint[1] = y;
      hpoint[2] = 0.0;
      hpoint[3] = 1.0;

      if (fabs(plane[2]) < 1e-6)
        {
        // Looking at plane edge-on, just put some
        // points at front clipping plane, others at back plane
        hpoint[2] = (((i & 1) == 0) ? 0.0 : 1.0);
        }
      else
        {
        // Intersect with the slice plane
        hpoint[2] = - (x*plane[0] + y*plane[1] + plane[3])/plane[2];

        // Clip to the front and back clipping planes
        if (hpoint[2] < 0)
          {
          hpoint[2] = 0.0;
          }
        else if (hpoint[2] > 1)
          {
          hpoint[2] = 1.0;
          }
        }

      // Transform into slice coords
      vtkMatrix4x4::MultiplyPoint(viewToSlice, hpoint, hpoint);

      x = hpoint[0]/hpoint[3];
      y = hpoint[1]/hpoint[3];

      // Find min/max in slice coords
      if (x < xmin) { xmin = x; }
      if (x > xmax) { xmax = x; }
      if (y < ymin) { ymin = y; }
      if (y > ymax) { ymax = y; }
      }

    // The ResliceExtent is always set to the renderer size,
    // this is the maximum size ever required and sticking to
    // this size avoids any memory reallocation on GPU or CPU
    int *size = ren->GetSize();
    int xsize = ((size[0] <= 0) ? 1 : size[0]);
    int ysize = ((size[1] <= 0) ? 1 : size[1]);

    extent[0] = 0;
    extent[1] = xsize - 1;
    extent[2] = 0;
    extent[3] = ysize - 1;
    extent[4] = 0;
    extent[5] = 0;

    // Find the spacing
    spacing[0] = (xmax - xmin)/xsize;
    spacing[1] = (ymax - ymin)/ysize;
    spacing[2] = 1.0;

    // Corner of resliced plane, including half-pixel offset to
    // exactly match texels to pixels in the final rendering
    origin[0] = xmin + 0.5*spacing[0];
    origin[1] = ymin + 0.5*spacing[1];
    origin[2] = z;
    }
  else
    {
    // Compute texel spacing from image spacing
    double inputSpacing[3];
    this->GetInput()->GetSpacing(inputSpacing);
    inputSpacing[0] = fabs(inputSpacing[0]);
    inputSpacing[1] = fabs(inputSpacing[1]);
    inputSpacing[2] = fabs(inputSpacing[2]);
    for (int j = 0; j < 2; j++)
      {
      double xc = this->ResliceMatrix->Element[j][0];
      double yc = this->ResliceMatrix->Element[j][1];
      double zc = this->ResliceMatrix->Element[j][2];
      xc *= xc;
      yc *= yc;
      zc *= zc;
      spacing[j] = (xc*inputSpacing[0] +
                    yc*inputSpacing[1] +
                    zc*inputSpacing[2]);
      }

    // Always set z spacing to unity
    spacing[2] = 1.0;

    // Find the bounds for the texture
    double xmin = VTK_DOUBLE_MAX;
    double xmax = -VTK_DOUBLE_MAX;
    double ymin = VTK_DOUBLE_MAX;
    double ymax = -VTK_DOUBLE_MAX;

    vtkPoints *points = this->SliceMapper->GetPoints();
    vtkIdType n = points->GetNumberOfPoints();
    if (n == 0)
      {
      double inputOrigin[3];
      this->GetInput()->GetOrigin(inputOrigin);
      xmin = inputOrigin[0];
      xmax = inputOrigin[0];
      ymin = inputOrigin[1];
      ymax = inputOrigin[1];
      }

    for (vtkIdType k = 0; k < n; k++)
      {
      double point[3];
      points->GetPoint(k, point);

      xmin = ((xmin < point[0]) ? xmin : point[0]);
      xmax = ((xmax > point[0]) ? xmax : point[0]);
      ymin = ((ymin < point[1]) ? ymin : point[1]);
      ymax = ((ymax > point[1]) ? ymax : point[1]);
      }

    double tol = 7.62939453125e-06;
    int xsize = vtkMath::Floor((xmax - xmin)/spacing[0] + tol);
    int ysize = vtkMath::Floor((ymax - ymin)/spacing[1] + tol);
    if (this->Border == 0)
      {
      xsize += 1;
      ysize += 1;
      }
    if (xsize < 1) { xsize = 1; }
    if (ysize < 1) { ysize = 1; }

    extent[0] = 0;
    extent[1] = xsize - 1;
    extent[2] = 0;
    extent[3] = ysize - 1;
    extent[4] = 0;
    extent[5] = 0;

    origin[0] = xmin + 0.5*spacing[0]*(this->Border != 0);
    origin[1] = ymin + 0.5*spacing[1]*(this->Border != 0);
    origin[2] = z;
    }

  // Prepare for reslicing
  reslice->SetResliceAxes(resliceMatrix);
  reslice->SetOutputExtent(extent);
  reslice->SetOutputSpacing(spacing);
  reslice->SetOutputOrigin(origin);

  if (this->SliceFacesCamera && this->InternalResampleToScreenPixels)
    {
    // if slice follows camera, use reslice to set the border
    reslice->SetBorder(this->Border);
    }
  else
    {
    // tell reslice to use a double-thickness border,
    // since the polygon geometry will dictate the actual size
    reslice->SetBorder(2);
    }
}

//----------------------------------------------------------------------------
// Do all the fancy math to set up the reslicing
void vtkImageResliceMapper::UpdateColorInformation(vtkImageProperty *property)
{
  vtkScalarsToColors *lookupTable = this->DefaultLookupTable;

  if (property)
    {
    double colorWindow = property->GetColorWindow();
    double colorLevel = property->GetColorLevel();
    if (property->GetLookupTable())
      {
      lookupTable = property->GetLookupTable();
      if (!property->GetUseLookupTableScalarRange())
        {
        lookupTable->SetRange(colorLevel - 0.5*colorWindow,
                              colorLevel + 0.5*colorWindow);
        }
      }
    else
      {
      lookupTable->SetRange(colorLevel - 0.5*colorWindow,
                            colorLevel + 0.5*colorWindow);
      }
    }
  else
    {
    lookupTable->SetRange(0, 255);
    }

  this->ImageReslice->SetLookupTable(lookupTable);
}

//----------------------------------------------------------------------------
// Do all the fancy math to set up the reslicing
void vtkImageResliceMapper::UpdateResliceInterpolation(
  vtkImageProperty *property)
{
  // set the interpolation mode and border
  int interpMode = VTK_RESLICE_NEAREST;

  if (property)
    {
    switch(property->GetInterpolationType())
      {
      case VTK_NEAREST_INTERPOLATION:
        interpMode = VTK_RESLICE_NEAREST;
        break;
      case VTK_LINEAR_INTERPOLATION:
        interpMode = VTK_RESLICE_LINEAR;
        break;
      case VTK_CUBIC_INTERPOLATION:
        interpMode = VTK_RESLICE_CUBIC;
        break;
      }
    }

  this->ImageReslice->SetInterpolationMode(interpMode);
}

//----------------------------------------------------------------------------
void vtkImageResliceMapper::CheckerboardImage(
  vtkImageData *input, vtkCamera *camera, vtkImageProperty *property)
{
  // Use focal point as center of checkerboard pattern.  This guarantees
  // exactly the same checkerboard for all images in the scene, which is
  // useful when doing multiple overlays.
  double focalPoint[4];
  camera->GetFocalPoint(focalPoint);
  focalPoint[3] = 1.0;

  double worldToSlice[16];
  vtkMatrix4x4::Invert(*this->SliceToWorldMatrix->Element, worldToSlice);

  vtkMatrix4x4::MultiplyPoint(worldToSlice, focalPoint, focalPoint);
  if (focalPoint[3] != 0.0)
    {
    focalPoint[0] /= focalPoint[3];
    focalPoint[1] /= focalPoint[3];
    focalPoint[2] /= focalPoint[3];
    }

  // Get the checkerboard spacing and apply the offset fraction
  double checkSpacing[2], checkOffset[2];
  property->GetCheckerboardSpacing(checkSpacing);
  property->GetCheckerboardOffset(checkOffset);
  checkOffset[0] = checkOffset[0]*checkSpacing[0] + focalPoint[0];
  checkOffset[1] = checkOffset[1]*checkSpacing[1] + focalPoint[1];

  // Adjust according to the origin and spacing of the slice data
  double origin[3], spacing[3];
  input->GetSpacing(spacing);
  input->GetOrigin(origin);
  checkOffset[0] = (checkOffset[0] - origin[0])/spacing[0];
  checkOffset[1] = (checkOffset[1] - origin[1])/spacing[1];
  checkSpacing[0] /= spacing[0],
  checkSpacing[1] /= spacing[1];

  // Apply the checkerboard to the data
  int extent[6];
  input->GetExtent(extent);
  unsigned char *data = static_cast<unsigned char *>(
    input->GetScalarPointerForExtent(extent));

  vtkImageMapper3D::CheckerboardRGBA(
    data, extent[1] - extent[0] + 1, extent[3] - extent[2] + 1,
    checkOffset[0], checkOffset[1], checkSpacing[0], checkSpacing[1]);
}

//----------------------------------------------------------------------------
// Compute the vertices of the polygon in the slice coordinate system
void vtkImageResliceMapper::UpdatePolygonCoords(vtkRenderer *ren)
{
  // Get the projection matrix
  double aspect = ren->GetTiledAspectRatio();
  vtkCamera *camera = ren->GetActiveCamera();
  vtkMatrix4x4 *viewMatrix = camera->GetViewTransformMatrix();
  vtkMatrix4x4 *projMatrix = camera->GetProjectionTransformMatrix(
                               aspect, 0, 1);

  // Compute other useful matrices
  double worldToView[16];
  double viewToWorld[16];
  vtkMatrix4x4::Multiply4x4(
    *projMatrix->Element, *viewMatrix->Element, worldToView);
  vtkMatrix4x4::Invert(worldToView, viewToWorld);

  double worldToSlice[16];
  double viewToSlice[16];
  vtkMatrix4x4::Invert(*this->SliceToWorldMatrix->Element, worldToSlice);
  vtkMatrix4x4::Multiply4x4(worldToSlice, viewToWorld, viewToSlice);

  // Get slice plane in world coords by passing null as the matrix
  double plane[4];
  this->GetSlicePlaneInDataCoords(0, plane);

  // Check whether normal is facing towards camera, the "ndop" is
  // the negative of the direction of projection for the camera
  double *ndop = viewMatrix->Element[2];
  if (vtkMath::Dot(ndop, plane) < 0)
    {
    plane[0] = -plane[0];
    plane[1] = -plane[1];
    plane[2] = -plane[2];
    plane[3] = -plane[3];
    }

  // Get the z position of the slice in slice coords
  // (requires plane to be normalized by GetSlicePlaneInDataCoords)
  double z = (plane[2] - 2.0)*plane[3];

  // Generate a tolerance based on the screen pixel size
  double fpoint[4];
  camera->GetFocalPoint(fpoint);
  fpoint[3] = 1.0;
  vtkMatrix4x4::MultiplyPoint(worldToView, fpoint, fpoint);
  fpoint[0] /= fpoint[3];
  fpoint[1] /= fpoint[3];
  fpoint[2] /= fpoint[3];
  fpoint[3] = 1.0;

  double topOfScreen[4], botOfScreen[4];
  fpoint[1] -= 1.0;
  vtkMatrix4x4::MultiplyPoint(viewToWorld, fpoint, topOfScreen);
  fpoint[1] += 2.0;
  vtkMatrix4x4::MultiplyPoint(viewToWorld, fpoint, botOfScreen);

  topOfScreen[0] /= topOfScreen[3];
  topOfScreen[1] /= topOfScreen[3];
  topOfScreen[2] /= topOfScreen[3];
  topOfScreen[3] = 1.0;

  botOfScreen[0] /= botOfScreen[3];
  botOfScreen[1] /= botOfScreen[3];
  botOfScreen[2] /= botOfScreen[3];
  botOfScreen[3] = 1.0;

  // height of view in world coords at focal point
  double viewHeight =
    sqrt(vtkMath::Distance2BetweenPoints(topOfScreen, botOfScreen));

  // height of view in pixels
  int height = ren->GetSize()[1];

  double tol = (height == 0 ? 0.5 : viewHeight*0.5/height); 

  // make the data bounding box (with or without border)
  int border = this->Border;
  double b = (border ? 0.5 : 0.0);
  double bounds[6];
  for (int ii = 0; ii < 3; ii++)
    {
    double c = b*this->DataSpacing[ii];
    int lo = this->DataWholeExtent[2*ii];
    int hi = this->DataWholeExtent[2*ii+1];
    if (border == 0 && lo == hi)
      { // apply tolerance to avoid degeneracy
      c = tol;
      }
    bounds[2*ii]   = lo*this->DataSpacing[ii] + this->DataOrigin[ii] - c;
    bounds[2*ii+1] = hi*this->DataSpacing[ii] + this->DataOrigin[ii] + c;
    }

  // transform the vertices to the slice coord system
  double xpoints[8];
  double ypoints[8];
  double weights[8];
  bool above[8];
  double mat[16];
  vtkMatrix4x4::Multiply4x4(*this->WorldToDataMatrix->Element,
                            *this->SliceToWorldMatrix->Element, mat);
  vtkMatrix4x4::Invert(mat, mat);

  for (int i = 0; i < 8; i++)
    {
    double point[4];
    point[0] = bounds[0 + ((i>>0)&1)];
    point[1] = bounds[2 + ((i>>1)&1)];
    point[2] = bounds[4 + ((i>>2)&1)];
    point[3] = 1.0;
    vtkMatrix4x4::MultiplyPoint(mat, point, point);
    xpoints[i] = point[0]/point[3];
    ypoints[i] = point[1]/point[3];
    weights[i] = point[2]/point[3] - z;
    above[i] = (weights[i] >= 0);
    }

  // go through the edges and find the new points 
  double newxpoints[12];
  double newypoints[12];
  double cx = 0.0;
  double cy = 0.0;
  int n = 0;
  for (int j = 0; j < 12; j++)
    {
    // verts from edges (sorry about this..)
    int i1 = (j & 3) | (((j<<1) ^ (j<<2)) & 4);
    int i2 = (i1 ^ (1 << (j>>2)));

    if (above[i1] ^ above[i2])
      {
      double w1 = weights[i2];
      double w2 = -weights[i1];
      newxpoints[n] = (w1*xpoints[i1] + w2*xpoints[i2])/(w1 + w2);
      newypoints[n] = (w1*ypoints[i1] + w2*ypoints[i2])/(w1 + w2);
      cx += newxpoints[n];
      cy += newypoints[n];
      n++;
      }
    }

  // n should never exceed six
  if (n > 6)
    {
    vtkErrorMacro("UpdateCutPolygon generated more than "
                  "6 points, please report a bug!");
    }

  double coords[18];

  if (n > 0)
    {
    // centroid 
    cx /= n;
    cy /= n;

    // sort the points to make a convex polygon
    double angles[6];
    for (int k = 0; k < n; k++)
      {
      double x = newxpoints[k];
      double y = newypoints[k];
      double t = atan2(y - cy, x - cx);
      int kk;
      for (kk = 0; kk < k; kk++)
        {
        if (t < angles[kk]) { break; }
        }
      for (int jj = k; jj > kk; --jj)
        {
        int jj3 = jj*3;
        angles[jj] = angles[jj-1];
        coords[jj3] = coords[jj3-3]; 
        coords[jj3+1] = coords[jj3-2]; 
        coords[jj3+2] = coords[jj3-1]; 
        }
      int kk3 = kk*3;
      angles[kk] = t;
      coords[kk3] = x;
      coords[kk3+1] = y;
      coords[kk3+2] = z;
      }
    }

  vtkPoints *points = this->SliceMapper->GetPoints();
  if (!points)
    {
    points = vtkPoints::New();
    points->SetDataTypeToDouble();
    this->SliceMapper->SetPoints(points);
    points->Delete();
    }

  points->SetNumberOfPoints(n);
  for (int k = 0; k < n; k++)
    {
    points->SetPoint(k, &coords[3*k]);
    }
}

//----------------------------------------------------------------------------
void vtkImageResliceMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "AutoAdjustImageQuality: "
     << (this->AutoAdjustImageQuality ? "On\n" : "Off\n");
  os << indent << "ResampleToScreenPixels: "
     << (this->ResampleToScreenPixels ? "On\n" : "Off\n");
}

//----------------------------------------------------------------------------
unsigned long vtkImageResliceMapper::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();

  // Include camera in MTime so that REQUEST_INFORMATION
  // will be called if the camera changes
  if (this->SliceFacesCamera || this->SliceAtFocalPoint)
    {
    vtkRenderer *ren = this->GetCurrentRenderer();
    if (ren)
      {
      vtkCamera *camera = ren->GetActiveCamera();
      unsigned long mTime2 = camera->GetMTime();
      if (mTime2 > mTime)
        {
        mTime = mTime2;
        }
      }
    }

  if (!this->SliceFacesCamera || !this->SliceAtFocalPoint)
    {
    unsigned long sTime = this->SlicePlane->GetMTime();
    if (sTime > mTime)
      {
      mTime = sTime;
      }
    }

  return mTime;
}

//----------------------------------------------------------------------------
double *vtkImageResliceMapper::GetBounds()
{
  // Modify to give just the slice bounds
  if (!this->GetInput())
    {
    vtkMath::UninitializeBounds(this->Bounds);
    return this->Bounds;
    }
  else
    {
    this->UpdateInformation();
    double *spacing = this->DataSpacing;
    double *origin = this->DataOrigin;
    int *extent = this->DataWholeExtent;

    int swapXBounds = (spacing[0] < 0);  // 1 if true, 0 if false
    int swapYBounds = (spacing[1] < 0);  // 1 if true, 0 if false
    int swapZBounds = (spacing[2] < 0);  // 1 if true, 0 if false

    this->Bounds[0] = origin[0] + (extent[0+swapXBounds] * spacing[0]);
    this->Bounds[2] = origin[1] + (extent[2+swapYBounds] * spacing[1]);
    this->Bounds[4] = origin[2] + (extent[4+swapZBounds] * spacing[2]);

    this->Bounds[1] = origin[0] + (extent[1-swapXBounds] * spacing[0]);
    this->Bounds[3] = origin[1] + (extent[3-swapYBounds] * spacing[1]);
    this->Bounds[5] = origin[2] + (extent[5-swapZBounds] * spacing[2]);

    return this->Bounds;
    }
}

//----------------------------------------------------------------------------
void vtkImageResliceMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  vtkGarbageCollectorReport(collector, this->ImageReslice, "ImageReslice");
  vtkGarbageCollectorReport(collector, this->SliceMapper, "SliceMapper");
}
