import vtk
import sys

fname = sys.argv[1]
odir = "/Users/demarle/tmp/xdmfout/"

x3r = vtk.vtkXdmf3Reader()
x3r.SetFileName(fname)
x3r.Update()
rdata = x3r.GetOutputDataObject(0)
print rdata
#for x in range(0,rdata.GetEdgeData().GetArray(0).GetNumberOfTuples()):
#  print x, rdata.GetEdgeData().GetArray(0).GetValue(x)

dsw = vtk.vtkGraphWriter()
dsw.SetFileName(odir+"x3w.vtk")
dsw.SetInputData(rdata)
dsw.Write()

x3w = vtk.vtkXdmf3Writer()
x3w.SetFileName(odir+"x3w.xdmf")
x3w.SetInputData(rdata)
x3w.Write()
