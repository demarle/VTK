#
# Choose the vtkSMP implementation
#
set(VTK_SMP_CMAKE_IMPLEMENTATION_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMP${VTK_SMP_IMPLEMENTATION_TYPE}Implementation.cmake)

include(${VTK_SMP_CMAKE_IMPLEMENTATION_FILE})

SET(KIT SMP)
SET(UKIT SMP)
SET(KIT_INTERFACE_LIBRARIES vtkCommon vtkGraphics ${VTK_SMP_IMPLEMENTATION_LIBRARIES})

set(Kit_SRCS
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  vtkSMP.cxx
  vtkSMPObjectFactory.cxx
  vtkSMPTransform.cxx
#  vtkSMPTransformFilter.cxx
  vtkSMPWarpVector.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkSMP.cxx
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  WRAP_EXCLUDE
)
SET_SOURCE_FILES_PROPERTIES(
  vtkSMP.cxx
  ${VTK_SMP_IMPLEMENTATION_TYPE}/vtkSMPImplementation.cxx
  PROPERTIES WRAP_SPECIAL 1
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
    vtkSMP.h
    vtkSMPObjectFactory.h
    vtkSMPTransform.h
#    vtkSMPTransformFilter.h
    vtkSMPWarpVector.h
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
