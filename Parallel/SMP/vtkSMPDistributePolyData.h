#ifndef VTKSMPDISTRIBUTEPOLYDATA_H
#define VTKSMPDISTRIBUTEPOLYDATA_H

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKPARALLELSMP_EXPORT vtkSMPDistributePolyData : public vtkPolyDataAlgorithm
{
public:
  static vtkSMPDistributePolyData *New();
  vtkTypeMacro(vtkSMPDistributePolyData,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSMPDistributePolyData();
  ~vtkSMPDistributePolyData();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkSMPDistributePolyData(const vtkSMPDistributePolyData&);  // Not implemented.
  void operator=(const vtkSMPDistributePolyData&);  // Not implemented.
};

#endif // VTKSMPDISTRIBUTEPOLYDATA_H
