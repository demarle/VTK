#!/usr/local/bin/python

from libVTKCommonPython import *
from libVTKGraphicsPython import *

#catch  load vtktcl 
# get the interactor ui
#source ../../examplesTcl/vtkInt.tcl
#source ../../examplesTcl/colors.tcl
from colors import *
ren = vtkRenderer()
renWin = vtkRenderWindow()
renWin.AddRenderer(ren)
iren = vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# read data
#
reader = vtkStructuredGridReader()
reader.SetFileName("../../../vtkdata/office.vtk")
reader.Update()  #force.a.read.to.occur()

length=reader.GetOutput().GetLength()
maxVelocity=reader.GetOutput().GetPointData().GetVectors().GetMaxNorm()
maxTime=(35.0*length/maxVelocity)

table1 = vtkStructuredGridGeometryFilter()
table1.SetInput(reader.GetOutput())
table1.SetExtent(11,15,7,9,8,8)
mapTable1 = vtkPolyDataMapper()
mapTable1.SetInput(table1.GetOutput())
mapTable1.ScalarVisibilityOff()
table1Actor = vtkActor()
table1Actor.SetMapper(mapTable1)
table1Actor.GetProperty().SetColor(.59,.427,.392)

table2 = vtkStructuredGridGeometryFilter()
table2.SetInput(reader.GetOutput())
table2.SetExtent(11,15,10,12,8,8)
mapTable2 = vtkPolyDataMapper()
mapTable2.SetInput(table2.GetOutput())
mapTable2.ScalarVisibilityOff()
table2Actor = vtkActor()
table2Actor.SetMapper(mapTable2)
table2Actor.GetProperty().SetColor(.59,.427,.392)

FilingCabinet1 = vtkStructuredGridGeometryFilter()
FilingCabinet1.SetInput(reader.GetOutput())
FilingCabinet1.SetExtent(15,15,7,9,0,8)
mapFilingCabinet1 = vtkPolyDataMapper()
mapFilingCabinet1.SetInput(FilingCabinet1.GetOutput())
mapFilingCabinet1.ScalarVisibilityOff()
FilingCabinet1Actor = vtkActor()
FilingCabinet1Actor.SetMapper(mapFilingCabinet1)
FilingCabinet1Actor.GetProperty().SetColor(.8,.8,.6)

FilingCabinet2 = vtkStructuredGridGeometryFilter()
FilingCabinet2.SetInput(reader.GetOutput())
FilingCabinet2.SetExtent(15,15,10,12,0,8)
mapFilingCabinet2 = vtkPolyDataMapper()
mapFilingCabinet2.SetInput(FilingCabinet2.GetOutput())
mapFilingCabinet2.ScalarVisibilityOff()
FilingCabinet2Actor = vtkActor()
FilingCabinet2Actor.SetMapper(mapFilingCabinet2)
FilingCabinet2Actor.GetProperty().SetColor(.8,.8,.6)

bookshelf1Top = vtkStructuredGridGeometryFilter()
bookshelf1Top.SetInput(reader.GetOutput())
bookshelf1Top.SetExtent(13,13,0,4,0,11)
mapBookshelf1Top = vtkPolyDataMapper()
mapBookshelf1Top.SetInput(bookshelf1Top.GetOutput())
mapBookshelf1Top.ScalarVisibilityOff()
bookshelf1TopActor = vtkActor()
bookshelf1TopActor.SetMapper(mapBookshelf1Top)
bookshelf1TopActor.GetProperty().SetColor(.8,.8,.6)

bookshelf1Bottom = vtkStructuredGridGeometryFilter()
bookshelf1Bottom.SetInput(reader.GetOutput())
bookshelf1Bottom.SetExtent(20,20,0,4,0,11)
mapBookshelf1Bottom = vtkPolyDataMapper()
mapBookshelf1Bottom.SetInput(bookshelf1Bottom.GetOutput())
mapBookshelf1Bottom.ScalarVisibilityOff()
bookshelf1BottomActor = vtkActor()
bookshelf1BottomActor.SetMapper(mapBookshelf1Bottom)
bookshelf1BottomActor.GetProperty().SetColor(.8,.8,.6)

bookshelf1Front = vtkStructuredGridGeometryFilter()
bookshelf1Front.SetInput(reader.GetOutput())
bookshelf1Front.SetExtent(13,20,0,0,0,11)
mapBookshelf1Front = vtkPolyDataMapper()
mapBookshelf1Front.SetInput(bookshelf1Front.GetOutput())
mapBookshelf1Front.ScalarVisibilityOff()
bookshelf1FrontActor = vtkActor()
bookshelf1FrontActor.SetMapper(mapBookshelf1Front)
bookshelf1FrontActor.GetProperty().SetColor(.8,.8,.6)

