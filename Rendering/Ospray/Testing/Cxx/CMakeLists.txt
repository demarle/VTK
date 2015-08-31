vtk_add_test_cxx(${vtk-module}CxxTests tests
  TestOsprayPass.cxx,NO_DATA,NO_VALID
  TestOspraySceneGraph.cxx,NO_DATA,NO_VALID,NO_OUTPUT
  )

vtk_test_cxx_executable(${vtk-module}CxxTests tests)
