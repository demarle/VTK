#ifndef __vtkFunctor_h__
#define __vtkFunctor_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"

class VTKPARALLELSMP_EXPORT vtkFunctor : public vtkObject
{
public:
  vtkTypeMacro(vtkFunctor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void operator () ( vtkIdType ) const = 0;

protected:
  vtkFunctor();
  ~vtkFunctor();

private:
  vtkFunctor(const vtkFunctor&);  // Not implemented.
  void operator=(const vtkFunctor&);  // Not implemented.

};

#endif