bookshelf1Back = vtkStructuredGridGeometryFilter()
bookshelf1Back.SetInput(reader.GetOutput())
bookshelf1Back.SetExtent(13,20,4,4,0,11)
mapBookshelf1Back = vtkPolyDataMapper()
mapBookshelf1Back.SetInput(bookshelf1Back.GetOutput())
mapBookshelf1Back.ScalarVisibilityOff()
bookshelf1BackActor = vtkActor()
bookshelf1BackActor.SetMapper(mapBookshelf1Back)
bookshelf1BackActor.GetProperty().SetColor(.8,.8,.6)

bookshelf1LHS = vtkStructuredGridGeometryFilter()
bookshelf1LHS.SetInput(reader.GetOutput())
bookshelf1LHS.SetExtent(13,20,0,4,0,0)
mapBookshelf1LHS = vtkPolyDataMapper()
mapBookshelf1LHS.SetInput(bookshelf1LHS.GetOutput())
mapBookshelf1LHS.ScalarVisibilityOff()
bookshelf1LHSActor = vtkActor()
bookshelf1LHSActor.SetMapper(mapBookshelf1LHS)
bookshelf1LHSActor.GetProperty().SetColor(.8,.8,.6)

bookshelf1RHS = vtkStructuredGridGeometryFilter()
bookshelf1RHS.SetInput(reader.GetOutput())
bookshelf1RHS.SetExtent(13,20,0,4,11,11)
mapBookshelf1RHS = vtkPolyDataMapper()
mapBookshelf1RHS.SetInput(bookshelf1RHS.GetOutput())
mapBookshelf1RHS.ScalarVisibilityOff()
bookshelf1RHSActor = vtkActor()
bookshelf1RHSActor.SetMapper(mapBookshelf1RHS)
bookshelf1RHSActor.GetProperty().SetColor(.8,.8,.6)

bookshelf2Top = vtkStructuredGridGeometryFilter()
bookshelf2Top.SetInput(reader.GetOutput())
bookshelf2Top.SetExtent(13,13,15,19,0,11)
mapBookshelf2Top = vtkPolyDataMapper()
mapBookshelf2Top.SetInput(bookshelf2Top.GetOutput())
mapBookshelf2Top.ScalarVisibilityOff()
bookshelf2TopActor = vtkActor()
bookshelf2TopActor.SetMapper(mapBookshelf2Top)
bookshelf2TopActor.GetProperty().SetColor(.8,.8,.6)

bookshelf2Bottom = vtkStructuredGridGeometryFilter()
bookshelf2Bottom.SetInput(reader.GetOutput())
bookshelf2Bottom.SetExtent(20,20,15,19,0,11)
mapBookshelf2Bottom = vtkPolyDataMapper()
mapBookshelf2Bottom.SetInput(bookshelf2Bottom.GetOutput())
mapBookshelf2Bottom.ScalarVisibilityOff()
bookshelf2BottomActor = vtkActor()
bookshelf2BottomActor.SetMapper(mapBookshelf2Bottom)
bookshelf2BottomActor.GetProperty().SetColor(.8,.8,.6)

bookshelf2Front = vtkStructuredGridGeometryFilter()
bookshelf2Front.SetInput(reader.GetOutput())
bookshelf2Front.SetExtent(13,20,15,15,0,11)
mapBookshelf2Front = vtkPolyDataMapper()
mapBookshelf2Front.SetInput(bookshelf2Front.GetOutput())
mapBookshelf2Front.ScalarVisibilityOff()
bookshelf2FrontActor = vtkActor()
bookshelf2FrontActor.SetMapper(mapBookshelf2Front)
bookshelf2FrontActor.GetProperty().SetColor(.8,.8,.6)

bookshelf2Back = vtkStructuredGridGeometryFilter()
bookshelf2Back.SetInput(reader.GetOutput())
bookshelf2Back.SetExtent(13,20,19,19,0,11)
mapBookshelf2Back = vtkPolyDataMapper()
mapBookshelf2Back.SetInput(bookshelf2Back.GetOutput())
mapBookshelf2Back.ScalarVisibilityOff()
bookshelf2BackActor = vtkActor()
bookshelf2BackActor.SetMapper(mapBookshelf2Back)
bookshelf2BackActor.GetProperty().SetColor(.8,.8,.6)

bookshelf2LHS = vtkStructuredGridGeometryFilter()
bookshelf2LHS.SetInput(reader.GetOutput())
bookshelf2LHS.SetExtent(13,20,15,19,0,0)
mapBookshelf2LHS = vtkPolyDataMapper()
mapBookshelf2LHS.SetInput(bookshelf2LHS.GetOutput())
mapBookshelf2LHS.ScalarVisibilityOff()
bookshelf2LHSActor = vtkActor()
bookshelf2LHSActor.SetMapper(mapBookshelf2LHS)
bookshelf2LHSActor.GetProperty().SetColor(.8,.8,.6)

