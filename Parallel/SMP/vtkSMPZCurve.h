/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPZCurve.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPZCurve - !!!!
// .SECTION Description
// !!!!

#ifndef VTKSMPZCURVE_H
#define VTKSMPZCURVE_H

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkPointSetAlgorithm.h"

class VTKPARALLELSMP_EXPORT vtkSMPZCurve : public vtkPointSetAlgorithm
{
public:
  static vtkSMPZCurve *New();
  vtkTypeMacro(vtkSMPZCurve,vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSMPZCurve();
  ~vtkSMPZCurve();

  // Description:
  // ?
  int RequestData(vtkInformation *,
                  vtkInformationVector **, vtkInformationVector *);

private:
  vtkSMPZCurve(const vtkSMPZCurve&);  // Not implemented.
  void operator=(const vtkSMPZCurve&);  // Not implemented.
};

#endif // VTKSMPZCURVE_H
