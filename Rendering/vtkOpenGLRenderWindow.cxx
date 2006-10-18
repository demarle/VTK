/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenGLRenderWindow.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLProperty.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLLight.h"
#include "vtkOpenGLActor.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkUnsignedCharArray.h"
#include "assert.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkgl.h"

#ifndef VTK_IMPLEMENT_MESA_CXX

vtkCxxRevisionMacro(vtkOpenGLRenderWindow, "1.72");
#endif

#define MAX_LIGHTS 8

// Initialize static member that controls global maximum number of multisamples
static int vtkOpenGLRenderWindowGlobalMaximumNumberOfMultiSamples = 8;

void vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(int val)
{
  if (val == vtkOpenGLRenderWindowGlobalMaximumNumberOfMultiSamples) return;
  vtkOpenGLRenderWindowGlobalMaximumNumberOfMultiSamples = val;
}

int vtkOpenGLRenderWindow::GetGlobalMaximumNumberOfMultiSamples()
{
  return vtkOpenGLRenderWindowGlobalMaximumNumberOfMultiSamples;
}

vtkOpenGLRenderWindow::vtkOpenGLRenderWindow()
{
  this->MultiSamples = vtkOpenGLRenderWindowGlobalMaximumNumberOfMultiSamples;
  this->TextureResourceIds = vtkIdList::New();
  if ( this->WindowName ) 
    delete [] this->WindowName;
  this->WindowName = new char[strlen("Visualization Toolkit - OpenGL")+1];
  strcpy( this->WindowName, "Visualization Toolkit - OpenGL" );
  
  this->OffScreenUseFrameBuffer=0;
}

// free up memory & close the window
vtkOpenGLRenderWindow::~vtkOpenGLRenderWindow()
{
  this->TextureResourceIds->Delete();
}

// Update system if needed due to stereo rendering.
void vtkOpenGLRenderWindow::StereoUpdate(void)
{
  // if stereo is on and it wasn't before
  if (this->StereoRender && (!this->StereoStatus))
    {
    switch (this->StereoType) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
        // not clear this is supposed to be empty,
        // but it has been that way forever.
        break;
      case VTK_STEREO_RED_BLUE:
        this->StereoStatus = 1;
        break;
      case VTK_STEREO_ANAGLYPH:
        this->StereoStatus = 1;
        break;
      case VTK_STEREO_DRESDEN:
        this->StereoStatus = 1;
        break;      
      case VTK_STEREO_INTERLACED:
        this->StereoStatus = 1;
      }
    }
  else if ((!this->StereoRender) && this->StereoStatus)
    {
    switch (this->StereoType) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
        this->StereoStatus = 0;
        break;
      case VTK_STEREO_RED_BLUE:
        this->StereoStatus = 0;
        break;
      case VTK_STEREO_ANAGLYPH:
        this->StereoStatus = 0;
        break;
      case VTK_STEREO_DRESDEN:
        this->StereoStatus = 0;
        break;
      case VTK_STEREO_INTERLACED:
        this->StereoStatus = 0;
        break;
      }
    }
}


