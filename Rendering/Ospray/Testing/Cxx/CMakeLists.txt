vtk_add_test_cxx(${vtk-module}CxxTests tests
  #tests that ospray scene graph classes are constructed OK
  TestOspraySceneGraph.cxx,NO_DATA,NO_VALID,NO_OUTPUT
  #tests that basic mesh rendering and state translation are correct
  TestOsprayRenderMesh.cxx,NO_DATA,NO_VALID
  #tests hotswappability of ospray and gl
  TestOsprayPass.cxx,NO_DATA,NO_VALID
  #tests that dynamic scene contents work acceptable
  TestOsprayDynamicScene.cxx,NO_DATA,NO_VALID,NO_OUTPUT
  #tests that we can have multiple render layers
  TestOsprayLayers.cxx,NO_DATA,NO_VALID,NO_OUTPUT
  #tests that tests the depth compositing works
  TestOsprayComposite.cxx,NO_DATA,NO_VALID,NO_OUTPUT
  )

vtk_test_cxx_executable(${vtk-module}CxxTests tests)
