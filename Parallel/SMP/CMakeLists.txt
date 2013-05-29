set(Module_SRCS
  vtkSMP.cxx
  vtkSMPContourFilter.cxx
  vtkSMPImplementation.cxx
  vtkSMPMergePoints.cxx
  vtkSMPMinMaxTree.cxx
  vtkSMPTransform.cxx
  #vtkSMPTransformFilter.cxx
  vtkSMPWarpVector.cxx
  vtkSMPZCurve.cxx
)

vtk_module_library(vtkParallelSMP ${Module_SRCS})
