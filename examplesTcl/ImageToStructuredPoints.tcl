# Convert an image to a structured points data set.
# Display the data set as a texture map.

# Now create the RenderWindow, Renderer and both Actors
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# create a plane source and actor
vtkPlaneSource plane;
vtkPolyDataMapper  planeMapper;
planeMapper SetInput [plane GetOutput];
vtkActor planeActor;
planeActor SetMapper planeMapper;




vtkImageVolume16Reader reader;
#reader DebugOn
reader ReleaseDataFlagOff;
reader SwapBytesOn;
reader SetDataDimensions 256 256 94 1;
reader SetFilePrefix "../data/fullHead/headsq";
reader SetDataMask 0x7fff;
#reader SetOutputScalarTypeToUnsignedChar

vtkImageToStructuredPoints image;
image SetScalarInput [reader GetOutput];
image SetExtent 0 255 0 255 20 20;

# load in the texture map
#
vtkTexture atext;
atext SetInput [image GetOutput];
atext InterpolateOn;
planeActor SetTexture atext;

# Add the actors to the renderer, set the background and size
ren1 AddActor planeActor;
ren1 SetBackground 0.1 0.2 0.4;
renWin SetSize 500 500;

# render the image
iren Initialize;
set cam1 [ren1 GetActiveCamera];
$cam1 Elevation -30;
$cam1 Roll -20;
renWin Render;

#renWin SetFileName "TPlane.tcl.ppm";
#renWin SaveImageAsPPM;

# prevent the tk window from showing up then start the event loop
wm withdraw .





