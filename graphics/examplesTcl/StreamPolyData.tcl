catch {load vtktcl}
if { [catch {set VTK_TCL $env(VTK_TCL)}] != 0} { set VTK_TCL "../../examplesTcl" }
if { [catch {set VTK_DATA $env(VTK_DATA)}] != 0} { set VTK_DATA "../../../vtkdata" }


set NUMBER_OF_PIECES 5

# Generate implicit model of a sphere
#
source $VTK_TCL/vtkInt.tcl
source $VTK_TCL/colors.tcl

# Create renderer stuff
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# create pipeline that handles ghost cells

vtkSphereSource sphere
    sphere SetRadius 3
    sphere SetPhiResolution 100
    sphere SetThetaResolution 150
    # sphere SetStartMethod {tk_messageBox -message "Executing with piece [[sphere GetOutput] GetUpdatePiece]"}

# Just playing with an alternative that is not currently used.
proc NotUsed {} {
  # This filter actually spoils the example because it asks for the whole input.
  # The only reason it is here is because sphere complains it cannot generate ghost cells.
  vtkExtractPolyDataPiece piece
    piece SetInput [sphere GetOutput]
    # purposely put seams in here.
    piece CreateGhostCellsOff

  # purposely put seams in here.
  vtkPolyDataNormals pdn
    pdn SetInput [piece GetOutput]
}

# Just playing with an alternative that is not currently used.
vtkDecimatePro deci
  deci SetInput [sphere GetOutput]
  # this did not remove seams as I thought it would
  deci BoundaryVertexDeletionOff
  #deci PreserveTopologyOn

# Since quadric Clustering does not handle borders properly yet,
# the pieces will have dramatic "eams"
vtkQuadricClustering q
  q SetInput [sphere GetOutput]
  q SetNumberOfXDivisions 5
  q SetNumberOfYDivisions 5
  q SetNumberOfZDivisions 10
  q UseInputPointsOn

vtkPolyDataStreamer streamer
  #streamer SetInput [deci GetOutput]
  streamer SetInput [q GetOutput]
  #streamer SetInput [pdn GetOutput]
  streamer SetNumberOfStreamDivisions $NUMBER_OF_PIECES



vtkPolyDataMapper mapper
  mapper SetInput [streamer GetOutput]
  mapper ScalarVisibilityOff
  mapper SetPiece 0
  mapper SetNumberOfPieces 2
  mapper ImmediateModeRenderingOn

vtkActor actor
    actor SetMapper mapper
    eval [actor GetProperty] SetColor $english_red



# Add the actors to the renderer, set the background and size
#

[ren1 GetActiveCamera] SetPosition 5 5 10
[ren1 GetActiveCamera] SetFocalPoint 0 0 0
ren1 AddActor actor
ren1 SetBackground 1 1 1
renWin SetSize 500 500
iren Initialize

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}

# prevent the tk window from showing up then start the event loop
wm withdraw .


