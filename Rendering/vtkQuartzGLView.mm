#import "vtkQuartzGLView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

@implementation vtkQuartzGLView

// perform post-nib load setup here - not called unless using nib file
- (void)awakeFromNib
{
    // Initialization
    bitsPerPixel = depthSize = (enum NSOpenGLPixelFormatAttribute)32;
}

- (id)initWithFrame:(NSRect)theFrame
{

  NSOpenGLPixelFormatAttribute attribs[] = 
    {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, (enum NSOpenGLPixelFormatAttribute)32,
        (enum NSOpenGLPixelFormatAttribute)nil};
        
  NSOpenGLPixelFormat *fmt;

  /* Choose a pixel format */
  fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
  if(!fmt)
    NSLog(@"Pixel format is nil");
  
  /* Create a GLX context */
  self = [super initWithFrame:theFrame pixelFormat:fmt];
  if (!self)
    NSLog(@"initWithFrame failed");

  [[self openGLContext] makeCurrentContext];
  [[self window] setAcceptsMouseMovedEvents:YES];
  return self;
}

- (vtkQuartzRenderWindow *)getVTKRenderWindow {
    return myVTKRenderWindow;
}

- (void)setVTKRenderWindow:(vtkQuartzRenderWindow *)theVTKRenderWindow {
    myVTKRenderWindow = theVTKRenderWindow;
}

- (vtkQuartzRenderWindowInteractor *)getVTKRenderWindowInteractor {
    return myVTKRenderWindowInteractor;
}

- (void)setVTKRenderWindowInteractor:(vtkQuartzRenderWindowInteractor *)theVTKRenderWindowInteractor {
    myVTKRenderWindowInteractor = theVTKRenderWindowInteractor;
}

- (void)drawRect:(NSRect)theRect
{
  NSRect visibleRect;

  // Get visible bounds...
  visibleRect = [self bounds];
  
  // Set proper viewport
  //glViewport((long int)visibleRect.origin.x, (long int)visibleRect.origin.y, 
	//     (long int)visibleRect.size.width, (long int)visibleRect.size.height);
  myVTKRenderWindow->Render();
  [[self openGLContext] flushBuffer];
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (BOOL)becomeFirstResponder
{
  return YES;
}

- (BOOL)resignFirstResponder
{
  return YES;
}

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint mouseLoc;
    int shiftDown = ([theEvent modifierFlags] & NSShiftKeyMask);
    int controlDown = ([theEvent modifierFlags] & NSControlKeyMask);

    mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
}


- (void)mouseDown:(NSEvent *)theEvent {
    BOOL keepOn = YES;
    NSPoint mouseLoc;
    int shiftDown = ([theEvent modifierFlags] & NSShiftKeyMask);
    int controlDown = ([theEvent modifierFlags] & NSControlKeyMask);

    mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
    
    do {
      switch ([theEvent type]) {
        case NSLeftMouseDragged:
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
            break;
        case NSLeftMouseUp:
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, NULL);
            keepOn = NO;
            [NSEvent stopPeriodicEvents];
            return;
        case NSPeriodic:
            [NSEvent stopPeriodicEvents];
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::TimerEvent, NULL);
            break;
        default:
            break;
        }
        theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask | NSLeftMouseDraggedMask | NSPeriodicMask];
        mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
        myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    }while (keepOn);
    return;
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    BOOL keepOn = YES;
    NSPoint mouseLoc;
    int shiftDown = ([theEvent modifierFlags] & NSShiftKeyMask);
    int controlDown = ([theEvent modifierFlags] & NSControlKeyMask);

    mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);

    do {
      switch ([theEvent type]) {
        case NSRightMouseDragged:
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
            break;
        case NSRightMouseUp:
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::RightButtonReleaseEvent, NULL);
            keepOn = NO;
            [NSEvent stopPeriodicEvents];
            return;
        case NSPeriodic:
            [NSEvent stopPeriodicEvents];
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::TimerEvent, NULL);
            break;
        default:
            break;
        }
        theEvent = [[self window] nextEventMatchingMask: NSRightMouseUpMask | NSRightMouseDraggedMask | NSPeriodicMask];
        mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
        myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    }while (keepOn);
    return;
}


- (void)otherMouseDown:(NSEvent *)theEvent {
    BOOL keepOn = YES;
    NSPoint mouseLoc;
    int shiftDown = ([theEvent modifierFlags] & NSShiftKeyMask);
    int controlDown = ([theEvent modifierFlags] & NSControlKeyMask);

    mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
    
    do {
      switch ([theEvent type]) {
        case NSOtherMouseDragged:
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
            break;
        case NSOtherMouseUp:
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, NULL);
            keepOn = NO;
            [NSEvent stopPeriodicEvents];
            return;
        case NSPeriodic:
            [NSEvent stopPeriodicEvents];
            myVTKRenderWindowInteractor->InvokeEvent(vtkCommand::TimerEvent, NULL);
            break;
        default:
            break;
        }
        theEvent = [[self window] nextEventMatchingMask: NSOtherMouseUpMask | NSOtherMouseDraggedMask | NSPeriodicMask];
        mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
        myVTKRenderWindowInteractor->SetEventInformation(mouseLoc.x, mouseLoc.y, controlDown, shiftDown);
    }while (keepOn);
    return;
}




@end
