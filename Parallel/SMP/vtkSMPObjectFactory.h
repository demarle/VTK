#ifndef __vtkSMPObjectFactory_h__
#define __vtkSMPObjectFactory_h__

#include "vtkObjectFactory.h"

class VTK_SMP_EXPORT vtkSMPObjectFactory : public vtkObjectFactory
{
public:
  vtkTypeMacro(vtkSMPObjectFactory,vtkObjectFactory);
  static vtkSMPObjectFactory *New();
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetVTKSourceVersion();

  virtual const char* GetDescription();

protected:
  vtkSMPObjectFactory();
  ~vtkSMPObjectFactory();

private:
  vtkSMPObjectFactory(const vtkSMPObjectFactory&);  // Not implemented.
  void operator=(const vtkSMPObjectFactory&);  // Not implemented.
};

#endif //__vtkSMPObjectFactory_h__
