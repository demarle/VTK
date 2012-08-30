#!/usr/bin/env python

# Simple volume rendering example.
reader = vtk.vtkSLCReader()
reader.SetFileName("" + str(VTK_DATA_ROOT) + "/Data/sphere.slc")
# Create transfer functions for opacity and color
opacityTransferFunction = vtk.vtkPiecewiseFunction()
opacityTransferFunction.AddPoint(0,0.0)
opacityTransferFunction.AddPoint(30,0.0)
opacityTransferFunction.AddPoint(80,0.5)
opacityTransferFunction.AddPoint(255,0.5)
colorTransferFunction = vtk.vtkColorTransferFunction()
colorTransferFunction.AddRGBPoint(0.0,0.0,0.0,0.0)
colorTransferFunction.AddRGBPoint(64.0,1.0,0.0,0.0)
colorTransferFunction.AddRGBPoint(128.0,0.0,0.0,1.0)
colorTransferFunction.AddRGBPoint(192.0,0.0,1.0,0.0)
colorTransferFunction.AddRGBPoint(255.0,0.0,0.2,0.0)
# Create properties, mappers, volume actors, and ray cast function
volumeProperty = vtk.vtkVolumeProperty()
volumeProperty.SetColor(colorTransferFunction)
volumeProperty.SetScalarOpacity(opacityTransferFunction)
volumeProperty.SetInterpolationTypeToLinear()
volumeProperty.ShadeOn()
ren1 = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(ren1)
renWin.SetSize(600,300)
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)
renWin.SetMultiSamples(0)
ren1.SetBackground(0.1,0.2,0.4)
i = 0
while i < 2:
    j = 0
    while j < 4:
        locals()[get_variable_name("volumeMapper_", i, "_", j, "")] = vtk.vtkVolumeTextureMapper2D()
        locals()[get_variable_name("volumeMapper_", i, "_", j, "")].SetInputConnection(reader.GetOutputPort())
        locals()[get_variable_name("volumeMapper_", i, "_", j, "")].CroppingOn()
        locals()[get_variable_name("volumeMapper_", i, "_", j, "")].SetCroppingRegionPlanes(17.5,32.5,17.5,32.5,17.5,32.5)
        locals()[get_variable_name("volume_", i, "_", j, "")] = vtk.vtkVolume()
        locals()[get_variable_name("volume_", i, "_", j, "")].SetMapper(locals()[get_variable_name("volumeMapper_", i, "_", j, "")])
        locals()[get_variable_name("volume_", i, "_", j, "")].SetProperty(volumeProperty)
        locals()[get_variable_name("outline_", i, "_", j, "")] = vtk.vtkVolumeOutlineSource()
        locals()[get_variable_name("outline_", i, "_", j, "")].SetVolumeMapper(locals()[get_variable_name("volumeMapper_", i, "_", j, "")])
        locals()[get_variable_name("outlineMapper_", i, "_", j, "")] = vtk.vtkDataSetMapper()
        locals()[get_variable_name("outlineMapper_", i, "_", j, "")].SetInputConnection(locals()[get_variable_name("outline_", i, "_", j, "")].GetOutputPort())
        locals()[get_variable_name("outlineActor_", i, "_", j, "")] = vtk.vtkActor()
        locals()[get_variable_name("outlineActor_", i, "_", j, "")].SetMapper(locals()[get_variable_name("outlineMapper_", i, "_", j, "")])
        locals()[get_variable_name("userMatrix_", i, "_", j, "")] = vtk.vtkTransform()
        locals()[get_variable_name("userMatrix_", i, "_", j, "")].PostMultiply()
        locals()[get_variable_name("userMatrix_", i, "_", j, "")].Identity()
        locals()[get_variable_name("userMatrix_", i, "_", j, "")].Translate(-25,-25,-25)
        if (i == 0):
            locals()[get_variable_name("userMatrix_", i, "_", j, "")].RotateX(expr.expr(globals(), locals(),["j","*","87","+","23"]))
            locals()[get_variable_name("userMatrix_", i, "_", j, "")].RotateY(16)
            pass
        else:
            locals()[get_variable_name("userMatrix_", i, "_", j, "")].RotateX(27)
            locals()[get_variable_name("userMatrix_", i, "_", j, "")].RotateY(expr.expr(globals(), locals(),["j","*","87","+","19"]))
            pass
        locals()[get_variable_name("userMatrix_", i, "_", j, "")].Translate(expr.expr(globals(), locals(),["j","*","65","+","25"]),expr.expr(globals(), locals(),["i","*","55","+","25"]),0)
        locals()[get_variable_name("volume_", i, "_", j, "")].SetUserTransform(locals()[get_variable_name("userMatrix_", i, "_", j, "")])
        locals()[get_variable_name("outlineActor_", i, "_", j, "")].SetUserTransform(locals()[get_variable_name("userMatrix_", i, "_", j, "")])
        ren1.AddViewProp(locals()[get_variable_name("outlineActor_", i, "_", j, "")])
        ren1.AddViewProp(locals()[get_variable_name("volume_", i, "_", j, "")])
        j = j + 1

    i = i + 1

volumeMapper_0_0.SetCroppingRegionFlagsToSubVolume()
volumeMapper_0_1.SetCroppingRegionFlagsToCross()
volumeMapper_0_2.SetCroppingRegionFlagsToInvertedCross()
volumeMapper_0_3.SetCroppingRegionFlags(24600)
volumeMapper_1_0.SetCroppingRegionFlagsToFence()
volumeMapper_1_1.SetCroppingRegionFlagsToInvertedFence()
volumeMapper_1_2.SetCroppingRegionFlags(1)
volumeMapper_1_3.SetCroppingRegionFlags(67117057)
outline_0_1.GenerateScalarsOn()
outline_0_2.GenerateScalarsOn()
outline_0_2.SetActivePlaneId(1)
outline_0_2.SetColor(1,0,1)
outline_0_3.GenerateScalarsOn()
outline_0_3.SetActivePlaneId(2)
outline_0_3.SetActivePlaneColor(0,1,1)
outlineActor_1_0.GetProperty().SetColor(0,1,0)
outline_1_1.GenerateFacesOn()
volume_1_1.VisibilityOff()
outline_1_2.GenerateFacesOn()
outline_1_2.GenerateScalarsOn()
volume_1_2.VisibilityOff()
outline_1_3.GenerateFacesOn()
outline_1_3.GenerateScalarsOn()
outline_1_3.SetActivePlaneId(1)
volume_1_3.VisibilityOff()
ren1.GetCullers().InitTraversal()
culler = ren1.GetCullers().GetNextItem()
culler.SetSortingStyleToBackToFront()
ren1.ResetCamera()
ren1.GetActiveCamera().Zoom(2.35)
renWin.Render()
def TkCheckAbort (__vtk__temp0=0,__vtk__temp1=0):
    foo = renWin.GetEventPending()
    if (foo != 0):
        renWin.SetAbortRender(1)
        pass

renWin.AddObserver("AbortCheckEvent",TkCheckAbort)
iren.Initialize()
# --- end of script --