void vtkOpenGLRenderWindow::OpenGLInit()
{
  glMatrixMode( GL_MODELVIEW );
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  // initialize blending for transparency
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable(GL_BLEND);

  if (this->PointSmoothing)
    {
    glEnable(GL_POINT_SMOOTH);
    }
  else
    {
    glDisable(GL_POINT_SMOOTH);
    }

  if (this->LineSmoothing)
    {
    glEnable(GL_LINE_SMOOTH);
    }
  else
    {
    glDisable(GL_LINE_SMOOTH);
    }

  if (this->PolygonSmoothing)
    {
    glEnable(GL_POLYGON_SMOOTH);
    }
  else
    {
    glDisable(GL_POLYGON_SMOOTH);
    }

  glEnable(GL_NORMALIZE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glAlphaFunc(GL_GREATER,0);
}


void vtkOpenGLRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

int vtkOpenGLRenderWindow::GetDepthBufferSize()
{
  GLint size;

  if ( this->Mapped )
    {
    this->MakeCurrent();
    size = 0;
    glGetIntegerv( GL_DEPTH_BITS, &size );
    return (int) size;
    }
  else
    {
    vtkDebugMacro(<< "Window is not mapped yet!" );
    return 24;
    }
}

unsigned char* vtkOpenGLRenderWindow::GetPixelData(int x1, int y1, 
                                                   int x2, int y2,
                                                   int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  unsigned char *data = 
    new unsigned char[(x_hi - x_low + 1)*(y_hi - y_low + 1)*3];
  this->GetPixelData(x1, y1, x2, y2, front, data);
  return data;
}

int vtkOpenGLRenderWindow::GetPixelData(int x1, int y1,
                                        int x2, int y2,
                                        int front,
                                        vtkUnsignedCharArray* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  int width  = abs(x_hi - x_low) + 1;
  int height = abs(y_hi - y_low) + 1;
  int size = 3*width*height;

  if ( data->GetMaxId()+1 != size)
    {
    vtkDebugMacro("Resizing array.");
    data->SetNumberOfComponents(3);
    data->SetNumberOfValues(size);
    }
  return this->GetPixelData(x1, y1, x2, y2, front, data->GetPointer(0));
  
}

int vtkOpenGLRenderWindow::GetPixelData(int x1, int y1,
                                        int x2, int y2,
                                        int front, unsigned char* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }

  glDisable( GL_SCISSOR_TEST );

#if defined(sparc) && !defined(GL_VERSION_1_2)
  // We need to read the image data one row at a time and convert it
  // from RGBA to RGB to get around a bug in Sun OpenGL 1.1
  long    xloop, yloop;
  unsigned char *buffer;
  unsigned char *p_data = NULL;
  
  buffer = new unsigned char [4*(x_hi - x_low + 1)];
  p_data = data;
  for (yloop = y_low; yloop <= y_hi; yloop++)
    {
    // read in a row of pixels
    glReadPixels(x_low,yloop,(x_hi-x_low+1),1,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    for (xloop = 0; xloop <= x_hi-x_low; xloop++)
      {
      *p_data = buffer[xloop*4]; p_data++;
      *p_data = buffer[xloop*4+1]; p_data++;
      *p_data = buffer[xloop*4+2]; p_data++;
      }
    }
  
  delete [] buffer;  
#else
  // If the Sun bug is ever fixed, then we could use the following
  // technique which provides a vast speed improvement on the SGI
  
  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_TEXTURE_2D );

  // Calling pack alignment ensures that we can grab the any size window
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glReadPixels(x_low, y_low, x_hi-x_low+1, y_hi-y_low+1, GL_RGB,
               GL_UNSIGNED_BYTE, data);
#endif

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
  
}

int vtkOpenGLRenderWindow::SetPixelData(int x1, int y1, int x2, int y2,
                                        vtkUnsignedCharArray *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  if (y1 < y2)
    {

    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  int width  = abs(x_hi - x_low) + 1;
  int height = abs(y_hi - y_low) + 1;
  int size = 3*width*height;

  if ( data->GetMaxId()+1 != size)
    {
    vtkErrorMacro("Buffer is of wrong size.");
    return VTK_ERROR;
    }
  return this->SetPixelData(x1, y1, x2, y2, data->GetPointer(0), front);

}

int vtkOpenGLRenderWindow::SetPixelData(int x1, int y1, int x2, int y2,
                                        unsigned char *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  // set the current window 
  this->MakeCurrent();

  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }

  if (y1 < y2)
    {

    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  glDisable( GL_SCISSOR_TEST );
  glViewport(0, 0, this->Size[0], this->Size[1]);

#if defined(sparc) && !defined(GL_VERSION_1_2)
  // We need to read the image data one row at a time and convert it
  // from RGBA to RGB to get around a bug in Sun OpenGL 1.1
  long    xloop, yloop;
  unsigned char *buffer;
  unsigned char *p_data = NULL;
  
  buffer = new unsigned char [4*(x_hi - x_low + 1)];

  // now write the binary info one row at a time
  glDisable(GL_BLEND);
  p_data = data;
  for (yloop = y_low; yloop <= y_hi; yloop++)
    {
    for (xloop = 0; xloop <= x_hi - x_low; xloop++)
      {
      buffer[xloop*4] = *p_data; p_data++;
      buffer[xloop*4+1] = *p_data; p_data++;
      buffer[xloop*4+2] = *p_data; p_data++;
      buffer[xloop*4+3] = 0xff;
      }
    /* write out a row of pixels */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1),
                   (2.0 * (GLfloat)(yloop) / this->Size[1] - 1),
                   -1.0 );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glDrawPixels((x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
  
  // This seems to be necessary for the image to show up
  glFlush();  
  
  glEnable(GL_BLEND);
#else
  // If the Sun bug is ever fixed, then we could use the following
  // technique which provides a vast speed improvement on the SGI
  
  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_TEXTURE_2D );

  // now write the binary info
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1), 
                 (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
                 -1.0 );
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
  glDisable(GL_BLEND);
  glDrawPixels((x_hi-x_low+1), (y_hi - y_low + 1),
               GL_RGB, GL_UNSIGNED_BYTE, data);
  glEnable(GL_BLEND);
  
  // This seems to be necessary for the image to show up
  glFlush();  
#endif
  
  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}

float* vtkOpenGLRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2,
                                               int front)
{

  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  float *data = new float[ (width*height*4) ];
  this->GetRGBAPixelData(x1, y1, x2, y2, front, data);

  return data;

}

int vtkOpenGLRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2,
                                            int front, vtkFloatArray* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  int size = 4*width*height;
  
  if ( data->GetMaxId()+1 != size)
    {
    vtkDebugMacro("Resizing array.");
    data->SetNumberOfComponents(4);
    data->SetNumberOfValues(size);
    }
  return this->GetRGBAPixelData(x1, y1, x2, y2, front, data->GetPointer(0));

}

int vtkOpenGLRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2,
                                            int front, float* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;


  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_TEXTURE_2D );

  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_FLOAT, data);

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}

void vtkOpenGLRenderWindow::ReleaseRGBAPixelData(float *data) 
{
  delete[] data;
}

int vtkOpenGLRenderWindow::SetRGBAPixelData(int x1, int y1, int x2, int y2,
                                            vtkFloatArray *data, int front,
                                            int blend)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }
  
  width  = abs(x_hi-x_low) + 1;
  height = abs(y_hi-y_low) + 1;

  int size = 4*width*height;
  if ( data->GetMaxId()+1 != size )
    {
    vtkErrorMacro("Buffer is of wrong size.");
    return VTK_ERROR;
    }

  return this->SetRGBAPixelData(x1, y1, x2, y2, data->GetPointer(0), front,
                                blend);
}

int vtkOpenGLRenderWindow::SetRGBAPixelData(int x1, int y1, int x2, int y2,
                                            float *data, int front, int blend)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  // set the current window 
  this->MakeCurrent();

  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }
  
  width  = abs(x_hi-x_low) + 1;
  height = abs(y_hi-y_low) + 1;

  /* write out a row of pixels */
  glDisable( GL_SCISSOR_TEST );
  glViewport(0, 0, this->Size[0], this->Size[1]);
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1), 
                 (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
                 -1.0 );
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1);

  if (!blend)
    {
    glDisable(GL_BLEND);
    glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);
    glEnable(GL_BLEND);
    }
  else
    {
    glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);
    }
  
  // This seems to be necessary for the image to show up
  glFlush();  

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}

unsigned char *vtkOpenGLRenderWindow::GetRGBACharPixelData(int x1, int y1, 
                                                           int x2, int y2, 
                                                           int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  unsigned char *data = new unsigned char[ (width*height)*4 ];
  this->GetRGBACharPixelData(x1, y1, x2, y2, front, data);

  return data;
}

int vtkOpenGLRenderWindow::GetRGBACharPixelData(int x1, int y1,
                                                int x2, int y2,
                                                int front,
                                                vtkUnsignedCharArray* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  int width  = abs(x_hi - x_low) + 1;
  int height = abs(y_hi - y_low) + 1;
  int size = 4*width*height;

  if ( data->GetMaxId()+1 != size)
    {
    vtkDebugMacro("Resizing array.");
    data->SetNumberOfComponents(4);
    data->SetNumberOfValues(size);
    }
  return this->GetRGBACharPixelData(x1, y1, x2, y2, front,
                                    data->GetPointer(0));
}

