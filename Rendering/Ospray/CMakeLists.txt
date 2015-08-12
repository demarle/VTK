set(Module_SRCS
  vtkOsprayViewNodeFactory.cxx
  vtkOsprayWindowViewNode.cxx
  )

vtk_module_library(vtkRenderingOspray ${Module_SRCS})
