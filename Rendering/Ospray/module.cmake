vtk_module(vtkRenderingOspray
  DEPENDS
    vtkRenderingSceneGraph
  TEST_DEPENDS
    vtkTestingCore
    vtkRendering${VTK_RENDERING_BACKEND}
  EXCLUDE_FROM_ALL
  )
