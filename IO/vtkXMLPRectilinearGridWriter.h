/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXMLPRectilinearGridWriter.h
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
// .NAME vtkXMLPRectilinearGridWriter - Write PVTK XML RectilinearGrid files.
// .SECTION Description
// vtkXMLPRectilinearGridWriter writes the PVTK XML RectilinearGrid
// file format.  One rectilinear grid input can be written into a
// parallel file format with any number of pieces spread across files.
// The standard extension for this writer's file format is "pvtr".
// This writer uses vtkXMLRectilinearGridWriter to write the
// individual piece files.

// .SECTION See Also
// vtkXMLRectilinearGridWriter

#ifndef __vtkXMLPRectilinearGridWriter_h
#define __vtkXMLPRectilinearGridWriter_h

#include "vtkXMLPStructuredDataWriter.h"

class vtkRectilinearGrid;

class VTK_IO_EXPORT vtkXMLPRectilinearGridWriter : public vtkXMLPStructuredDataWriter
{
public:
  static vtkXMLPRectilinearGridWriter* New();
  vtkTypeRevisionMacro(vtkXMLPRectilinearGridWriter,vtkXMLPStructuredDataWriter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Get/Set the writer's input.
  void SetInput(vtkRectilinearGrid* input);
  vtkRectilinearGrid* GetInput();
  
  // Description:
  // Get the default file extension for files written by this writer.
  const char* GetDefaultFileExtension();
  
protected:
  vtkXMLPRectilinearGridWriter();
  ~vtkXMLPRectilinearGridWriter();
  
  const char* GetDataSetName();
  vtkXMLStructuredDataWriter* CreateStructuredPieceWriter(); 
  void WritePData(vtkIndent indent);
  
private:
  vtkXMLPRectilinearGridWriter(const vtkXMLPRectilinearGridWriter&);  // Not implemented.
  void operator=(const vtkXMLPRectilinearGridWriter&);  // Not implemented.
};

#endif
