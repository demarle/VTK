set(Module_SRCS
  vtkXdmf3Common.cxx
  vtkXdmf3Reader.cxx
  vtkXdmf3Writer.cxx
  )

set_source_files_properties(
  vtkXdmf3Common
  WRAP_EXCLUDE
)

vtk_module_library(vtkIOXdmf3 ${Module_SRCS})
