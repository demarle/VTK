import vtk
xr = vtk.vtkXdmf3Reader()

testfilenames = [
#"/sirnotappearinginthisfilm.xmf"
#"/Data/ParaViewData/Data/RectGrid2.vtk"
#"/Data/ParaViewData/Data/Iron Xdmf/Iron_Protein.ImageData.xmf"
"/Data/ARL/XdmfExamples/testFile.xmf"
#"/Data/ARL/XdmfExamples/3DCoRectMesh.xmf"
#"/Data/ARL/XdmfExamples/3DRectMesh.xmf"
#"/Data/ARL/XdmfExamples/3DSMesh.xmf"
]

for fname in testfilenames:
  print "Trying: ", fname
  xr.CanReadFile(fname)

fname = testfilenames[-1]
print "Reading: ", fname
xr.SetFileName(fname)

xr.UpdateInformation()
xr.Update()

#ptf = vtk.vtkPassThroughFilter()
#ptf.SetInputConnection(xr.GetOutputPort())
#ptf.Update()
