import vtk

def printArray(array):
  print array.GetName()
  print array.GetRange()
  print array.GetNumberOfTuples(), array.GetNumberOfComponents()
  for x in range(0,array.GetNumberOfTuples()):
    if (x<10):
      print x, array.GetValue(x)

def printGraph(rdata):
  print rdata
  print "VERT ARRAYS"
  for x in range(0,rdata.GetVertexData().GetNumberOfArrays()):
     printArray(rdata.GetVertexData().GetArray(x))
  print "EDGE ARRAYS"
  for x in range(0,rdata.GetEdgeData().GetNumberOfArrays()):
     printArray(rdata.GetEdgeData().GetArray(x))

import sys
if len(sys.argv) != 2:
  print "usage:", sys.argv[0], " infile.xdmf"
  exit(0)
fname = sys.argv[1]

print "READ XDMF3 GRAPH"
x3r = vtk.vtkXdmf3Reader()
x3r.SetFileName(fname)
x3r.Update()
rdata = x3r.GetOutputDataObject(0)
printGraph(rdata)

print "THRESHOLD"
tgf1 = vtk.vtkThresholdGraph()
tgf1.SetInputData(rdata)
tgf1.SetInputArrayToProcess(0,0,0,4, "GPS") #4=FIELD_ASSOCIATION_VERTICES
#(-74499998.0, 41299997.0)
#(-74499998.0, -73500016.0)
tgf1.SetLowerThreshold(-74499998)
tgf1.SetUpperThreshold(-74450000)
#tgf1.SetInputArrayToProcess(0,0,0,5, "Packet Size") #4=FIELD_ASSOCIATION_VERTICES
#(2.0, 92366.0)
#tgf1.SetLowerThreshold(10000)
#tgf1.SetUpperThreshold(10001)
tgf1.Update()
odata = tgf1.GetOutputDataObject(0)
printGraph(odata)

print "THRESHOLD"
tgf2 = vtk.vtkThresholdGraph()
tgf2.SetInputData(odata)
#tgf2.SetInputArrayToProcess(0,0,0,4, "GPS") #4=FIELD_ASSOCIATION_VERTICES
#(-74499998.0, 41299997.0)
#tgf2.SetLowerThreshold(-74499998)
#tgf2.SetUpperThreshold(-74400000)
tgf2.SetInputArrayToProcess(0,0,0,5, "Packet Size") #4=FIELD_ASSOCIATION_VERTICES
#(2.0, 92366.0)
tgf2.SetLowerThreshold(0)
tgf2.SetUpperThreshold(15000)
tgf2.Update()
#odata = tgf2.GetOutputDataObject(0)
#printGraph(odata)

print "MAKING VIEW"
vdata = odata
view = vtk.vtkGraphLayoutView()
view.AddRepresentationFromInput(vdata)
#view.SetVertexLabelArrayName("ID")
#view.SetVertexLabelVisibility(True)
#view.SetLayoutStrategyToRandom()
#view.SetLayoutStrategyToForceDirected()
#view.SetLayoutStrategyToClustering2D()
#view.SetLayoutStrategyToFast2D()
print view.GetLayoutStrategyName()
view.SetVertexLabelFontSize(20)

theme = vtk.vtkViewTheme.CreateMellowTheme()
theme.SetLineWidth(10)
theme.SetPointSize(3)
theme.SetCellOpacity(1)
view.ApplyViewTheme(theme)
theme.FastDelete()

view.GetRenderWindow().SetSize(600, 600)
view.ResetCamera()
view.Render()

view.GetInteractor().Start()
