get_filename_component(VTK_SMP_IMPLEMENTATION_HEADER_DIR ${VTK_SMP_CMAKE_IMPLEMENTATION_FILE} PATH)
include_directories(${VTK_SMP_IMPLEMENTATION_HEADER_DIR})

FIND_PACKAGE(TBB REQUIRED)
include_directories(${TBB_INCLUDE_DIRS})

set(VTK_SMP_IMPLEMENTATION_LIBRARIES ${TBB_LIBRARIES})
