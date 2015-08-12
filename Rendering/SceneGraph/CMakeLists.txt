set(Module_SRCS
  vtkViewNode.cxx
  vtkWindowViewNode.cxx
  )

vtk_module_library(vtkRenderingSceneGraph ${Module_SRCS})
