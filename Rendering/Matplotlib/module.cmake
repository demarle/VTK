vtk_module(vtkRenderingMatplotlib
  IMPLEMENTS
    vtkRenderingMathText
  DEPENDS
    vtkRenderingCore
    vtkWrappingPython
  TEST_DEPENDS
    vtkInteractionImage
    vtkIOExport
    vtkTestingRendering
    vtkRenderingGL2PS
    vtkRenderingOpenGL
    vtkViewsContext2D
  )
