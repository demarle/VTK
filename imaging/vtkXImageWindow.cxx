/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXImageWindow.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkXImageWindow.h"


void vtkXImageWindow::GetShiftsAndMasks(int &rshift, int &gshift, 
					int &bshift,
					unsigned long &rmask,
					unsigned long &gmask,
					unsigned long &bmask)
{
  XWindowAttributes winAttribs;
  XVisualInfo temp1;

  XGetWindowAttributes(this->DisplayId, this->WindowId, &winAttribs);
  temp1.visualid = winAttribs.visual->visualid;
  int nvisuals = 0;
  XVisualInfo* visuals = 
    XGetVisualInfo(this->DisplayId, VisualIDMask, &temp1, &nvisuals);   
  if (nvisuals == 0)  vtkErrorMacro(<<"Could not get color masks");
  
  rmask = visuals->red_mask;
  gmask = visuals->green_mask;
  bmask = visuals->blue_mask;
  
  XFree(visuals);

  // Compute the shifts needed to align the color masks with the 
  // pixels
  rshift = 0;
  gshift = 0;
  bshift = 0;
  unsigned long tmp;

  tmp = rmask;
  while (tmp != 0)
    {
    tmp = tmp >> 1;
    rshift++;
    }
  rshift -= 8;
  tmp = gmask;
  while (tmp != 0)
    {
    tmp = tmp >> 1;
    gshift++;
    }
  gshift -= 8;
  tmp = bmask;
  while (tmp != 0)
    {
    tmp = tmp >> 1;
    bshift++;
    }
  bshift -= 8;
}


unsigned char *vtkXImageWindow::GetPixelData(int x1, int y1, 
					     int x2, int y2, int)
{
  vtkDebugMacro (<< "Getting pixel data...");

  int width  = (abs(x2 - x1)+1);
  int height = (abs(y2 - y1)+1);
  int rshift, gshift, bshift;
  unsigned long rmask, gmask, bmask;

  this->GetShiftsAndMasks(rshift,gshift,bshift,rmask,gmask,bmask);

  // Get the XImage
  XImage* image = XGetImage(this->DisplayId, this->WindowId, x1, y1,
			    width, height, AllPlanes, XYPixmap);

  // Allocate space for the data
  unsigned char*  data = new unsigned char[width*height*3];
  if (!data) 
    {
    vtkErrorMacro(<< "Failed to malloc space for pixel data!");
    return (unsigned char*) NULL;
    }

  // Pointer to index through the data
  unsigned char* p_data = data;

  // Set up the loop indices
  int yloop = 0;
  int xloop = 0;
  unsigned long pixel = 0; 
  int y_low = 0;
  int y_hi = 0;
  int x_low = 0;
  int x_hi = 0;

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

  for (yloop = y_hi; yloop >= y_low; yloop--)
    {
    for (xloop = x_low; xloop <= x_hi ; xloop++)
      {
      pixel = XGetPixel(image, xloop, yloop);
      *p_data = (pixel & rmask) >> rshift; p_data++;
      *p_data = (pixel & gmask) >> gshift; p_data++;
      *p_data = (pixel & bmask) >> bshift; p_data++;
      }
    }
 
  XDestroyImage(image);
  
  return data;
}

void vtkXImageWindow::SwapBuffers()
{
  static int swapFlag = 0;

  if (swapFlag == 0)
    {
    swapFlag = 1;
    }
  else
    {
    XCopyArea(this->DisplayId, this->Drawable, this->WindowId, this->Gc, 
	      0, 0, this->Size[0], this->Size[1], 0, 0);
    XSync(this->DisplayId, False);
    XFlush(this->DisplayId);
    swapFlag = 0;
    }

}

void *vtkXImageWindow::GetGenericDrawable()
{
  if (this->DoubleBuffer)
    {
    if (!this->Drawable)
      {
      this->Drawable = XCreatePixmap(this->DisplayId, this->WindowId, 
				     this->Size[0], this->Size[1],
				     this->VisualDepth);
      this->PixmapWidth = this->Size[0];
      this->PixmapHeight = this->Size[1];
      }
    else if ((this->PixmapWidth != this->Size[0]) || 
	     (this->PixmapHeight != this->Size[1]))
      {
      XFreePixmap(this->DisplayId, this->Drawable);
      this->Drawable = XCreatePixmap(this->DisplayId, this->WindowId, 
				      this->Size[0], this->Size[1],
				      this->VisualDepth);       
      this->PixmapWidth = this->Size[0];
      this->PixmapHeight = this->Size[1];
      }
    return (void *) this->Drawable;
    }
  else
    {
    return (void *) this->WindowId;
    }

}


