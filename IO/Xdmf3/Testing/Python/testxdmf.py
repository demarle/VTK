import vtk

readOKfiles = [
"2DCoRectMesh.xmf", #had to add NumberType and AttributeType or xdmf failed
"2DSMesh.xmf", #ditto
"3DCoRectMesh.xmf",
"3DCoRectMeshTimeStep.xmf", #vtk reader gets confused with T[B B B]
"3DSMesh.xmf",
"Hexahedron.xmf",
"HexahedronTimestep.wtime.xmf",
"HexahedronTimestep.xmf",
"Iron_Protein.ImageData.xmf" #had to change DataStructure to DataItem (deprecated), and get rid of whitespace and template around hdf5 filename
"Iron_Protein.ImageData.Collection.xmf" #ditto
"Iron_Protein.StructuredGrid.xmf" #ditto
"Iron_Protein.StructuredGrid.Collection.xmf", #ditto
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
"quadrilateral.xmf",
"rectTest.xmf",
]


correctfiles = [
]

badfiles = [
"sirnotappearinginthisfilm.xmf"
"hexahedron_x_y.xmf", #xdmf doesn't have X_Y anymore
"hexahedron_x_y_z.xmf" #xdmf doesn't have X_Y_Z anymore
"Test1.xmf" #have to strip h5 filename
#add AttributeType+NumberType
#convert DataStructure to DataItem
#TODO: handle 1D in HDF5 - new code not as tolerant of array dim mismatches
#TODO:stop trying to mark active scalars as old ones get deleted
#TODO:it doesn't yet read the sets in as multiblockk
"RodPlate/RodPlate.xmf", #strip filename,
#TODO: write i*j instead of i j, new code not as tolerant of array dim mismatches
#gets geom a bit off with i*j,
#doesn't like "Function" -> 'Content' not found in itemProperties in XdmfArray::populateItem
#ignores BaseOffset in block 2 and as a result gets topology messed up
"poisson_3dall_2.xmf", #strip h5 name
#TODO doesn't like "set"
"2DRectMesh.xmf", #loads but attributes are out of order and crashed parview when you colormap
"3DRectMesh.xmf", #ditto
"Iron_Protein.RectilinearGrid.xmf", #ditto
"Iron_Protein.RectilinearGrid.Collection.xmf", #ditto
"PolygonOctagon.xmf", #won't open in paraview
"Polyline.xmf", #ditto
"tensor.xmf", #xdmf2 makes up cells for the points 3 doesn't
"testFile.xmf", #looks very different in 2 and 3
]

testfilenames = [
]

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
