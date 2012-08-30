#!/usr/bin/env python

reader = vtk.vtkSimplePointsReader()
reader.SetFileName("" + str(VTK_DATA_ROOT) + "/Data/points.txt")
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(reader.GetOutputPort())
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetPointSize(5)
ren1 = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(ren1)
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)
ren1.AddActor(actor)
renWin.SetSize(300,300)
iren.Initialize()
renWin.Render()
# --- end of script --
