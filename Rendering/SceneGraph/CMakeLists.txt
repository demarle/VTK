set(Module_SRCS
  vtkActorNode.cxx
  vtkCameraNode.cxx
  vtkLightNode.cxx
  vtkRendererNode.cxx
  vtkViewNode.cxx
  vtkViewNodeCollection.cxx
  vtkViewNodeFactory.cxx
  vtkWindowNode.cxx
  )

vtk_module_library(vtkRenderingSceneGraph ${Module_SRCS})
