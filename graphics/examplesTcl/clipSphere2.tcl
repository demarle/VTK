#
# Demonstrate the use of clipping on polygonal data
#
source vtkInt.tcl
source "colors.tcl"

# create pipeline
#
vtkSphereSource sphere;
    sphere SetRadius 1;
    sphere SetPhiResolution 50;
    sphere SetThetaResolution 50;
#vtkPlaneSource sphere;
#    sphere SetXResolution 10;
#    sphere SetYResolution 25;
#vtkConeSource sphere;
#    sphere SetResolution 10;

vtkPlane plane;
    plane SetOrigin 0.25 0 0;
    plane SetNormal -1 -1 0;
vtkImplicitWindowFunction iwf;
    iwf SetImplicitFunction plane;
    iwf SetWindowRange -.2 .2
    iwf SetWindowValues 0 1;
#vtkClipPolyData clipper;
vtkClipper clipper;
    clipper SetInput [sphere GetOutput];
    clipper SetClipFunction iwf;
    clipper SetValue 0.0;
    clipper DebugOn;
vtkDataSetMapper clipMapper;
    clipMapper SetInput [clipper GetOutput];
    clipMapper ScalarsVisibleOff;
vtkActor clipActor;
    clipActor SetMapper clipMapper;
    eval [clipActor GetProperty] SetColor $peacock;

# Create graphics stuff
#
vtkRenderMaster rm;
set renWin [rm MakeRenderWindow];
set ren1   [$renWin MakeRenderer];
set iren [$renWin MakeRenderWindowInteractor];

# Add the actors to the renderer, set the background and size
#
$ren1 AddActors clipActor;
$ren1 SetBackground 1 1 1;
$renWin SetSize 500 500;
$iren Initialize;

# render the image
#
$iren SetUserMethod {wm deiconify .vtkInteract};

# prevent the tk window from showing up then start the event loop
wm withdraw .


