vtk_module(vtkIOGeometry
  GROUPS
    StandAlone
  DEPENDS
    vtkCommonDataModel
    vtkCommonSystem
    vtkCommonMisc
    vtkIOCore
    vtkzlib
  TEST_DEPENDS
    vtkIOAMR
    vtkIOLegacy
    vtkFiltersGeometry
    vtkRenderingOpenGL
    vtkTestingRendering
  )