//----------------------------------------------------------------------------
vtkXImageWindow::vtkXImageWindow()
{
  vtkDebugMacro(<< "vtkXImageWindow::vtkXImageWindow");
  this->DisplayId = (Display *)NULL;
  this->WindowId = (Window)(NULL);
  this->ParentId = (Window)(NULL);
  this->ColorMap = (Colormap) NULL;
  this->Drawable = (Pixmap) NULL;
  this->IconPixmap = (Pixmap) NULL;
  this->Offset = 0;
  this->OwnDisplay = 0;
  this->PixmapWidth = 0;
  this->PixmapHeight = 0;
}


//----------------------------------------------------------------------------
vtkXImageWindow::~vtkXImageWindow()
{
  vtkDebugMacro(<< "vtkXImageWindow::vtkXImageWindow");

  /* free the Xwindow we created no need to free the colormap */
  if (this->DisplayId && this->WindowId && this->WindowCreated)
    {
    XFreeGC(this->DisplayId, this->Gc);
    XDestroyWindow(this->DisplayId,this->WindowId);
    }
  if (this->DisplayId)
    {
    XSync(this->DisplayId,0);
    }
  if (this->OwnDisplay && this->DisplayId)
    {
    XCloseDisplay(this->DisplayId);
    }
}


//----------------------------------------------------------------------------
void vtkXImageWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageWindow::PrintSelf(os, indent);
  os << indent << "Parent Id: " << this->ParentId << "\n";
  os << indent << "Window Id: " << this->WindowId << "\n";
  os << indent << "Display Id: " << this->DisplayId << "\n";
  os << indent << "Visual Id: " << this->VisualId << "\n";
  os << indent << "Visual Depth: " << this->VisualDepth << "\n";
  os << indent << "Visual Class: " << this->VisualClass << "\n";
  os << indent << "ColorMap: " << this->ColorMap << "\n";
  os << indent << "GC: " << this->Gc << "\n";
  os << indent << "Icon Pixmap: " << this->IconPixmap << "\n";
  os << indent << "Offset: " << this->Offset << "\n";
  os << indent << "Colors: " << this->Colors << "\n";
  os << indent << "Number Of Colors: " << this->NumberOfColors << "\n";
  os << indent << "Drawable: " << this->Drawable << "\n";

}


void vtkXImageWindow::SetBackgroundColor(float r, float g, float b)
{
  unsigned long background = 0;

  // I think these need to be unsigned char
  unsigned short red = 0;
  unsigned short green = 0;
  unsigned short blue = 0;

  // Check if colors are > 1 ???
  red = (unsigned short) (r * 255.0);
  green = (unsigned short) (g * 255.0);
  blue = (unsigned short)  (b * 255.0);

  background = background | (blue << 16);
  background = background | (green << 8);
  background = background | red;

  vtkDebugMacro(<<"vtkXImageWindow::SetBackgroundColor - value: " << background);

  vtkDebugMacro(<<"vtkXImageWindow::SetBackgroundColor - red: " << red << ", green: " << green <<
  ", blue: " << blue);

  // what should I do if a window has not been created (lawcc dfss)
  if (this->WindowId == (Window)(NULL))
    {
    this->MakeDefaultWindow();
    }
  
  XSetWindowBackground(this->DisplayId, this->WindowId, background);

  XClearWindow(this->DisplayId, this->WindowId);

  XFlush(this->DisplayId);
   
  XSync(this->DisplayId, False); 
}

