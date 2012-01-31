project(KaapiBench)

if(NOT VTK_BINARY_DIR)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
endif(NOT VTK_BINARY_DIR)

set(HDRS
  # Ugly hack to have WIP.txt in Qt-Creator hierarchie
  "WIP.txt"
)

include_directories("include" ${VTK_TDX_INCLUDE_PATH})

IF(VTK_USE_SMP)
  string(TOLOWER ${VTK_SMP_IMPLEMENTATION_TYPE} SMP_NAME)

  add_definitions(-DVTK_CAN_USE_SMP)

  add_executable(${SMP_NAME}Bench MACOSX_BUNDLE ${HDRS} "main.cpp")
  target_link_libraries(${SMP_NAME}Bench vtkIO vtkRendering vtkSMP)

  install(TARGETS ${SMP_NAME}Bench RUNTIME DESTINATION bin)
ELSE(VTK_USE_SMP)
  add_executable(purevtkBench MACOSX_BUNDLE ${HDRS} "main.cpp")
  target_link_libraries(purevtkBench vtkIO vtkRendering vtkGraphics)

  install(TARGETS purevtkBench RUNTIME DESTINATION bin)
ENDIF(VTK_USE_SMP)
