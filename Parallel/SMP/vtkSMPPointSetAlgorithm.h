/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPPointSetAlgorithm.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPPointSetAlgorithm - base class to derive from to create
// multi-threaded vtkPointSet producing algorithms
// .SECTION Description
// vtkSMPPointSetAlgorithm is a convenience class for algorithms writers who
// want to produce vtkStructuredGrid, vtkPolyData or vtkUnstructuredGrid
// data sets and use multi-threaded techniques in the data processing pass.
// .SECTION See Also
// vtkPointSetAlgorithm

#ifndef __vtkSMPPointSetAlgorithm_h
#define __vtkSMPPointSetAlgorithm_h

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkSMPAlgorithm.h"

class vtkPointSet;
class vtkPolyData;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

class VTKPARALLELSMP_EXPORT vtkSMPPointSetAlgorithm : public vtkSMPAlgorithm
{
public:
  static vtkSMPPointSetAlgorithm *New();
  vtkTypeMacro(vtkSMPPointSetAlgorithm,vtkSMPAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output data object for a port on this algorithm.
  vtkPointSet* GetOutput();
  vtkPointSet* GetOutput(int);

  // Description:
  // Get the output as vtkPolyData.
  vtkPolyData *GetPolyDataOutput();

  // Description:
  // Get the output as vtkStructuredGrid.
  vtkStructuredGrid *GetStructuredGridOutput();

  // Description:
  // Get the output as vtkUnstructuredGrid.
  vtkUnstructuredGrid *GetUnstructuredGridOutput();

  // Description:
  // Assign a data object as input. Note that this method does not
  // establish a pipeline connection. Use SetInputConnection() to
  // setup a pipeline connection.
  void SetInputData(vtkDataObject*);
  void SetInputData(int, vtkDataObject*);
  void SetInputData(vtkPointSet*);
  void SetInputData(int, vtkPointSet*);

  // Description:
  // Assign a data object as input. Note that this method does not
  // establish a pipeline connection. Use AddInputConnection() to
  // setup a pipeline connection.
  void AddInputData(vtkDataObject *);
  void AddInputData(vtkPointSet*);
  void AddInputData(int, vtkPointSet*);
  void AddInputData(int, vtkDataObject*);

  // this method is not recommended for use, but lots of old style filters
  // use it
  vtkDataObject *GetInput();

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector);

protected:
  vtkSMPPointSetAlgorithm();
  ~vtkSMPPointSetAlgorithm() {};

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestDataObject(vtkInformation* request,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int ExecuteInformation(vtkInformation*,
                                 vtkInformationVector**,
                                 vtkInformationVector*) {return 1;};

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*) {return 1;};

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int ComputeInputUpdateExtent(vtkInformation*,
                                       vtkInformationVector**,
                                       vtkInformationVector*)
    {
      return 1;
    };

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkSMPPointSetAlgorithm(const vtkSMPPointSetAlgorithm&);  // Not implemented.
  void operator=(const vtkSMPPointSetAlgorithm&);  // Not implemented.
};

#endif
