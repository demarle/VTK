/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPPolyDataAlgorithm.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPPolyDataAlgorithm - base class to derive from to create
// multi-threaded vtkPolyData producing algorithms.
// producing filters
// .SECTION Description
// vtkSMPPolyDataAlgorithm is a convenience class for algorithms writers who
// want to produce vtkPolyData and use multi-threaded techniques in the data
// processing pass.
// .SECTION See Also
// vtkPolyDataAlgorithm

#ifndef __vtkSMPPolyDataAlgorithm_h
#define __vtkSMPPolyDataAlgorithm_h

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkSMPAlgorithm.h"
#include "vtkPolyData.h" // makes things a bit easier

class vtkDataSet;
class vtkPolyData;

class VTKPARALLELSMP_EXPORT vtkSMPPolyDataAlgorithm : public vtkSMPAlgorithm
{
public:
  static vtkSMPPolyDataAlgorithm *New();
  vtkTypeMacro(vtkSMPPolyDataAlgorithm,vtkSMPAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output data object for a port on this algorithm.
  vtkPolyData* GetOutput();
  vtkPolyData* GetOutput(int);
  virtual void SetOutput(vtkDataObject* d);

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

  // This method is not recommended for use, but lots of old style filters
  // use it
  vtkDataObject* GetInput();
  vtkDataObject *GetInput(int port);
  vtkPolyData *GetPolyDataInput(int port);

  // Description:
  // Assign a data object as input. Note that this method does not
  // establish a pipeline connection. Use SetInputConnection() to
  // setup a pipeline connection.
  void SetInputData(vtkDataObject *);
  void SetInputData(int, vtkDataObject*);

  // Description:
  // Assign a data object as input. Note that this method does not
  // establish a pipeline connection. Use AddInputConnection() to
  // setup a pipeline connection.
  void AddInputData(vtkDataObject *);
  void AddInputData(int, vtkDataObject*);

protected:
  vtkSMPPolyDataAlgorithm();
  ~vtkSMPPolyDataAlgorithm();

  // Description:
  // Overridden to ...?
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  // Description:
  // Overridden to ...?
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkSMPPolyDataAlgorithm(const vtkSMPPolyDataAlgorithm&);  // Not implemented.
  void operator=(const vtkSMPPolyDataAlgorithm&);  // Not implemented.
};

#endif