void vtkXImageWindow::EraseWindow()
{
  
  // what should I do if a window has not been created (lawcc dfss)
  if (this->WindowId == (Window)(NULL))
    {
    this->MakeDefaultWindow();
    }
  
  // If double buffering is on and we don't have a drawable
  // yet, then we better make one
  if (this->DoubleBuffer && !this->Drawable)
    {
    this->GetGenericDrawable();
    }

  // Erase the drawable if double buffering is on
  // and the drawable exists
  if (this->DoubleBuffer && this->Drawable)
    {
vtkWarningMacro ("EraseWindow");
    // Get the old foreground and background
    XGCValues vals;
    XGetGCValues(this->DisplayId, this->Gc, GCForeground, &vals);
    unsigned long oldForeground = vals.foreground;

    // Set the foreground color to the background so the rectangle
    // matches the background color
    XColor aColor;
    aColor.red = 65535;
    aColor.green = 0;
    aColor.blue = 0;
    XAllocColor(this->DisplayId,this->ColorMap,&aColor);
    XSetForeground(this->DisplayId, this->Gc, aColor.pixel);
    XFillRectangle(this->DisplayId, this->Drawable, this->Gc, 0, 0, 
	           this->Size[0], this->Size[1]);

    // Reset the foreground to it's previous color
    XSetForeground (this->DisplayId, this->Gc, oldForeground);
    }
  // otherwise, erase the window
  else
    {
    XClearWindow(this->DisplayId,this->WindowId);
    XFlush(this->DisplayId);
    }
}


// Get this RenderWindow's X window id.
Window vtkXImageWindow::GetWindowId()
{
//  vtkDebugMacro(<< "vtkXImageWindow::GetWindowID ");

  return this->WindowId;
}

// Get this RenderWindow's parent X window id.
Window vtkXImageWindow::GetParentId()
{
//  vtkDebugMacro(<< "vtkXImageWindow::GetParentID ");
  return this->ParentId;
}

// Sets the parent of the window that WILL BE created.
void vtkXImageWindow::SetParentId(Window arg)
{
  if (this->ParentId)
    {
    vtkErrorMacro("ParentId is already set.");
    return;
    }
  
//  vtkDebugMacro(<< "vtkXImageWindow::SetParentId ");

  this->ParentId = arg;
}

void vtkXImageWindow::SetParentId(void* arg)
{
  this->SetParentId((Window)arg);
}


// Get the position in screen coordinates (pixels) of the window.
int *vtkXImageWindow::GetPosition(void)
{
  XWindowAttributes attribs;
  int x,y;
  Window child;
 
  // what should I do if a window has not been created (lawcc dfss)
  if (this->WindowId == (Window)(NULL))
    {
    this->MakeDefaultWindow();
    }
 
  // if we aren't mapped then just return the ivar 
  if ( ! this->Mapped)
    {
    return(this->Position);
    }

  //  Find the current window size 
  XGetWindowAttributes(this->DisplayId, this->WindowId, &attribs);
  x = attribs.x;
  y = attribs.y;

  XTranslateCoordinates(this->DisplayId,this->WindowId,
			RootWindowOfScreen(ScreenOfDisplay(this->DisplayId,0)),
			x,y,&this->Position[0],&this->Position[1],&child);

  return this->Position;
}

// Move the window to a new position on the display.
void vtkXImageWindow::SetPosition(int x, int y)
{
  // what should I do if a window has not been created (lawcc dfss)
  if (this->WindowId == (Window)(NULL))
    {
    this->MakeDefaultWindow();
    }
  
  // if we aren't mapped then just set the ivars
  if (!this->Mapped)
    {
    if ((this->Position[0] != x)||(this->Position[1] != y))
      {
      this->Modified();
      }
    this->Position[0] = x;
    this->Position[1] = y;
    return;
    }

  XMoveResizeWindow(this->DisplayId,this->WindowId,x,y,
                    this->Size[0], this->Size[1]);
  XSync(this->DisplayId,False);
}


void vtkXImageWindow::SetSize(int x, int y)
{
//  vtkDebugMacro (<< "vtkXImageWindow::SetSize");

  // If the values have changed, change the ivars
  if ((this->Size[0] != x) || (this->Size[1] != y))
    {
    this->Modified();
    this->Size[0] = x;
    this->Size[1] = y;

    }

  // If the window isn't displayed, return
  if (!this->Mapped)
    {
    return;
    }

   // Make the X window call 
   XResizeWindow(this->DisplayId, this->WindowId, this->Size[0], this->Size[1]);


   // Need the XFlush to clear the X queue or else there are X timing problems
   // i.e. The first image may not display correctly
   XFlush(this->DisplayId);
   
   XSync(this->DisplayId, False); 

   //this->Render();

}


