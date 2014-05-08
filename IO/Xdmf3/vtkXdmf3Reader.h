/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXdmf3Reader.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkXdmf3Reader - Reads <tt>eXtensible Data Model and Format</tt> files
// .SECTION Description
// vtkXdmf3Reader reads XDMF data files so that they can be visualized using
// VTK. The output data produced by this reader depends on the number of grids
// in the data file. If the data file has a single domain with a single grid,
// then the output type is a vtkDataSet subclass of the appropriate type,
// otherwise it's a vtkMultiBlockDataSet.
//
// .SECTION Caveats
// Uses the XDMF API (http://www.xdmf.org)

#ifndef __vtkXdmf3Reader_h
#define __vtkXdmf3Reader_h

#include "vtkIOXdmf3Module.h" // For export macro
#include "vtkDataReader.h"

class VTKIOXDMF3_EXPORT vtkXdmf3Reader : public vtkDataReader
{
public:
  static vtkXdmf3Reader* New();
  vtkTypeMacro(vtkXdmf3Reader, vtkDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Determine if the file can be read with this reader.
  virtual int CanReadFile(const char* filename);

protected:
  vtkXdmf3Reader();
  ~vtkXdmf3Reader();

  //Overridden to announce that we make general DataObjects.
  virtual int FillOutputPortInformation(int port, vtkInformation *info);

  //Overridden to handle RDO requests the way we need to
  virtual int ProcessRequest(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);

  //Overridden to create the correct vtkDataObject subclass for the file.
  virtual int RequestDataObject(
    vtkInformationVector *);

  //Overridden to announce temporal information and to participate in
  //structured extent splitting.
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);

  //Read the XDMF and HDF input files and fill in vtk data objects.
  virtual int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);

private:
  vtkXdmf3Reader(const vtkXdmf3Reader&); // Not implemented
  void operator=(const vtkXdmf3Reader&); // Not implemented

  class Internals;
  Internals *Internal;
};

#endif
