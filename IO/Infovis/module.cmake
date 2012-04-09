vtk_module(vtkIOInfovis
  GROUPS
    StandAlone
  DEPENDS
    vtkCommonDataModel
    vtkCommonSystem
    vtkCommonMisc
    vtkIOCore
    vtkInfovisCore
    vtklibxml2
  TEST_DEPENDS
    vtkInfovisLayout
    vtkRenderingCore
    vtkTestingRendering
  )