int* vtkXImageWindow::GetSize()
{
  XWindowAttributes attribs;
 
  vtkDebugMacro (<< "vtkXImageWindow::GetSize");
 
  // if we aren't mapped then just return the ivar 
  if ( ! this->Mapped)
    {
    vtkDebugMacro (<< "vtkXImageWindow::GetSize - Window not mapped");
    return(this->Size);
    }

  //  Find the current window size 
  XFlush(this->DisplayId);
  XSync(this->DisplayId, False);

  XGetWindowAttributes(this->DisplayId, this->WindowId, &attribs);


  // May want to put in call to update Size ivars if different than 
  // what X thinks the size is

  this->Size[0] = attribs.width;
  this->Size[1] = attribs.height;

  return this->Size; 

}

//----------------------------------------------------------------------------
void vtkXImageWindow::MakeDefaultWindow() 
{
  char name[80];
  int screen;
  XVisualInfo info;
  XSetWindowAttributes values;
  Window window;
  XSizeHints xsh;
  int x, y, width, height;

  vtkDebugMacro (<< "vtkXImageWindow::MakeDefaultWindow" ); 
  
  strcpy(name,"vtk - X Viewer Window");

  // make sure we have a connection to the X server.
  if ( ! this->DisplayId)
    {
    if ( ( this->DisplayId = XOpenDisplay((char *)NULL)) == NULL) 
      {
      cerr <<"cannot connect to X server"<< XDisplayName((char *)NULL)<< endl;
      exit(-1);
      }
    this->OwnDisplay = 1;
    }
  
  
  screen = DefaultScreen(this->DisplayId);
  this->GetDefaultVisualInfo(&info);
  
  // Create a window 
  // If this is a pseudocolor visual, create a color map.
  values.colormap = this->GetDesiredColormap();
  
  XColor aColor;
  aColor.red = 0;
  aColor.green = 0;
  aColor.blue = 0;
  XAllocColor(this->DisplayId,values.colormap,&aColor);
  values.background_pixel = aColor.pixel;
  values.border_pixel = None;
  values.event_mask = 0;
  values.override_redirect = False;
  //  if ((w > 0) && (x >= 0) && (!borders))
  //  values.override_redirect = True;
  XFlush(this->DisplayId);

  // get a default parent if one has not been set.
  if (! this->ParentId)
    {
    this->ParentId = RootWindow(this->DisplayId, screen);
    }

  // if size not set use default of 256
  if (this->Size[0] == 0) 
    {
    this->Size[0] = 256;
    this->Size[1] = 256;
    }
    
  xsh.flags = USSize;
  if ((this->Position[0] >= 0)&&(this->Position[1] >= 0))
    {
    xsh.flags |= USPosition;
    xsh.x =  (int)(this->Position[0]);
    xsh.y =  (int)(this->Position[1]);
    }
  
  x = ((this->Position[0] >= 0) ? this->Position[0] : 5);
  y = ((this->Position[1] >= 0) ? this->Position[1] : 5);
  width = ((this->Size[0] > 0) ? this->Size[0] : 300);
  height = ((this->Size[1] > 0) ? this->Size[1] : 300);

  xsh.width  = width;
  xsh.height = height;

  window = XCreateWindow(this->DisplayId, this->ParentId,
			 x, y, width, height, 0, info.depth, 
			 InputOutput, info.visual,
			 CWEventMask | CWBackPixel | CWBorderPixel | 
			 CWColormap | CWOverrideRedirect, 
			 &values);
  XSetStandardProperties(this->DisplayId, window, name, name, None, 0, 0, 0);
  XSetNormalHints(this->DisplayId,window,&xsh);

  XSync(this->DisplayId, False);
  
  // Select event types wanted 
  XSelectInput(this->DisplayId, window,
	       ExposureMask | KeyPressMask | ButtonPressMask |
	       PointerMotionMask | StructureNotifyMask | PropertyChangeMask);
  
  // Map Window onto Screen and sysc
  XMapWindow(this->DisplayId, window);
  
  XSync(this->DisplayId,0);
 
// ####

  XVisualInfo templ;
  XVisualInfo *visuals;
  int nvisuals;
  XWindowAttributes attributes;
 
  this->WindowId = (Window)(window);
 
  // Create a graphics contect for this window
  this->Gc = XCreateGC(this->DisplayId, this->WindowId, 0, NULL);
  XSetForeground(this->DisplayId, this->Gc, 0XFFFFFF);
  XSetBackground(this->DisplayId, this->Gc, 0X000000);

  // Get the visual
  if ( ! XGetWindowAttributes(this->DisplayId, this->WindowId, &attributes))
    {
    vtkErrorMacro(<< "SetWindow: Could not get window attributes.");
    return;
    }
  this->VisualId = attributes.visual;
  this->VisualDepth = attributes.depth;
  this->ColorMap = attributes.colormap;
 
  //####
  if (this->ColorMap == None) 
           vtkDebugMacro(<<"vtkXImageWindow::MakeDefaultWindow - No colormap!");
  if (attributes.map_installed == False) 
           vtkDebugMacro(<<"vtkXImageWindow::MakeDefaultWindow - Colormap not installed!");

  // Get the visual class
  templ.visualid = this->VisualId->visualid;
  visuals = XGetVisualInfo(this->DisplayId,
                           VisualIDMask,
                           &templ, &nvisuals);
  if (nvisuals == 0)
    {
    vtkErrorMacro(<< "Could not get visual class");
    }
  this->VisualClass = visuals->c_class;

  XFree(visuals);

  // Make sure the color map is set up properly.
  if (this->VisualClass == DirectColor)
    {
    vtkDebugMacro(<< "vtkXImageWindow::MakeDefaultWindow - Allocating direct color map");
    this->AllocateDirectColorMap();
    }

  this->Mapped = 1;
  this->WindowCreated = 1;


// ####
 
  return;
}


