set(Module_SRCS
  vtkViewNode.cxx
  vtkViewNodeCollection.cxx
  vtkViewNodeFactory.cxx
  vtkWindowViewNode.cxx
  )

vtk_module_library(vtkRenderingSceneGraph ${Module_SRCS})
