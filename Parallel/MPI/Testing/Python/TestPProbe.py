#!/usr/bin/env python

if (info.command(globals(), locals(),  vtkMesaRenderer) != ""):
    _graphics_fact = vtk.vtkGraphicsFactory()
    _graphics_fact.SetUseMesaClasses(1)
    del _graphics_fact
    pass
# create a rendering window and renderer
Ren1 = vtk.vtkRenderer()
Ren1.SetBackground(.5,.8,1)
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(Ren1)
renWin.SetSize(300,300)
puts.myProcId()
if (myProcId > 0):
    renWin.SetPosition(expr.expr(globals(), locals(),["myProcId","*","350"]),0)
    renWin.OffScreenRenderingOn()
    pass
# camera parameters
camera = Ren1.GetActiveCamera()
camera.SetPosition(199.431,196.879,15.7781)
camera.SetFocalPoint(33.5,33.5,33.5)
camera.SetViewUp(0.703325,-0.702557,0.108384)
camera.SetViewAngle(30)
camera.SetClippingRange(132.14,361.741)
ironProt0 = vtk.vtkPDataSetReader()
ironProt0.SetFileName("" + str(VTK_DATA_ROOT) + "/Data/ironProt.vtk")
Geometry4 = vtk.vtkPOutlineFilter()
Geometry4.SetInputConnection(ironProt0.GetOutputPort())
Mapper4 = vtk.vtkPolyDataMapper()
Mapper4.SetInputConnection(Geometry4.GetOutputPort())
Mapper4.SetImmediateModeRendering(0)
Mapper4.SetScalarRange(0,1)
Mapper4.SetScalarVisibility(0)
Mapper4.SetScalarModeToDefault()
Actor4 = vtk.vtkActor()
Actor4.SetMapper(Mapper4)
Actor4.GetProperty().SetRepresentationToSurface()
Actor4.GetProperty().SetInterpolationToGouraud()
Actor4.GetProperty().SetColor(1,1,1)
Ren1.AddActor(Actor4)
probeLine = vtk.vtkLineSource()
probeLine.SetPoint1(0,67,10)
probeLine.SetPoint2(67,0,50)
probeLine.SetResolution(500)
vtkMPIController.controler()
Probe0 = vtk.vtkPProbeFilter()
Probe0.SetSourceConnection(ironProt0.GetOutputPort())
Probe0.SetInputConnection(probeLine.GetOutputPort())
Probe0.SetController(controler)
Tuber0 = vtk.vtkTubeFilter()
Tuber0.SetInputConnection(Probe0.GetOutputPort())
Tuber0.SetNumberOfSides(10)
Tuber0.SetCapping(0)
Tuber0.SetRadius(1)
Tuber0.SetVaryRadius(1)
Tuber0.SetRadiusFactor(10)
Tuber0.Update()
Mapper6 = vtk.vtkPolyDataMapper()
Mapper6.SetInputConnection(Tuber0.GetOutputPort())
Mapper6.SetImmediateModeRendering(0)
Mapper6.SetScalarRange(0,228)
Mapper6.SetScalarVisibility(1)
Mapper6.SetScalarModeToUsePointFieldData()
Mapper6.ColorByArrayComponent(scalars,-1)
Mapper6.UseLookupTableScalarRangeOn()
Actor6 = vtk.vtkActor()
Actor6.SetMapper(Mapper6)
Actor6.GetProperty().SetRepresentationToSurface()
Actor6.GetProperty().SetInterpolationToGouraud()
Ren1.AddActor(Actor6)
if (numProcs > 1):
    compManager.SetRenderWindow(renWin)
    compManager.InitializePieces()
    pass
renWin.SetWindowName("Process " + str(myProcId) + "")
if (numProcs < 2):
    renWin.Render()
    pass
# --- end of script --
