set(VTK_SMP_IMPLEMENTATION_TYPE "Sequential" CACHE STRING
  "Name of the implementation of SMP techniques."
)
set_property(CACHE VTK_SMP_IMPLEMENTATION_TYPE PROPERTY STRINGS Sequential Kaapi OpenMP TBB)

if( NOT ("${VTK_SMP_IMPLEMENTATION_TYPE}" STREQUAL "Kaapi" OR
         "${VTK_SMP_IMPLEMENTATION_TYPE}" STREQUAL "TBB" OR
         "${VTK_SMP_IMPLEMENTATION_TYPE}" STREQUAL "OpenMP"))
  set(VTK_SMP_IMPLEMENTATION_TYPE "Sequential")
endif()

set(VTK_SMP_CMAKE_IMPLEMENTATION_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMP${VTK_SMP_IMPLEMENTATION_TYPE}Implementation.cmake)
include(${VTK_SMP_CMAKE_IMPLEMENTATION_FILE})

message("${VTK_SMP_IMPLEMENTATION_TYPE}")
set(vtkParallelSMP_SYSTEM_INCLUDE_DIRS ${VTK_SMP_IMPLEMENTATION_HEADER_DIR})

set(SMP_Core
  vtkSMP.cxx
  vtkFunctor.cxx
  vtkFunctorInitializable.cxx
  vtkTask.cxx
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  vtkSMPMinMaxTree.cxx
)

set(SMP_Filters
  vtkSMPContourFilter.cxx
  vtkSMPMergePoints.cxx
  vtkSMPTransform.cxx
  #vtkSMPTransformFilter.cxx
  vtkSMPWarpVector.cxx
  vtkSMPZCurve.cxx
)

set(Module_SRCS
  ${SMP_Core}
  ${SMP_Filters}
)

vtk_module_library(vtkParallelSMP ${Module_SRCS})
target_link_libraries(vtkParallelSMP gomp tbb)
