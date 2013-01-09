#ifndef VTKSMPZCURVE_H
#define VTKSMPZCURVE_H

#include "vtkPointSetAlgorithm.h"

class VTK_SMP_EXPORT vtkSMPZCurve : public vtkPointSetAlgorithm
{
public:
  static vtkSMPZCurve *New();
  vtkTypeMacro(vtkSMPZCurve,vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSMPZCurve();
  ~vtkSMPZCurve();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
private:
  vtkSMPZCurve(const vtkSMPZCurve&);  // Not implemented.
  void operator=(const vtkSMPZCurve&);  // Not implemented.
};

#endif // VTKSMPZCURVE_H
