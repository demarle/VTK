catch {load vtktcl}
#
# Demonstrate the use of clipping and capping on polyhedral data
#
source ../../examplesTcl/vtkInt.tcl
source ../../examplesTcl/colors.tcl

# create a sphere and clip it
#
vtkSphereSource sphere
    sphere SetRadius 1
    sphere SetPhiResolution 10
    sphere SetThetaResolution 10
vtkPlane plane
    plane SetOrigin 0 0 0
    plane SetNormal -1 -1 0
vtkClipPolyData clipper
    clipper SetInput [sphere GetOutput]
    clipper SetClipFunction plane
    clipper GenerateClipScalarsOn
    clipper GenerateClippedOutputOn
    clipper SetValue 0
vtkPolyDataMapper clipMapper
    clipMapper SetInput [clipper GetOutput]
    clipMapper ScalarVisibilityOff

vtkProperty backProp
    eval backProp SetDiffuseColor $tomato
vtkActor clipActor
    clipActor SetMapper clipMapper
    eval [clipActor GetProperty] SetColor $peacock
    clipActor SetBackfaceProperty backProp

# now extract feature edges
vtkFeatureEdges boundaryEdges
  boundaryEdges SetInput [clipper GetOutput]
  boundaryEdges BoundaryEdgesOn
  boundaryEdges FeatureEdgesOff
  boundaryEdges NonManifoldEdgesOff

vtkCleanPolyData boundaryClean
  boundaryClean SetInput [boundaryEdges GetOutput]

vtkStripper boundaryStrips
  boundaryStrips SetInput [boundaryClean GetOutput]
  boundaryStrips Update

vtkPolyData boundaryPoly
  boundaryPoly SetPoints [[boundaryStrips GetOutput] GetPoints]
  boundaryPoly SetPolys [[boundaryStrips GetOutput] GetLines]

vtkTriangleFilter boundaryTriangles
  boundaryTriangles SetInput boundaryPoly


vtkPolyDataMapper boundaryMapper
  boundaryMapper SetInput boundaryPoly

vtkActor boundaryActor
  boundaryActor SetMapper boundaryMapper
  eval [boundaryActor GetProperty] SetColor $banana

# Create graphics stuff
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# Add the actors to the renderer, set the background and size
#
ren1 AddActor clipActor
ren1 AddActor boundaryActor
ren1 SetBackground 1 1 1
[ren1 GetActiveCamera] Azimuth 30
[ren1 GetActiveCamera] Elevation 30
[ren1 GetActiveCamera] Dolly 1.2
ren1 ResetCameraClippingRange

renWin SetSize 400 400
iren Initialize

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}
#renWin SetFileName "capSphere.tcl.ppm"
#renWin SaveImageAsPPM

# prevent the tk window from showing up then start the event loop
wm withdraw .
