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

find_program(HOSTNAME_CMD NAMES hostname)
exec_program(${HOSTNAME_CMD} ARGS OUTPUT_VARIABLE MY_HOSTNAME)
if("${MY_HOSTNAME}" STREQUAL "idfreeze")
  add_definitions(-DHIDE_VTK_WINDOW)
endif("${MY_HOSTNAME}" STREQUAL "idfreeze")

IF(VTK_USE_SMP)
  string(TOLOWER ${VTK_SMP_IMPLEMENTATION_TYPE} SMP_NAME)

  include_directories("~/.local/usr/include")
  add_definitions(-DVTK_CAN_USE_SMP=1 -DVTK_WINDOW_NAME=\"${SMP_NAME}\")

  add_executable(${SMP_NAME}Bench MACOSX_BUNDLE "main.cpp" ${HDRS})
  target_link_libraries(${SMP_NAME}Bench vtkIO vtkRendering vtkSMP vtkParallel vtkHybrid)

  install(TARGETS ${SMP_NAME}Bench RUNTIME DESTINATION bin)

  add_executable(viewMesh MACOSX_BUNDLE "viewer.cpp")
  target_link_libraries(viewMesh vtkIO vtkSMP)
  install(TARGETS viewMesh RUNTIME DESTINATION bin)

ELSE(VTK_USE_SMP)
  IF(VTK_USE_MPI)
    find_package(MPI REQUIRED)
    include_directories(${MPI_CXX_INCLUDE_PATH})

    set(HDRS ${HDRS} "vtkPEnSightGoldBinaryReader.h" "vtkPEnSightGoldReader.h" "vtkPEnSightReader.h" "vtkPGenericEnSightReader.h")
    set(SRCS "main_mpi.cpp" "vtkPEnSightGoldBinaryReader.cxx" "vtkPEnSightGoldReader.cxx" "vtkPEnSightReader.cxx" "vtkPGenericEnSightReader.cxx")

    add_executable(mpiBench MACOSX_BUNDLE ${SRCS} ${HDRS})
    target_link_libraries(mpiBench vtkIO vtkRendering vtkGraphics vtkParallel MapReduceMPI)

    install(TARGETS mpiBench RUNTIME DESTINATION bin)
  ELSE(VTK_USE_MPI)
    add_definitions(-DVTK_WINDOW_NAME=\"VTKreference\")

    add_executable(purevtkBench MACOSX_BUNDLE "main.cpp" ${HDRS})
    target_link_libraries(purevtkBench vtkIO vtkRendering vtkGraphics vtkHybrid)

    install(TARGETS purevtkBench RUNTIME DESTINATION bin)
  ENDIF(VTK_USE_MPI)
ENDIF(VTK_USE_SMP)

add_executable(buildBenchMeshes MACOSX_BUNDLE "meshes.cpp")
target_link_libraries(buildBenchMeshes vtkIO vtkRendering)
install(TARGETS buildBenchMeshes RUNTIME DESTINATION bin)
