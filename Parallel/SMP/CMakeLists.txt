set(VTK_SMP_IMPLEMENTATION_TYPE "Sequential" CACHE STRING
  "Name of the implementation of SMP techniques. Valid name are Kaapi and OpenMP. Other names backup to sequential implementation"
)

if( NOT ("${VTK_SMP_IMPLEMENTATION_TYPE}" STREQUAL "Kaapi" OR
         "${VTK_SMP_IMPLEMENTATION_TYPE}" STREQUAL "OpenMP"))
  set(VTK_SMP_IMPLEMENTATION_TYPE "Sequential")
endif()

set(VTK_SMP_CMAKE_IMPLEMENTATION_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMP${VTK_SMP_IMPLEMENTATION_TYPE}Implementation.cmake)
include(${VTK_SMP_CMAKE_IMPLEMENTATION_FILE})

set(Module_SRCS
  vtkSMP.cxx
  vtkSMPContourFilter.cxx
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  vtkSMPMergePoints.cxx
  vtkSMPMinMaxTree.cxx
  vtkSMPTransform.cxx
  #vtkSMPTransformFilter.cxx
  vtkSMPWarpVector.cxx
  vtkSMPZCurve.cxx
)

vtk_module_library(vtkParallelSMP ${Module_SRCS})
target_link_libraries(vtkParallelSMP gomp)
