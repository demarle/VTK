package require vtk
package require vtkinteraction

# create a rendering window and renderer
vtkRenderer ren1

vtkRenderWindow renWin
renWin AddRenderer ren1
renWin StereoCapableWindowOn  

vtkRenderWindowInteractor iren
iren SetRenderWindow renWin

vtkGenericEnSightReader reader
reader SetCaseFileName $VTK_DATA_ROOT/Data/EnSight/blow1_ascii.case
reader SetTimeValue 1
reader ReadAllVariablesOff
reader AddPointVariableName displacement
reader AddCellVariableName thickness
reader AddVariableName displacement 1
reader Update

vtkGeometryFilter geom
geom SetInput [reader GetOutput]

vtkPolyDataMapper mapper
mapper SetInput [geom GetOutput]
mapper SetScalarRange 0.5 1.0

vtkActor actor
actor SetMapper mapper

# assign our actor to the renderer
ren1 AddActor actor

# enable user interface interactor
iren AddObserver UserEvent {wm deiconify .vtkInteract}
iren Initialize

[ren1 GetActiveCamera] SetPosition 99.3932 17.6571 -22.6071
[ren1 GetActiveCamera] SetFocalPoint 3.5 12 1.5 
[ren1 GetActiveCamera] SetViewAngle 30
[ren1 GetActiveCamera] SetViewUp 0.239617 -0.01054 0.97081
ren1 ResetCameraClippingRange

renWin Render

# prevent the tk window from showing up then start the event loop
wm withdraw .

