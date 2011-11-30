#ifndef __vtkSMPWarpVector_h__
#define __vtkSMPWarpVector_h__

#include "vtkWarpVector.h"

class vtkInformation;
class vtkInformationVector;

class VTK_SMP_EXPORT vtkSMPWarpVector : public vtkWarpVector
{
public :
  vtkTypeMacro(vtkSMPWarpVector,vtkWarpVector);
  static vtkSMPWarpVector *New();
  void PrintSelf(ostream& os, vtkIndent indent);

protected :
  vtkSMPWarpVector();
  ~vtkSMPWarpVector();

  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

private :
  vtkSMPWarpVector(const vtkSMPWarpVector&);  // Not implemented.
  void operator=(const vtkSMPWarpVector&);  // Not implemented.

};

#endif //__vtkSMPWarpVector_h__
