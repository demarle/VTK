#
# Choose the vtkSMP implementation
#
set(VTK_SMP_CMAKE_IMPLEMENTATION_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMP${VTK_SMP_IMPLEMENTATION_TYPE}Implementation.cmake)

include(${VTK_SMP_CMAKE_IMPLEMENTATION_FILE})

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -std=c++11")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -std=c++11")

SET(KIT SMP)
SET(UKIT SMP)
SET(KIT_INTERFACE_LIBRARIES vtkCommon vtkGraphics ${VTK_SMP_IMPLEMENTATION_LIBRARIES})

set(Kit_SRCS
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  vtkSMP.cxx
  vtkSMPObjectFactory.cxx
  vtkSMPTransform.cxx
  vtkSMPContourFilter.cxx
  vtkSMPWarpVector.cxx
  vtkSMPMergePoints.cxx
  vtkSMPMinMaxTree.cxx
  vtkSMPZCurve.cxx
  vtkSMPExecutive.cxx
  vtkSMPDistributePolyData.cxx
  vtkSMPImageConvolve.cxx
  vtkSMPImageDotProduct.cxx
  vtkSMPPipeline.cxx
  vtkSMPAlgorithm.cxx
  vtkSMPPolyDataAlgorithm.cxx
  vtkSMPPointSetAlgorithm.cxx
  vtkSMPContourFilter2.cxx
  vtkSMPClipDataSet.cxx
  vtkSMPImageGaussianSmooth.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkSMP.cxx
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  WRAP_EXCLUDE
)

SET_SOURCE_FILES_PROPERTIES(
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPThreadLocalStorage.h
  HEADER_FILE_ONLY
)

SET_SOURCE_FILES_PROPERTIES(
  vtkSMP.cxx
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  PROPERTIES WRAP_SPECIAL 1
)

SET_SOURCE_FILES_PROPERTIES(
  vtkFunctor
  vtkFunctorInitializable
  vtkTask
  vtkSMPAlgorithm
  ABSTRACT
)

SET(Kit_EXTRA_SRCS)
SET(Kit_EXTRA_CMDS)

#-----------------------------------------------------------------------------
# Include CMake code common to all kits.
INCLUDE(${VTK_CMAKE_DIR}/KitCommonBlock.cmake)
#-----------------------------------------------------------------------------

IF(NOT VTK_INSTALL_NO_DEVELOPMENT)
  SET(__inst_files
    ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.h
    ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
    vtkSMP.h
    vtkSMPObjectFactory.h
    vtkSMPTransform.h
    vtkSMPContourFilter.h
    vtkSMPWarpVector.h
    vtkSMPMergePoints.h
    vtkSMPMinMaxTree.h
    vtkSMPZCurve.h
    vtkSMPExecutive.h
    vtkSMPDistributePolyData.h
    vtkSMPImageConvolve.h
    vtkSMPImageDotProduct.h
    vtkSMPPipeline.h
    vtkSMPAlgorithm.h
    vtkSMPPolyDataAlgorithm.h
    vtkSMPPointSetAlgorithm.h
    vtkSMPContourFilter2.h
    vtkSMPClipDataSet.h
    vtkSMPImageGaussianSmooth.h
    )

  INSTALL(FILES ${__inst_files}
    DESTINATION ${VTK_INSTALL_INCLUDE_DIR_CM24}
    COMPONENT Development)

  SET(__inst_files
    ${VTK_SOURCE_DIR}/${KIT}/Testing/Cxx/SMP.cxx
    )

  INSTALL(FILES ${__inst_files}
    DESTINATION ${VTK_INSTALL_PACKAGE_DIR_CM24}/testing
    COMPONENT Development)
ENDIF(NOT VTK_INSTALL_NO_DEVELOPMENT)
