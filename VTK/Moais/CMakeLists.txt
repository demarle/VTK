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
  add_executable(KaapiBench MACOSX_BUNDLE ${HDRS} "main.cpp")
  target_link_libraries(KaapiBench vtkIO vtkRendering vtkSMP "numa")

  install(TARGETS KaapiBench RUNTIME DESTINATION bin)
ELSE(VTK_USE_SMP)
  add_executable(SequentialBench MACOSX_BUNDLE ${HDRS} "main_seq.cpp")
  target_link_libraries(SequentialBench vtkIO vtkRendering vtkGraphics)

  install(TARGETS SequentialBench RUNTIME DESTINATION bin)
ENDIF(VTK_USE_SMP)
