"""
This test verifies that vtk's Xdmf reader will read a sampling of small
to moderate size data files that cover a spectrum of file format features.
"""

import sys
import vtk
from vtk.util.misc import vtkGetDataRoot
VTK_DATA_ROOT = vtkGetDataRoot()

goodFiles = [
#from XDMF
"2DCoRectMesh.xmf",
"2DRectMesh.xmf",
"2DSMesh.xmf",
"3DCoRectMesh.xmf",
"3DRectMesh.xmf",
"3DSMesh.xmf",
"Graph.xmf",
"Hexahedron.xmf",
"HexahedronTimestep.xmf",
"Mixed.xmf",
"Tetrahedron.xmf",
"TetrahedronMultipleGrids.xmf",
"TetrahedronSpatial.xmf",
"TetrahedronSpatialTimestep.xmf",
"TetrahedronTimestep.xmf",
"Triangle.xmf",
"corect.xmf",
"hex20.xmf",
"hexahedron_xy.xmf",
"hexahedron_xyz.xmf",
"output.xmf",
"quadrilateral.xmf",
"rectTest.xmf",
"tensor.xmf",
#from ParaView
"Iron/Iron_Protein.ImageData.xmf",
"Iron/Iron_Protein.ImageData.Collection.xmf",
"Iron/Iron_Protein.RectilinearGrid.xmf",
"Iron/Iron_Protein.RectilinearGrid.Collection.xmf",
"Iron/Iron_Protein.StructuredGrid.xmf",
"Iron/Iron_Protein.StructuredGrid.Collection.xmf",
"Scenario1_p1.xmf",
]

badFiles = [
"sirnotappearinginthisfilm.xmf",
#have to implement a CanReadFile

  #"hexahedron_x_y.xmf",
#xdmf doesn't have X_Y anymore

  #"hexahedron_x_y_z.xmf",
#xdmf doesn't have X_Y_Z anymore

"Test1.xmf",
#TODO: handle 1D in HDF5 - new code not as tolerant of array dim mismatches
#TODO:stop trying to mark active scalars as old ones get deleted
#TODO:it doesn't yet read the sets in as multiblockk

#"RodPlate/RodPlate.xmf",
#TODO: write i*j instead of i j, new code not as tolerant of array dim mismatches
#gets geom a bit off with i*j,
#doesn't like "Function" -> 'Content' not found in itemProperties in XdmfArray::populateItem
#ignores BaseOffset in block 2 and as a result gets topology messed up

"poisson_3dall_2.xmf",
#TODO: doesn't like "set"

"PolygonOctagon.xmf",
#won't open in paraview

"Polyline.xmf",

#xdmf2 makes up cells for the points 3 doesn't
"testFile.xmf",
#looks very different in 2 and 3
]

testfilenames = goodFiles

if __name__ == "__main__":
  for fname in testfilenames:
    xr = vtk.vtkXdmf3Reader()
    afname = "" + str(VTK_DATA_ROOT) + "/Data/XDMF/" + fname
    print "Trying ", afname
    xr.CanReadFile(afname)
    xr.SetFileName(afname)
    xr.UpdateInformation()
    xr.Update()
    ds = xr.GetOutputDataObject(0)
    if not ds:
      print "Got zero output from known good file"
      sys.exit(vtk.VTK_ERROR)
    #if res != vtk.VTK_OK:
    #  print "Could not read", afname
    #  sys.exit(vtk.VTK_ERROR)