int vtkOpenGLRenderWindow::GetRGBACharPixelData(int x1, int y1,
                                                int x2, int y2,
                                                int front,
                                                unsigned char* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;


  // set the current window
  this->MakeCurrent();


  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }


  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  glDisable( GL_SCISSOR_TEST );
  
  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_TEXTURE_2D );

  glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                data);

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }

}


int vtkOpenGLRenderWindow::SetRGBACharPixelData(int x1,int y1,int x2,int y2,
                                                vtkUnsignedCharArray *data,
                                                int front, int blend)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }
  
  width  = abs(x_hi-x_low) + 1;
  height = abs(y_hi-y_low) + 1;

  int size = 4*width*height;
  if ( data->GetMaxId()+1 != size )
    {
    vtkErrorMacro("Buffer is of wrong size. It is " << data->GetMaxId()+1
                  << ", it should be: " << size);
    return VTK_ERROR;
    }

  return this->SetRGBACharPixelData(x1, y1, x2, y2, data->GetPointer(0),
                                    front, blend);
  
}

int vtkOpenGLRenderWindow::SetRGBACharPixelData(int x1, int y1, int x2,
                                                int y2, unsigned char *data,
                                                int front, int blend)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;


  // set the current window
  this->MakeCurrent();


  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }


  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }


  width  = abs(x_hi-x_low) + 1;
  height = abs(y_hi-y_low) + 1;


  /* write out a row of pixels */
  glViewport(0, 0, this->Size[0], this->Size[1]);
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1),
                 (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
                 -1.0 );
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();


  glDisable( GL_SCISSOR_TEST );
  
  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_TEXTURE_2D );

  if (!blend)
    {
    glDisable(GL_BLEND);
    glDrawPixels( width, height, GL_RGBA, GL_UNSIGNED_BYTE, 
                  data);
    glEnable(GL_BLEND);
    }
  else
    {
    glDrawPixels( width, height, GL_RGBA, GL_UNSIGNED_BYTE, 
                  data);
    }

  // This seems to be necessary for the image to show up
  glFlush();  

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}


int vtkOpenGLRenderWindow::GetZbufferData( int x1, int y1, int x2, int y2,
                                           float* z_data )
{
  int             y_low;
  int             x_low;
  int             width, height;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    }
  else
    {
    y_low = y2; 
    }

  if (x1 < x2)
    {
    x_low = x1; 
    }
  else
    {
    x_low = x2; 
    }

  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_SCISSOR_TEST );
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );

  glReadPixels( x_low, y_low, 
                width, height,
                GL_DEPTH_COMPONENT, GL_FLOAT,
                z_data );

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}

float *vtkOpenGLRenderWindow::GetZbufferData( int x1, int y1, int x2, int y2  )
{
  float           *z_data;

  int             width, height;
  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  z_data = new float[width*height];
  this->GetZbufferData(x1, y1, x2, y2, z_data);

  return z_data;
}

int vtkOpenGLRenderWindow::GetZbufferData( int x1, int y1, int x2, int y2,
                                           vtkFloatArray *buffer )
{
  int  width, height;
  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;
  int size = width*height;
  if ( buffer->GetMaxId()+1 != size)
    {
    vtkDebugMacro("Resizing array.");
    buffer->SetNumberOfComponents(1);
    buffer->SetNumberOfValues(size);
    }
  return this->GetZbufferData(x1, y1, x2, y2, buffer->GetPointer(0));
}

int vtkOpenGLRenderWindow::SetZbufferData( int x1, int y1, int x2, int y2,
                                           vtkFloatArray *buffer )
{
  int width, height;
  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;
  int size = width*height;
  if ( buffer->GetMaxId()+1 != size )
    {
    vtkErrorMacro("Buffer is of wrong size.");
    return VTK_ERROR;
    }
  return this->SetZbufferData(x1, y1, x2, y2, buffer->GetPointer(0));
}

