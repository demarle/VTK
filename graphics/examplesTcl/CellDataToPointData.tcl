# cell scalars to point scalars
# get the interactor ui
source ../../examplesTcl/vtkInt.tcl

# Create the RenderWindow, Renderer and RenderWindowInteractor
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# create a 2*2 cell/3*3 pt structuredgrid
vtkPoints points
  points InsertNextPoint    -1  1  0   
  points InsertNextPoint     0  1  0
  points InsertNextPoint     1  1  0
  points InsertNextPoint    -1  0  0
  points InsertNextPoint     0  0  0
  points InsertNextPoint     1  0  0
  points InsertNextPoint    -1 -1  0 
  points InsertNextPoint     0 -1  0
  points InsertNextPoint     1 -1  0

vtkFloatScalars faceColors
  faceColors InsertNextScalar 0
  faceColors InsertNextScalar 1
  faceColors InsertNextScalar 1
  faceColors InsertNextScalar 2

vtkStructuredGrid grid
  grid SetDimensions 3 3 1
  grid SetPoints points
  [grid GetCellData] SetScalars faceColors

vtkCellDataToPointData Cell2Point
  Cell2Point SetInput grid
  Cell2Point PassCellDataOn

vtkDataSetMapper mapper
  mapper SetInput [Cell2Point GetStructuredGridOutput]
  mapper SetScalarModeToUsePointData
  mapper SetScalarRange 0 2

vtkActor actor
    actor SetMapper mapper

# Add the actors to the renderer, set the background and size
ren1 AddActor actor
ren1 SetBackground 0.1 0.2 0.4
renWin SetSize 256 256

# render the image
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize
wm withdraw .

renWin SetFileName CellDataToPointData.tcl.ppm
#renWin SaveImageAsPPM
