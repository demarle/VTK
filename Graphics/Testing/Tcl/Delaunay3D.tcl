package require vtktcl_interactor

# create some random points in the unit cube centered at (.5,.5,.5)
#
vtkMath math
vtkPoints points
for {set i 0} {$i<25} {incr i 1} {
    eval points InsertPoint $i [math Random 0 1] [math Random 0 1] [math Random 0 1]
}

vtkPolyData profile
    profile SetPoints points

# triangulate them
#
vtkDelaunay3D del
    del SetInput profile
    del BoundingTriangulationOn
    del SetTolerance 0.01
    del SetAlpha 0.2
    del BoundingTriangulationOff
    
vtkShrinkFilter shrink
    shrink SetInput [del GetOutput]
    shrink SetShrinkFactor 0.9

vtkDataSetMapper map
    map SetInput [shrink GetOutput]

vtkActor triangulation
    triangulation SetMapper map
    [triangulation GetProperty] SetColor 1 0 0

# Create graphics stuff
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# Add the actors to the renderer, set the background and size
#
ren1 AddActor triangulation
ren1 SetBackground 1 1 1
renWin SetSize 250 250
renWin Render

set cam1 [ren1 GetActiveCamera]
$cam1 Zoom 1.5

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}
renWin Render

# prevent the tk window from showing up then start the event loop
wm withdraw .



