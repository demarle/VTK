package require vtktcl

# Test the median filter.






# Image pipeline

vtkPNGReader reader
reader SetFileName $VTK_DATA_ROOT/Data/fullhead15.png

vtkImageMedian3D median
median SetInput [reader GetOutput]
median SetKernelSize 7 7 1
median ReleaseDataFlagOff


vtkImageViewer viewer
#viewer DebugOn
viewer SetInput [median GetOutput]
viewer SetColorWindow 2000
viewer SetColorLevel 1000

viewer Render








