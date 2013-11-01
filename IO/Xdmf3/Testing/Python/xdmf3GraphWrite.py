import vtk
import sys

if len(sys.argv) != 3:
  print "usage:", sys.argv[0], " outdir infile.xdmf"
  exit(0)
odir = sys.argv[1]
fname = sys.argv[2]

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
