import vtk

readOKfiles = [
"2DCoRectMesh.xmf",
"2DRectMesh.xmf",
"2DSMesh.xmf",
"3DCoRectMesh.xmf",
"3DCoRectMeshTimeStep.xmf",
"3DRectMesh.xmf",
"3DSMesh.xmf",
"Hexahedron.xmf",
"HexahedronTimestep.wtime.xmf",
"HexahedronTimestep.xmf",
"Iron_Protein.ImageData.xmf",
"Iron_Protein.ImageData.Collection.xmf",
"Iron_Protein.RectilinearGrid.xmf",
"Iron_Protein.RectilinearGrid.Collection.xmf",
"Iron_Protein.StructuredGrid.xmf",
"Iron_Protein.StructuredGrid.Collection.xmf",
"Mixed.xmf",
"Scenario1_p1.xmf",
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
]


correctfiles = [
]

badfiles = [
"sirnotappearinginthisfilm.xmf",
"hexahedron_x_y.xmf",
#xdmf doesn't have X_Y anymore
"hexahedron_x_y_z.xmf",
#xdmf doesn't have X_Y_Z anymore
"Test1.xmf",
#TODO: handle 1D in HDF5 - new code not as tolerant of array dim mismatches
#TODO:stop trying to mark active scalars as old ones get deleted
#TODO:it doesn't yet read the sets in as multiblockk
"RodPlate/RodPlate.xmf",
#TODO: write i*j instead of i j, new code not as tolerant of array dim mismatches
#gets geom a bit off with i*j,
#doesn't like "Function" -> 'Content' not found in itemProperties in XdmfArray::populateItem
#ignores BaseOffset in block 2 and as a result gets topology messed up
"poisson_3dall_2.xmf",
#TODO: doesn't like "set"
"PolygonOctagon.xmf",
#won't open in paraview
"Polyline.xmf",
#ditto
"tensor.xmf",
#xdmf2 makes up cells for the points 3 doesn't
"testFile.xmf",
#looks very different in 2 and 3
]

testfilenames = [
]

testfilenames = readOKfiles

for fname in testfilenames:
  xr = vtk.vtkXdmf3Reader()
  afname = "/Data/ARL/XdmfExamples/" + fname
  print "Trying: ", afname
  xr.CanReadFile(afname)
  xr.SetFileName(afname)
  xr.UpdateInformation()
  xr.Update()

#ptf = vtk.vtkPassThroughFilter()
#ptf.SetInputConnection(xr.GetOutputPort())
#ptf.Update()
