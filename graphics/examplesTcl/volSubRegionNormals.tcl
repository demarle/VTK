# Test the recomputation of normals within a subregion
catch {load vtktcl}
source ../../examplesTcl/vtkInt.tcl

vtkSLCReader reader
    reader SetFileName "../../../vtkdata/bolt.slc"

vtkPiecewiseFunction opacityTransferFunction
    opacityTransferFunction AddPoint   80  0.0
    opacityTransferFunction AddPoint  100  1.0

vtkColorTransferFunction colorTransferFunction
    colorTransferFunction AddRGBPoint     0 1.0 1.0 1.0
    colorTransferFunction AddRGBPoint   255 1.0 1.0 1.0

vtkVolumeProperty volumeProperty
    volumeProperty SetColor colorTransferFunction
    volumeProperty SetScalarOpacity opacityTransferFunction
    volumeProperty ShadeOn
    volumeProperty SetInterpolationTypeToLinear

vtkFiniteDifferenceGradientEstimator gradest

vtkVolumeRayCastCompositeFunction  compositeFunction

vtkVolumeRayCastMapper volumeMapper
    volumeMapper SetScalarInput [reader GetOutput]
    volumeMapper SetVolumeRayCastFunction compositeFunction
    volumeMapper SetSampleDistance 0.5
    volumeMapper SetGradientEstimator gradest

vtkVolume volume
    volume SetVolumeMapper volumeMapper
    volume SetVolumeProperty volumeProperty

vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
    renWin SetSize 256 256
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

ren1 AddVolume volume
ren1 SetBackground 0.1 0.2 0.4
renWin Render

[ren1 GetActiveCamera] Zoom 1.6

gradest BoundsClipOn

## First bounds intentionally out of range for testing
gradest SetSampleSpacingInVoxels 5
gradest SetBounds -1000 1000 -1000 1000 -1000 1000
renWin Render

gradest SetSampleSpacingInVoxels 1
gradest SetBounds 0 50 0 50 0 50
renWin Render

gradest SetBounds 0 30 70 90 0 50
renWin Render

proc TkCheckAbort {} {
  set foo [renWin GetEventPending]
  if {$foo != 0} {renWin SetAbortRender 1}
}
renWin SetAbortCheckMethod {TkCheckAbort}

iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize


renWin SetFileName "valid/volSubRegionNormals.ppm"
renWin SaveImageAsPPM

wm withdraw .

