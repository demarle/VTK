vtk_module(vtkMathText
  DEPENDS
    vtkRenderingCore
    vtkWrappingPython
  TEST_DEPENDS
    vtkTestingRendering
    vtkInteractionImage
    vtkRenderingOpenGL
  EXCLUDE_FROM_WRAPPING
  )