int vtkOpenGLRenderWindow::SetZbufferData( int x1, int y1, int x2, int y2,
                                           float *buffer )
{
  int             y_low;
  int             x_low;
  int             width, height;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    }
  else
    {
    y_low = y2; 
    }

  if (x1 < x2)
    {
    x_low = x1; 
    }
  else
    {
    x_low = x2; 
    }

  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR);

  glViewport(0, 0, this->Size[0], this->Size[1]);
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos2f( 2.0 * (GLfloat)(x_low) / this->Size[0] - 1, 
                 2.0 * (GLfloat)(y_low) / this->Size[1] - 1);
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  // Turn of texturing in case it is on - some drivers have a problem
  // getting / setting pixels with texturing enabled.
  glDisable( GL_SCISSOR_TEST );
  glDisable( GL_TEXTURE_2D );
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );

  glDrawPixels( width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

  // This seems to be necessary for the image to show up
  glFlush();  

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}


void vtkOpenGLRenderWindow::RegisterTextureResource (GLuint id)
{
  this->TextureResourceIds->InsertNextId ((int) id);
}

// ----------------------------------------------------------------------------
// Description:
// Create an offScreen window based on OpenGL framebuffer extension.
// Return if the creation was successful or not.
// \pre positive_width: width>0
// \pre positive_height: height>0
// \pre not_initialized: !OffScreenUseFrameBuffer
// \post valid_result: (result==0 || result==1)
//                     && (result implies OffScreenUseFrameBuffer)
int vtkOpenGLRenderWindow::CreateHardwareOffScreenWindow(int width, int height)
{
  assert("pre: positive_width" && width>0);
  assert("pre: positive_height" && height>0);
  assert("pre: not_initialized" && !this->OffScreenUseFrameBuffer);
  
  // 1. create a regular OpenGLcontext (ie create a window)
  this->CreateAWindow();
  this->MakeCurrent();
  
  // 2. check for OpenGL extensions Gl_EXT_framebuffer_object and
  // GL_ARB_texture_non_power_of_two
  vtkOpenGLExtensionManager *extensions=vtkOpenGLExtensionManager::New();
  extensions->SetRenderWindow(this);
  
  int supports_GL_EXT_framebuffer_object=extensions->ExtensionSupported("GL_EXT_framebuffer_object");
  int supports_GL_ARB_texture_non_power_of_two=extensions->ExtensionSupported("GL_ARB_texture_non_power_of_two");
  int supports_GL_ARB_draw_buffers=extensions->ExtensionSupported("GL_ARB_draw_buffers");
  int result=0;
  
  if(!(supports_GL_EXT_framebuffer_object && supports_GL_ARB_texture_non_power_of_two && supports_GL_ARB_draw_buffers))
    {
    if(!supports_GL_EXT_framebuffer_object)
      {
      vtkDebugMacro(<<" extension GL_EXT_framebuffer_object is not supported. Hardware accelerated offscreen rendering is not available");
      }
    if(!supports_GL_ARB_texture_non_power_of_two)
      {
      vtkDebugMacro(<<" extension GL_ARB_texture_non_power_of_two is not supported. Hardware accelerated offscreen rendering is not available");
      }
    if(!supports_GL_ARB_draw_buffers)
      {
      vtkDebugMacro(<<" extension GL_ARB_draw_buffers is not supported. Hardware accelerated offscreen rendering is not available");
      }
    this->DestroyWindow();
    }
  else
    {
    result=1;
    extensions->LoadExtension("GL_EXT_framebuffer_object");
    extensions->LoadExtension("GL_ARB_texture_non_power_of_two");
    extensions->LoadExtension("GL_ARB_draw_buffers");
    
    // 3. regular framebuffer code
    this->NumberOfFrameBuffers=1;
    GLboolean flag;
    glGetBooleanv(GL_DOUBLEBUFFER,&flag);
    if(flag)
      {
      this->NumberOfFrameBuffers<<=1;
      }
    glGetBooleanv(GL_STEREO,&flag);
    if(flag)
      {
      this->NumberOfFrameBuffers<<=1;
      }
    
    // Up to 4: double buffer and stereo
    GLuint textureObjects[4];
    
    GLuint frameBufferObject;
    GLuint depthRenderBufferObject;
    vtkgl::GenFramebuffersEXT(1, &frameBufferObject); // color
    vtkgl::GenRenderbuffersEXT(1, &depthRenderBufferObject); // depth
    int i=0;
    while(i<this->NumberOfFrameBuffers)
      {
      textureObjects[i]=0;
      ++i;
      }
    glGenTextures(this->NumberOfFrameBuffers,textureObjects);
    // Color buffers
    vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,frameBufferObject);
    i=0;
    while(i<this->NumberOfFrameBuffers)
      {
      glBindTexture(GL_TEXTURE_2D,textureObjects[i]);
      vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT,
                                     vtkgl::COLOR_ATTACHMENT0_EXT+i,
                                     GL_TEXTURE_2D, textureObjects[i], 0);
      ++i;
      }
    
    // Depth buffer
    vtkgl::BindRenderbufferEXT(vtkgl::RENDERBUFFER_EXT,
                               depthRenderBufferObject);
    vtkgl::FramebufferRenderbufferEXT(vtkgl::FRAMEBUFFER_EXT,
                                      vtkgl::DEPTH_ATTACHMENT_EXT,
                                      vtkgl::RENDERBUFFER_EXT,
                                      depthRenderBufferObject);
    
    // Restore default frame buffer.
    vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,0);
    
    i=0;
    while(i<this->NumberOfFrameBuffers)
      {
      glBindTexture(GL_TEXTURE_2D,textureObjects[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
      ++i;
      }
    // Set up the depth render buffer
    vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,frameBufferObject);
    vtkgl::BindRenderbufferEXT(vtkgl::RENDERBUFFER_EXT,depthRenderBufferObject);
    vtkgl::RenderbufferStorageEXT(vtkgl::RENDERBUFFER_EXT,
                                  vtkgl::DEPTH_COMPONENT24,width,height);
    vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,0);
    
    vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,frameBufferObject);
    GLenum buffer[4];
    buffer[0] = vtkgl::COLOR_ATTACHMENT0_EXT;
    buffer[1] = vtkgl::COLOR_ATTACHMENT1_EXT;
    buffer[2] = vtkgl::COLOR_ATTACHMENT2_EXT;
    buffer[3] = vtkgl::COLOR_ATTACHMENT3_EXT;
    vtkgl::DrawBuffersARB(this->NumberOfFrameBuffers,buffer);
    
    // Save GL objects by static casting to standard C types. GL* types
    // are not allowed in VTK header files.
    this->FrameBufferObject=static_cast<unsigned int>(frameBufferObject);
    this->DepthRenderBufferObject=static_cast<unsigned int>(depthRenderBufferObject);
    i=0;
    while(i<this->NumberOfFrameBuffers)
      {
      this->TextureObjects[i]=static_cast<unsigned int>(textureObjects[i]);
      ++i;
      }
    
    this->OffScreenUseFrameBuffer=1;
    }
  extensions->Delete();

  // A=>B = !A || B
  assert("post: valid_result" && (result==0 || result==1) && (!result || OffScreenUseFrameBuffer));
  return result;
}