bookshelf2RHS = vtkStructuredGridGeometryFilter()
bookshelf2RHS.SetInput(reader.GetOutput())
bookshelf2RHS.SetExtent(13,20,15,19,11,11)
mapBookshelf2RHS = vtkPolyDataMapper()
mapBookshelf2RHS.SetInput(bookshelf2RHS.GetOutput())
mapBookshelf2RHS.ScalarVisibilityOff()
bookshelf2RHSActor = vtkActor()
bookshelf2RHSActor.SetMapper(mapBookshelf2RHS)
bookshelf2RHSActor.GetProperty().SetColor(.8,.8,.6)

window = vtkStructuredGridGeometryFilter()
window.SetInput(reader.GetOutput())
window.SetExtent(20,20,6,13,10,13)
mapWindow = vtkPolyDataMapper()
mapWindow.SetInput(window.GetOutput())
mapWindow.ScalarVisibilityOff()
windowActor = vtkActor()
windowActor.SetMapper(mapWindow)
windowActor.GetProperty().SetColor(.3,.3,.5)

outlet = vtkStructuredGridGeometryFilter()
outlet.SetInput(reader.GetOutput())
outlet.SetExtent(0,0,9,10,14,16)
mapOutlet = vtkPolyDataMapper()
mapOutlet.SetInput(outlet.GetOutput())
mapOutlet.ScalarVisibilityOff()
outletActor = vtkActor()
outletActor.SetMapper(mapOutlet)
outletActor.GetProperty().SetColor(0,0,0)

inlet = vtkStructuredGridGeometryFilter()
inlet.SetInput(reader.GetOutput())
inlet.SetExtent(0,0,9,10,0,6)
mapInlet = vtkPolyDataMapper()
mapInlet.SetInput(inlet.GetOutput())
mapInlet.ScalarVisibilityOff()
inletActor = vtkActor()
inletActor.SetMapper(mapInlet)
inletActor.GetProperty().SetColor(0,0,0)

outline = vtkStructuredGridOutlineFilter()
outline.SetInput(reader.GetOutput())
mapOutline = vtkPolyDataMapper()
mapOutline.SetInput(outline.GetOutput())
outlineActor = vtkActor()
outlineActor.SetMapper(mapOutline)
outlineActor.GetProperty().SetColor(0,0,0)

# Create source for streamtubes
seeds = vtkPointSource()
seeds.SetRadius(0.075)
seeds.SetCenter(0.1,2.7,0.5)
seeds.SetNumberOfPoints(25)
streamers = vtkStreamLine()
streamers.SetInput(reader.GetOutput())
streamers.SetSource(seeds.GetOutput())
streamers.SetMaximumPropagationTime(500)
streamers.SetStepLength(0.1)
streamers.Update()
mapStreamers = vtkPolyDataMapper()
mapStreamers.SetInput(streamers.GetOutput())
mapStreamers.SetScalarRange(  \
	reader.GetOutput().GetPointData().GetScalars().GetRange() )
streamersActor = vtkActor()
streamersActor.SetMapper(mapStreamers)

ren.AddActor(table1Actor)
ren.AddActor(table2Actor)
ren.AddActor(FilingCabinet1Actor)
ren.AddActor(FilingCabinet2Actor)
ren.AddActor(bookshelf1TopActor)
ren.AddActor(bookshelf1BottomActor)
ren.AddActor(bookshelf1FrontActor)
ren.AddActor(bookshelf1BackActor)
ren.AddActor(bookshelf1LHSActor)
ren.AddActor(bookshelf1RHSActor)
ren.AddActor(bookshelf2TopActor)
ren.AddActor(bookshelf2BottomActor)
ren.AddActor(bookshelf2FrontActor)
ren.AddActor(bookshelf2BackActor)
ren.AddActor(bookshelf2LHSActor)
ren.AddActor(bookshelf2RHSActor)
ren.AddActor(windowActor)
ren.AddActor(outletActor)
ren.AddActor(inletActor)
ren.AddActor(outlineActor)
ren.AddActor(streamersActor)

ren.SetBackground(slate_grey)

aCamera = vtkCamera()
aCamera.SetClippingRange(0.726079,36.3039)
aCamera.SetFocalPoint(2.43584,2.15046,1.11104)
aCamera.SetPosition(-4.76183,-10.4426,3.17203)
aCamera.ComputeViewPlaneNormal()
aCamera.SetViewUp(0.0511273,0.132773,0.989827)
aCamera.SetViewAngle(18.604)
aCamera.Zoom(1.2)

ren.SetActiveCamera(aCamera)

renWin.SetSize(500,300)
iren.Initialize()
#renWin SetFileName "office.tcl.ppm"
#renWin SaveImageAsPPM

# interact with data
#wm withdraw .

iren.Start()