//----------------------------------------------------------------------------
void vtkXImageWindow::GetDefaultVisualInfo(XVisualInfo *info) 
{
  int screen;
  XVisualInfo templ;
  XVisualInfo *visuals, *v;
  XVisualInfo *best = NULL;
  int nvisuals;
  int i, rate, bestRate = 100;
  
//  vtkDebugMacro (<< "vtkImageWindow::GetDefaultVisualInfo" );

  screen = DefaultScreen(this->DisplayId);  
  templ.screen = screen;
  //templ.depth = 24;
  //templ.c_class = DirectColor;

  // Get a list of all the possible visuals for this screen.
  visuals = XGetVisualInfo(this->DisplayId,
			   // VisualScreenMask | VisualClassMask,
			   VisualScreenMask,
			   &templ, &nvisuals);
  
  if (nvisuals == 0)
    {
    vtkErrorMacro(<< "Could not get a visual");
    }
  
  for (v = visuals, i = 0; i < nvisuals; v++, i++)
    {
    // which are available

#if 0
    if (this->Debug)
      {
      if (v->c_class == TrueColor)
	vtkDebugMacro(<< "Available: " << v->depth << " bit TrueColor");
      if (v->c_class == DirectColor)
	vtkDebugMacro(<< "Available: " << v->depth << " bit DirectColor");
      if (v->c_class == PseudoColor)
	vtkDebugMacro(<< "Available: " << v->depth << " bit PseudoColor");
      }
#endif 


    // We only handle three types of visuals now.
    // Rate the visual
    if (v->depth == 24 && v->c_class == TrueColor)
      {
      rate = 1;
      }
    else if (v->depth == 24 && v->c_class == DirectColor)
      {
      rate = 2;
      }
    else if (v->depth == 8 && v->c_class == PseudoColor)
      {
      rate = 3;
      }
    else
      {
      rate = 50;
      }
    
    if (rate < bestRate)
      {
      bestRate = rate;
      best = v;
      }
    }

  if (bestRate >= 50)
    {
    vtkWarningMacro("Could not find a visual I like");
    }
 
#if 0 
  if (this->Debug)
    {
    if (best->c_class == TrueColor)
      vtkDebugMacro(<< "Chose: " << best->depth << " bit TrueColor");
    if (best->c_class == DirectColor)
      vtkDebugMacro(<< "Chose: " << best->depth << " bit DirectColor");
    if (best->c_class == PseudoColor)
      vtkDebugMacro(<< "Chose: " << best->depth << " bit PseudoColor");
    }
#endif

  
  // Copy visual
  *info = *best;
  
  XFree(visuals);
}

