set(Module_SRCS
  vtkSILBuilder.cxx
  vtkXdmfDataArray.cxx
  vtkXdmfHeavyData.cxx
  vtkXdmfReader.cxx
  vtkXdmfReaderInternal.cxx
  vtkXdmfWriter.cxx
  )

set_source_files_properties(
  vtkXdmfReaderInternal
  vtkXdmfHeavyData
  WRAP_EXCLUDE
  )

set_source_files_properties(
  vtkXdmfDataArray
  WRAP_EXCLUDE
  )

vtk_module_library(vtkIOXdmf2 ${Module_SRCS})
