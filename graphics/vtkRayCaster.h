/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRayCaster.h
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
// .NAME vtkRayCaster - A helper object for the renderer that controls ray casting

// .SECTION Description
// vtkRayCaster is an automatically created object within vtkRenderer. It is used
// for ray casting operations. It stores variables such as the view rays, and
// information on multiresolution image rendering which are queried by the 
// specific ray casters.

// .SECTION see also
// vtkRenderer vtkViewRays

#ifndef __vtkRayCaster_h
#define __vtkRayCaster_h

#include "vtkRenderer.h"
#include "vtkVolume.h"
#include "vtkViewRays.h"
#include "vtkVolumeMapper.h"

#define VTK_MAX_VIEW_RAYS_LEVEL 4

class VTK_EXPORT vtkRayCaster : public vtkObject
{
public:

// Description:
// Constructor for vtkRayCaster
  vtkRayCaster();


// Description:
// Destructor for vtkRayCaster
  ~vtkRayCaster();

  static vtkRayCaster *New() {return new vtkRayCaster;};
  const char *GetClassName() {return "vtkRayCaster";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Renders its volumes to create a composite image.

// Description:
// Main routine to do the volume rendering.
  int Render(vtkRenderer *);

 
  // Methods for a vtkVolumeMapper to retrieve latest color and zbuffer
  float *GetCurrentColorBuffer() { return (this->cbuffer); };
  float *GetCurrentZBuffer() { return (this->zbuffer); };

  float *GetPerspectiveViewRays();

// Description:
// Get the size in pixels of the view rays for the selected scale indexl
  void GetViewRaysSize( int size[2] );


  float *GetParallelStartPosition( void );
  float *GetParallelIncrements( void );


// Description:
// Set the scale factor for a given level. This is used during multi-
// resolution interactive rendering
  void SetImageScale(int level, float scale); 


// Description:
// Get the scale factor for a given level. This is used during multi-
// resolution interactive rendering
  float GetImageScale(int level); 

  
  vtkSetClampMacro(SelectedImageScaleIndex, int, 0, VTK_MAX_VIEW_RAYS_LEVEL-1);
  vtkGetMacro( SelectedImageScaleIndex, int );

  void SetViewRaysStepSize(int level, float scale); 
  float GetViewRaysStepSize(int level); 

  vtkSetObjectMacro(Renderer,vtkRenderer);
  vtkGetObjectMacro(Renderer,vtkRenderer);


// Description:
// This method returns the scale that should be applied to the viewport
// for geometric rendering, and for the image in volume rendering. It 
// is either explicitly set (if AutomaticScaleAdjustment is off) or
// is adjusted automatically to get the desired frame rate.
//
// Note: IMPORTANT!!!! This should only be called once per render!!!
//
  float GetViewportScaleFactor( vtkRenderer *ren );

  float GetViewportStepSize( );
  
  vtkGetMacro( AutomaticScaleAdjustment, int );

// Description:
// Turn the automatic scale adjustment on
  void AutomaticScaleAdjustmentOn( void );


// Description:
// Turn the automatic scale adjustment off
  void AutomaticScaleAdjustmentOff( void );


  vtkSetClampMacro( AutomaticScaleLowerLimit, float, 0.0, 1.0 );
  vtkGetMacro( AutomaticScaleLowerLimit, float );

  int GetImageScaleCount( void ) { return VTK_MAX_VIEW_RAYS_LEVEL; };

  // Description:
  // Set/Get the value of bilinear image zooming.
  vtkSetMacro( BilinearImageZoom, int );
  vtkGetMacro( BilinearImageZoom, int );
  vtkBooleanMacro( BilinearImageZoom, int );

  vtkGetMacro( TotalRenderTime, float );

protected:

  void NearestNeighborZoom(float *smallImage, float *largeImage,
			   int smallDims[2], int largeDims[2] );

  void BilinearZoom(float *smallImage, float *largeImage,
			   int smallDims[2], int largeDims[2] );


  float		*zbuffer;
  float		*cbuffer;
  vtkRenderer   *Renderer;

  vtkViewRays   ViewRays[VTK_MAX_VIEW_RAYS_LEVEL+1];
  float         ImageScale[VTK_MAX_VIEW_RAYS_LEVEL+1];
  int		BilinearImageZoom;
  int           SelectedImageScaleIndex;
  int           StableImageScaleCounter;
  float         PreviousAllocatedTime;
  int           AutomaticScaleAdjustment;
  float         AutomaticScaleLowerLimit;
  float         ImageRenderTime[2];
  float         OldViewport[4];
  float         ViewRaysStepSize[VTK_MAX_VIEW_RAYS_LEVEL];
  float         TotalRenderTime;

  virtual void RescaleImage( float *RGBAImage, int smallSize[2]);
};

#endif


