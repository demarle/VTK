#!/usr/local/bin/python

from vtkpython import *
from WindowLevelInterface import *

# Halves the size of the image in the x, Y and Z dimensions.
# Computes the whole volume, but streams the input using the streaming
# functionality in vtkImageFilter class.


# Image pipeline

reader = vtkImageReader()
reader.SetDataByteOrderToLittleEndian()
reader.SetDataExtent(0,255,0,255,1,93)
reader.SetFilePrefix("../../../vtkdata/fullHead/headsq")
reader.SetDataMask(0x7fff)

shrink = vtkImageShrink3D()
shrink.SetInput(reader.GetOutput())
shrink.SetShrinkFactors(2,2,2)
shrink.AveragingOn()
shrink.SetInputMemoryLimit(150)
#shrink.DebugOn()

viewer = vtkImageViewer()
#viewer.DebugOn()
viewer.SetInput(shrink.GetOutput())
viewer.SetZSlice(22)
viewer.SetColorWindow(3000)
viewer.SetColorLevel(1500)

#make interface
WindowLevelInterface(viewer)
