#!/usr/local/bin/python

from libVTKCommonPython import *
from libVTKGraphicsPython import *

#catch  load vtktcl 
# this is a tcl version of hawaii coloration

# get the interactor ui
#source ../../examplesTcl/vtkInt.tcl
#source ../../examplesTcl/colors.tcl
from colors import *
# Create the RenderWindow, Renderer and both Actors
#
ren = vtkRenderer()
renWin = vtkRenderWindow()
renWin.AddRenderer(ren)
iren = vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# read a vtk file
#
hawaii = vtkPolyDataReader()
hawaii.SetFileName("../../../vtkdata/honolulu.vtk")
hawaii.Update()
elevation = vtkElevationFilter()
elevation.SetInput(hawaii.GetOutput())
elevation.SetLowPoint(0,0,0)
elevation.SetHighPoint(0,0,1000)
elevation.SetScalarRange(0,1000)
lut = vtkLookupTable()
lut.SetHueRange(0.7,0)
lut.SetSaturationRange(1.0,0)
lut.SetValueRange(0.5,1.0)
#    lut SetNumberOfColors 8
#    lut Build
#    eval lut SetTableValue 0 $turquoise_blue 1
#    eval lut SetTableValue 1 $sea_green_medium 1
#    eval lut SetTableValue 2 $sea_green_dark 1
#    eval lut SetTableValue 3 $olive_green_dark 1
#    eval lut SetTableValue 4 $brown 1
#    eval lut SetTableValue 5 $beige 1
#    eval lut SetTableValue 6 $light_beige 1
#    eval lut SetTableValue 7 $bisque 1
hawaiiMapper = vtkDataSetMapper()
hawaiiMapper.SetInput(elevation.GetOutput())
hawaiiMapper.SetScalarRange(0,1000)
hawaiiMapper.SetLookupTable(lut)
hawaiiActor = vtkActor()
hawaiiActor.SetMapper(hawaiiMapper)

# Add the actors to the renderer, set the background and size
#
ren.AddActor(hawaiiActor)
ren.SetBackground(1,1,1)
renWin.SetSize(500,500)
renWin.DoubleBufferOff()
ren.SetBackground(0.1,0.2,0.4)

# render the image
#
iren.Initialize()

ren.GetActiveCamera().Zoom(1.8)
renWin.Render()
#renWin SetFileName "hawaii.tcl.ppm"
#renWin SaveImageAsPPM

# prevent the tk window from showing up then start the event loop
#wm withdraw .


iren.Start()
