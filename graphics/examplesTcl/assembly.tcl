catch {load vtktcl}
if { [catch {set VTK_TCL $env(VTK_TCL)}] != 0} { set VTK_TCL "../../examplesTcl" }
if { [catch {set VTK_DATA $env(VTK_DATA)}] != 0} { set VTK_DATA "../../../vtkdata" }

# this demonstrates use of assemblies
# include get the vtk interactor ui
source $VTK_TCL/vtkInt.tcl

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# create four parts: a top level assembly and three primitives
#
vtkSphereSource sphere
vtkPolyDataMapper sphereMapper
    sphereMapper SetInput [sphere GetOutput]
vtkActor sphereActor
    sphereActor SetMapper sphereMapper
    sphereActor SetOrigin 2 1 3
    sphereActor RotateY 6
    sphereActor SetPosition 2.25 0 0
    [sphereActor GetProperty] SetColor 1 0 1

vtkCubeSource cube
vtkPolyDataMapper cubeMapper
    cubeMapper SetInput [cube GetOutput]
vtkActor cubeActor
    cubeActor SetMapper cubeMapper
    cubeActor SetPosition 0.0 .25 0
    [cubeActor GetProperty] SetColor 0 0 1

vtkConeSource cone
vtkPolyDataMapper coneMapper
    coneMapper SetInput [cone GetOutput]
vtkActor coneActor
    coneActor SetMapper coneMapper
    coneActor SetPosition 0 0 .25
    [coneActor GetProperty] SetColor 0 1 0

vtkCylinderSource cylinder;#top part
vtkPolyDataMapper cylinderMapper
    cylinderMapper SetInput [cylinder GetOutput]
vtkActor cylinderActor
    cylinderActor SetMapper cylinderMapper
    [cylinderActor GetProperty] SetColor 1 0 0
vtkAssembly assembly
    assembly AddPart cylinderActor
    assembly AddPart sphereActor
    assembly AddPart cubeActor
    assembly AddPart coneActor
    assembly SetOrigin 5 10 15
    assembly AddPosition 5 0 0
    assembly RotateX 15

# Add the actors to the renderer, set the background and size
#
ren1 AddActor assembly
ren1 AddActor coneActor
ren1 SetBackground 0.1 0.2 0.4
renWin SetSize 450 450

# Get handles to some useful objects
#
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize
renWin Render

#renWin SetFileName assembly.tcl.ppm
#renWin SaveImageAsPPM

# prevent the tk window from showing up then start the event loop
wm withdraw .