int vtkXImageWindow::GetDesiredDepth()
{
  XVisualInfo v;

//  vtkDebugMacro (<< "vtkXImageWindow::GetDesiredDepth");

  // get the default visual to use 
  this->GetDefaultVisualInfo(&v);

  return v.depth;  
}

// Get a visual from the windowing system.
Visual *vtkXImageWindow::GetDesiredVisual ()
{
  XVisualInfo v;

//  vtkDebugMacro (<< "vtkXImageWindow::GetDesiredVisual");

  // get the default visual to use 
  this->GetDefaultVisualInfo(&v);

  return v.visual;  
}


// Get a colormap from the windowing system.
Colormap vtkXImageWindow::GetDesiredColormap ()
{
  XVisualInfo v;

//  vtkDebugMacro (<< "vtkXImageWindow::GetDesiredColorMap");

  if (this->ColorMap) return this->ColorMap;
  
  // get the default visual to use 
  this->GetDefaultVisualInfo(&v);

  if (v.depth == 8)
    {
    this->ColorMap = this->MakeColorMap(v.visual);
    }
  else
    {
    this->ColorMap = 
      XCreateColormap(this->DisplayId, RootWindow(this->DisplayId, v.screen),
		      v.visual, AllocNone);
    }
  
  return this->ColorMap;  
}


void vtkXImageWindow::SetWindowId(void *arg)
{
//  vtkDebugMacro (<< "vtkXImageWindow::SetWindowId");

  this->SetWindowId((Window)arg);
}

void vtkXImageWindow::SetWindowId(Window arg)
{
  arg = arg;
  // Here to allow me to compile
}

// Set the X display id for this ImageXWindow to use to a pre-existing 
// X display id.
void vtkXImageWindow::SetDisplayId(Display  *arg)
{
//  vtkDebugMacro(<< "Setting DisplayId to " << (void *)arg << "\n"); 

  this->DisplayId = arg;
  this->OwnDisplay = 0;
}
void vtkXImageWindow::SetDisplayId(void *arg)
{
//  vtkDebugMacro(<< "Setting DisplayId to " << (void *)arg << "\n"); 
  this->SetDisplayId((Display *)arg);
  this->OwnDisplay = 0;
}