// ----------------------------------------------------------------------------
// Description:
// Destroy an offscreen window based on OpenGL framebuffer extension.
// \pre initialized: OffScreenUseFrameBuffer
// \post destroyed: !OffScreenUseFrameBuffer
void vtkOpenGLRenderWindow::DestroyHardwareOffScreenWindow()
{
  assert("pre: initialized" && this->OffScreenUseFrameBuffer);
  
  this->MakeCurrent();
  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, 0 );
  
  GLuint frameBufferObject=static_cast<GLuint>(this->FrameBufferObject);
  vtkgl::DeleteFramebuffersEXT(1,&frameBufferObject);
  
  GLuint depthRenderBufferObject=static_cast<GLuint>(this->DepthRenderBufferObject);
  vtkgl::DeleteRenderbuffersEXT(1,&depthRenderBufferObject);
  
  GLuint textureObjects[4];
  int i=0;
  while(i<this->NumberOfFrameBuffers)
    {
    textureObjects[i]=static_cast<GLuint>(this->TextureObjects[i]);
    ++i;
    }
  
  glDeleteTextures(this->NumberOfFrameBuffers,textureObjects);
  this->DestroyWindow();
  
  this->OffScreenUseFrameBuffer=0;
  
  assert("post: destroyed" && !this->OffScreenUseFrameBuffer);
}
