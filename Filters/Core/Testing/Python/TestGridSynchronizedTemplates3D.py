#!/usr/bin/env python
import vtk
from vtk.test import Testing
from vtk.util.misc import vtkGetDataRoot
VTK_DATA_ROOT = vtkGetDataRoot()

# cut data
pl3d = vtk.vtkMultiBlockPLOT3DReader()
pl3d.SetXYZFileName("" + str(VTK_DATA_ROOT) + "/Data/combxyz.bin")
pl3d.SetQFileName("" + str(VTK_DATA_ROOT) + "/Data/combq.bin")
pl3d.SetScalarFunctionNumber(100)
pl3d.SetVectorFunctionNumber(202)
pl3d.Update()
pl3d_output = pl3d.GetOutput().GetBlock(0)
range = pl3d_output.GetPointData().GetScalars().GetRange()
min = lindex(range,0)
max = lindex(range,1)
value = expr.expr(globals(), locals(),["(","min","+","max",")","/","2.0"])
#vtkGridSynchronizedTemplates3D cf
cf = vtk.vtkContourFilter()
cf.SetInputData(pl3d_output)
cf.SetValue(0,value)
#cf ComputeNormalsOff
cfMapper = vtk.vtkPolyDataMapper()
cfMapper.ImmediateModeRenderingOn()
cfMapper.SetInputConnection(cf.GetOutputPort())
cfMapper.SetScalarRange(pl3d_output.GetPointData().GetScalars().GetRange())
cfActor = vtk.vtkActor()
cfActor.SetMapper(cfMapper)
#outline
outline = vtk.vtkStructuredGridOutlineFilter()
outline.SetInputData(pl3d_output)
outlineMapper = vtk.vtkPolyDataMapper()
outlineMapper.SetInputConnection(outline.GetOutputPort())
outlineActor = vtk.vtkActor()
outlineActor.SetMapper(outlineMapper)
outlineActor.GetProperty().SetColor(0,0,0)
## Graphics stuff
# Create the RenderWindow, Renderer and both Actors
#
ren1 = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.SetMultiSamples(0)
renWin.AddRenderer(ren1)
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)
# Add the actors to the renderer, set the background and size
#
ren1.AddActor(outlineActor)
ren1.AddActor(cfActor)
ren1.SetBackground(1,1,1)
renWin.SetSize(400,400)
cam1 = ren1.GetActiveCamera()
cam1.SetClippingRange(3.95297,50)
cam1.SetFocalPoint(9.71821,0.458166,29.3999)
cam1.SetPosition(2.7439,-37.3196,38.7167)
cam1.SetViewUp(-0.16123,0.264271,0.950876)
iren.Initialize()
# render the image
#
# loop over surfaces
i = 0
while i < 17:
    cf.SetValue(0,expr.expr(globals(), locals(),["min","+","(","i","/","16.0",")*(","max","-","min",")"]))
    renWin.Render()
    i = i + 1

cf.SetValue(0,expr.expr(globals(), locals(),["min","+","(","0.2",")*(","max","-","min",")"]))
renWin.Render()
# prevent the tk window from showing up then start the event loop
# --- end of script --
