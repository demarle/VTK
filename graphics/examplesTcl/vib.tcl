catch {load vtktcl}
# this is a tcl version of plate vibration
# get the interactor ui
source ../../examplesTcl/vtkInt.tcl

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# read a vtk file
#
vtkPolyDataReader plate
    plate SetFileName "../../../vtkdata/plate.vtk"
    plate SetVectorsName "mode2"
vtkPolyDataNormals normals
    normals SetInput [plate GetOutput]
vtkWarpVector warp
    warp SetInput [normals GetOutput]
    warp SetScaleFactor 0.5
vtkVectorDot color
    color SetInput [warp GetOutput]
vtkDataSetMapper plateMapper
    plateMapper SetInput [warp GetOutput]
#    plateMapper SetInput [color GetOutput]
vtkActor plateActor
    plateActor SetMapper plateMapper

# create the outline
#
vtkOutlineFilter outline
    outline SetInput [plate GetOutput]
vtkPolyDataMapper spikeMapper
    spikeMapper SetInput [outline GetOutput]
vtkActor outlineActor
    outlineActor SetMapper spikeMapper
eval [outlineActor GetProperty] SetColor 0.0 0.0 0.0

# Add the actors to the renderer, set the background and size
#
ren1 AddActor plateActor
ren1 AddActor outlineActor
ren1 SetBackground 0.2 0.3 0.4
renWin SetSize 500 500

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize
#renWin SetFileName "vib.tcl.ppm"
#renWin SaveImageAsPPM

# prevent the tk window from showing up then start the event loop
wm withdraw .


