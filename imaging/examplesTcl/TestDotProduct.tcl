catch {load vtktcl}
# This script shows the magnitude of an image in frequency domain.


set sliceNumber 22

set VTK_FLOAT              1
set VTK_INT                2
set VTK_SHORT              3
set VTK_UNSIGNED_SHORT     4
set VTK_UNSIGNED_CHAR      5

set VTK_IMAGE_X_AXIS             0
set VTK_IMAGE_Y_AXIS             1
set VTK_IMAGE_Z_AXIS             2
set VTK_IMAGE_TIME_AXIS          3
set VTK_IMAGE_COMPONENT_AXIS     4


# Image pipeline

vtkImageVolume16Reader reader
[reader GetCache] ReleaseDataFlagOff
reader SetDataByteOrderToLittleEndian
reader SetDataDimensions 256 256 93
reader SetFilePrefix "../../../vtkdata/fullHead/headsq"
reader SetDataMask 0x7fff
reader SetOutputScalarType $VTK_FLOAT
#reader DebugOn

vtkImageShiftScale scale2
scale2 SetInput [reader GetOutput]
scale2 SetScale 0.05

vtkImageGradient gradient
gradient SetInput [scale2 GetOutput]
gradient SetAxes $VTK_IMAGE_X_AXIS $VTK_IMAGE_Y_AXIS $VTK_IMAGE_Z_AXIS
gradient ReleaseDataFlagOff
#gradient DebugOn

vtkImageDotProduct magnitude
magnitude SetInput1 [gradient GetOutput]
magnitude SetInput2 [gradient GetOutput]
magnitude ReleaseDataFlagOff

#vtkImageViewer viewer
vtkImageViewer viewer
viewer SetAxes $VTK_IMAGE_X_AXIS $VTK_IMAGE_Y_AXIS $VTK_IMAGE_Z_AXIS
viewer SetInput [magnitude GetOutput]
viewer SetCoordinate2 $sliceNumber
viewer SetColorWindow 1000
viewer SetColorLevel 500
#viewer DebugOn
viewer Render


#make interface
#

frame .slice
button .slice.up -text "Slice Up" -command SliceUp
button .slice.down -text "Slice Down" -command SliceDown

frame .wl
frame .wl.f1
label .wl.f1.windowLabel -text Window
scale .wl.f1.window -from 1 -to 2000 -orient horizontal -command SetWindow
frame .wl.f2
label .wl.f2.levelLabel -text Level
scale .wl.f2.level -from 1 -to 1000 -orient horizontal -command SetLevel
checkbutton .wl.video -text "Inverse Video" -variable inverseVideo -command SetInverseVideo


.wl.f1.window set 1000
.wl.f2.level set 500


pack .slice .wl -side left
pack .slice.up .slice.down -side top
pack .wl.f1 .wl.f2 .wl.video -side top
pack .wl.f1.windowLabel .wl.f1.window -side left
pack .wl.f2.levelLabel .wl.f2.level -side left


proc SliceUp {} {
   global sliceNumber
   if {$sliceNumber < 92} {set sliceNumber [expr $sliceNumber + 1]}
   puts $sliceNumber
   viewer SetCoordinate2 $sliceNumber
   viewer Render
}

proc SliceDown {} {
   global sliceNumber
   if {$sliceNumber > 0} {set sliceNumber [expr $sliceNumber - 1]}
   puts $sliceNumber
   viewer SetCoordinate2 $sliceNumber
   viewer Render
}

proc SetWindow window {
   viewer SetColorWindow $window
   viewer Render
}

proc SetLevel level {
   viewer SetColorLevel $level
   viewer Render
}

proc SetInverseVideo {} {
   if { $inverseVideo == 0 } {
      viewer SetWindow -255
   } else {
      viewer SetWindow 255
   }		
   viewer Render
}





#$renWin Render
#wm withdraw .








