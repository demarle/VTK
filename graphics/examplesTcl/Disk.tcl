catch {load vtktcl}
# this is a tcl version of the Mace example
# include get the vtk interactor ui
source ../../examplesTcl/vtkInt.tcl

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

vtkDiskSource disk
    disk SetInnerRadius 1.0
    disk SetOuterRadius 2.0
    disk SetRadialResolution 1
    disk SetCircumferentialResolution 20

vtkPolyDataMapper diskMapper
    diskMapper SetInput [disk GetOutput]
vtkActor diskActor
    diskActor SetMapper diskMapper

# Add the actors to the renderer, set the background and size
#
ren1 AddActor diskActor
ren1 SetBackground 0.1 0.2 0.4
renWin SetSize 450 450

# Get handles to some useful objects
#
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize
renWin Render

#renWin SetFileName Disk.tcl.ppm
#renWin SaveImageAsPPM

# prevent the tk window from showing up then start the event loop
wm withdraw .