Display *vtkXImageWindow::GetDisplayId()
{
//  vtkDebugMacro(<< "vtkXImageWindow::GetDisplayId\n"); 
  return(this->DisplayId);
}
GC vtkXImageWindow::GetGC()
{
//  vtkDebugMacro(<< "vtkXImageWindow::GetGC\n"); 
  return(this->Gc);
}
//----------------------------------------------------------------------------
Colormap vtkXImageWindow::MakeColorMap(Visual *visual) 
{
  int idx;
  int value;
  unsigned long planeMask, pval[256];
  int screen;
  Colormap  defaultMap, newMap;
  XColor    defccells[256];
  
//  vtkDebugMacro(<< "vtkXImageWindow::MakeColorMap\n"); 
  
  this->Offset = 0;

  screen = DefaultScreen(this->DisplayId);
  defaultMap = DefaultColormap(this->DisplayId, screen);
  
  if ( !XAllocColorCells(this->DisplayId, defaultMap, 0, &planeMask, 0, 
			 pval, (unsigned int) this->NumberOfColors))
    {
    // can't allocate NUM_COLORS from Def ColorMap
    // create new ColorMap ... but first cp some def ColorMap
    
    newMap = XCreateColormap(this->DisplayId, 
			     RootWindow(this->DisplayId, screen),
			     visual, AllocNone);
    this->Offset = 100;
    if (! XAllocColorCells(this->DisplayId, newMap, 1, &planeMask, 0, pval,
			   (unsigned int)256))
      {
      vtkErrorMacro(<< "Sorry cann't allocate any more Colors");
      return (Colormap)(NULL);
      }
    
    for ( idx = 0 ; idx < 256; idx++) 
      {
      defccells[idx].pixel = idx; 
      }
    XQueryColors(this->DisplayId, defaultMap, defccells, 256);
    
    for (idx = 0 ; idx < 256; idx++)
      {
      // Value should range between ? and ?
      value = 1000 + (int)(60000.0 * (float)(idx - this->Offset) / (float)(this->NumberOfColors));
    
      if ( (idx < this->Offset)) 
	{
	this->Colors[idx].pixel = defccells[idx].pixel;
	this->Colors[idx].red   = defccells[idx].red ;
	this->Colors[idx].green = defccells[idx].green ;
	this->Colors[idx].blue  = defccells[idx].blue ;
	this->Colors[idx].flags = DoRed | DoGreen | DoBlue ;
	XStoreColor(this->DisplayId, newMap, &(this->Colors[idx]));
	}
      else 
	{
	this->Colors[idx].pixel = pval[idx];
	this->Colors[idx].red   = value ;
	this->Colors[idx].green = value ; 
	this->Colors[idx].blue  = value ;
	this->Colors[idx].flags = DoRed | DoGreen | DoBlue ;
	XStoreColor(this->DisplayId, newMap, &(this->Colors[idx]));
	}
      }
    XInstallColormap(this->DisplayId, newMap);
    return newMap;
    }
  else
    {
    for (idx = 0 ; idx < this->NumberOfColors ; idx++)
      {
      if (idx) 
	{
	value = (((192 * idx)/(this->NumberOfColors -1)) << 8)  + 16000;
	}
      else 
	{
	value = 0;
	}
      this->Colors[idx].pixel = pval[idx];
      this->Colors[idx].red   = value ;
      this->Colors[idx].green = value ;
      this->Colors[idx].blue  = value ;
      this->Colors[idx].flags = DoRed | DoGreen | DoBlue ;
      XStoreColor(this->DisplayId, defaultMap, &(this->Colors[idx]));
      }

    return defaultMap;
    } 
}



//----------------------------------------------------------------------------
void vtkXImageWindow::AllocateDirectColorMap() 
{
  int idx;
  int value;
  unsigned long planeMask, pval[256];
  Colormap newMap;
  
//  vtkDebugMacro(<< "vtkXImageWindow::AllocateDirectColorMap\n"); 
  
  this->Offset = 100;

  // Get the colors in the current color map.
  for ( idx = 0 ; idx < 256; idx++) 
    {
    this->Colors[idx].pixel = idx; 
    }
  XQueryColors(this->DisplayId, this->ColorMap, this->Colors, 256);
    
  
  newMap = XCreateColormap(this->DisplayId, this->WindowId,
			   this->VisualId, AllocNone);
  if (! XAllocColorCells(this->DisplayId, newMap, 1, &planeMask, 0, pval,
			 (unsigned int)256))
    {
    vtkErrorMacro(<< "Sorry cann't allocate any more Colors");
    return;
    }
  
  // Set up the colors
  for (idx = 0; idx < 100; ++idx)
    {
    this->Colors[idx].pixel = pval[idx];
    this->Colors[idx].flags = DoRed | DoGreen | DoBlue ;
    XStoreColor(this->DisplayId, newMap, &(this->Colors[idx]));
    }
  for (idx = 0 ; idx < this->NumberOfColors; ++idx)
    {
    // Value should range between 0 and 65000
    value = 1000 + (int)(60000.0 * (float)(idx)/(float)(this->NumberOfColors));
    this->Colors[idx+100].pixel = pval[idx];
    this->Colors[idx+100].red   = value ;
    this->Colors[idx+100].green = value ; 
    this->Colors[idx+100].blue  = value ;
    this->Colors[idx+100].flags = DoRed | DoGreen | DoBlue ;
    XStoreColor(this->DisplayId, newMap, &(this->Colors[idx+100]));
    }
  XInstallColormap(this->DisplayId, newMap);
  this->ColorMap = newMap;
  XSetWindowColormap(this->DisplayId, this->WindowId, this->ColorMap);
}
















