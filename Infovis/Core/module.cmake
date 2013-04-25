vtk_module(vtkInfovisCore
  GROUPS
    StandAlone
  DEPENDS
    vtkCommonDataModel
    vtkCommonSystem
    vtkFiltersExtraction
    vtkFiltersGeneral
  TEST_DEPENDS
    vtkInfovisLayout
    vtkRenderingOpenGL
    vtkTestingRendering
    vtkInteractionStyle
    vtkIOInfovis
  )
