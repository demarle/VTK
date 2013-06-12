#ifndef __vtkSMPClipDataSet_h
#define __vtkSMPClipDataSet_h

#include "vtkClipDataSet.h"

class VTK_SMP_EXPORT vtkSMPClipDataSet : public vtkClipDataSet
{
public:
  vtkTypeMacro(vtkSMPClipDataSet,vtkClipDataSet);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkSMPClipDataSet *New();

protected:
  vtkSMPClipDataSet();
  ~vtkSMPClipDataSet();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkSMPClipDataSet(const vtkSMPClipDataSet&);  // Not implemented.
  void operator=(const vtkSMPClipDataSet&);  // Not implemented.
};

#endif
