vtk_module(vtkInteractionWidgets
  GROUPS
    Rendering
  DEPENDS
    vtkRenderingAnnotation
    vtkRenderingFreeType
    vtkRenderingVolume
    vtkFiltersModeling
    vtkFiltersHybrid
    vtkImagingGeneral
    vtkImagingHybrid
    vtkInteractionStyle
  TEST_DEPENDS
    vtkIOLegacy
    vtkIOXML
    vtkRenderingVolumeOpenGL
    vtkRenderingFreeTypeOpenGL
    vtkTestingRendering
    vtkInteractionStyle
    vtkFiltersModeling
    vtkFiltersFlowPaths
    vtkRenderingLOD
    vtkImagingStencil
  )
