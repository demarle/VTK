find_package(OSPRay REQUIRED)
include_directories(${OSPRAY_INCLUDE_DIRS})

set(Module_SRCS
  vtkOsprayViewNodeFactory.cxx
  vtkOsprayWindowViewNode.cxx
  )

vtk_module_library(vtkRenderingOspray ${Module_SRCS})

target_link_libraries(${vtk-module} LINK_PRIVATE ${OSPRAY_LIBRARIES} ${QT_LIBRARIES})

# OSPRay_Core uses MMTime which is in it's own special library.
if(WIN32)
  target_link_libraries(${vtk-module} LINK_PRIVATE Winmm)
endif()
