vtk_add_test_cxx(TestSMPUG.cxx NO_DATA NO_VALID NO_OUTPUT)
vtk_add_test_cxx(TestSMPPD.cxx NO_DATA NO_VALID NO_OUTPUT)
vtk_add_test_cxx(TestSMPPD2.cxx NO_DATA NO_VALID NO_OUTPUT)
vtk_test_cxx_executable(${vtk-module}CxxTests RENDERING_FACTORY)
