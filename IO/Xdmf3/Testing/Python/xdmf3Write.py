import vtk
import sys

if len(sys.argv) != 3:
  print "usage:", sys.argv[0], " outdir infile.xdmf"
  exit(0)
odir = sys.argv[1]
fname = sys.argv[2]

dsw = vtk.vtkDataSetWriter()

if "vtkXdmfWriter" in dir(vtk):
  print "XDMF2 READ " + fname
  x2r = vtk.vtkXdmfReader()
  x2r.SetFileName(fname)
  x2r.Update()
  rdata = x2r.GetOutputDataObject(0)
  print rdata
  if rdata.GetPointData().GetArray(0):
    for x in range(0,rdata.GetPointData().GetArray(0).GetNumberOfTuples()):
      print rdata.GetPointData().GetArray(0).GetValue(x)

  ofname = odir+"x2w.vtk"
  print "VTK WRITE "+ ofname
  dsw.SetInputConnection(x2r.GetOutputPort())
  dsw.SetFileName(ofname)
  dsw.Write()

  ofname = odir+"x2w.xdmf"
  print "XDMF2 WRITE "+ ofname
  x2w = vtk.vtkXdmfWriter()
  x2w.SetInputConnection(x2r.GetOutputPort())
  x2w.SetFileName(odir+"x2w.xdmf")
  x2w.Write()

print "XDMF3 READ " + fname
x3r = vtk.vtkXdmf3Reader()
x3r.SetFileName(fname)
x3r.Update()
rdata = x3r.GetOutputDataObject(0)
print rdata
if rdata.GetPointData().GetArray(0):
  for x in range(0,rdata.GetPointData().GetArray(0).GetNumberOfTuples()):
    print rdata.GetPointData().GetArray(0).GetValue(x)

ofname = odir+"x3w.vtk"
print "VTK WRITE "+ ofname
dsw.SetInputConnection(x3r.GetOutputPort())
dsw.SetFileName(odir+"x3w.vtk")
dsw.Write()

ofname = odir+"x3w.xdmf"
print "XDMF3 WRITE "+ ofname
x3w = vtk.vtkXdmf3Writer()
x3w.SetInputData(x3r.GetOutputDataObject(0))
x3w.SetFileName(odir+"x3w.xdmf")
x3w.Write()
