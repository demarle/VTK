/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAbstractParticleWriter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAbstractParticleWriter - abstract class to write particle data to file
// .SECTION Description
// vtkAbstractParticleWriter is an abstract class which is used by
// vtkTemporalStreamTracer to write particles out during simulations.
// This calls is abstract and provides a TimeStep and FileName.
// Subclasses of this should provide the necessary IO.

// .SECTION Caveats
// See vtkWriter

// .SECTION See Also
// vtkTemporalStreamTracer

#ifndef __vtkAbstractParticleWriter_h
#define __vtkAbstractParticleWriter_h

#include "vtkWriter.h"

class VTK_IO_EXPORT vtkAbstractParticleWriter : public vtkWriter
{
public:
  vtkTypeRevisionMacro(vtkAbstractParticleWriter,vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the TimeStep that is being written
  vtkSetMacro(TimeStep, int);
  vtkGetMacro(TimeStep, int);

  // Description:
  // Set/get the FileName that is being written to
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
   vtkAbstractParticleWriter();
  ~vtkAbstractParticleWriter();

  virtual void WriteData() = 0; //internal method subclasses must respond to
  int          TimeStep;
  char        *FileName;

private:
  vtkAbstractParticleWriter(const vtkAbstractParticleWriter&);  // Not implemented.
  void operator=(const vtkAbstractParticleWriter&);  // Not implemented.
};

#endif
