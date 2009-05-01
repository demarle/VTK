from vtk import *

source = vtkRandomGraphSource()
source.SetNumberOfVertices(150)
source.SetEdgeProbability(0.01)
source.SetUseEdgeProbability(True)
source.SetStartWithTree(True)

view = vtkGraphLayoutView()
view.AddRepresentationFromInputConnection(source.GetOutputPort())
view.SetVertexLabelArrayName("vertex id")
view.SetVertexLabelVisibility(True)
view.SetVertexColorArrayName("vertex id")
view.SetColorVertices(True)
view.SetLayoutStrategyToSpanTree()
view.SetInteractionModeTo3D() # Left mouse button causes 3D rotate instead of zoom
view.SetLabelPlacementModeToLabelPlacer()

theme = vtkViewTheme.CreateMellowTheme()
theme.SetCellColor(.2,.2,.6)
theme.SetLineWidth(2)
theme.SetPointSize(10)
view.ApplyViewTheme(theme)
theme.FastDelete()

window = vtkRenderWindow()
window.SetSize(600, 600)
view.SetupRenderWindow(window)
view.GetRenderer().ResetCamera()
window.Render()
window.GetInteractor().Start()
