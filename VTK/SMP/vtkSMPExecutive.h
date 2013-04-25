#ifndef VTKSMPEXECUTIVE_H
#define VTKSMPEXECUTIVE_H

#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObject.h"

class VTK_SMP_EXPORT vtkSMPExecutive : public vtkStreamingDemandDrivenPipeline
{
  vtkSMPExecutive( const vtkSMPExecutive& );
  void operator =( const vtkSMPExecutive& );

protected:
  vtkSMPExecutive();
  ~vtkSMPExecutive();

public:
  vtkTypeMacro(vtkSMPExecutive, vtkStreamingDemandDrivenPipeline);
  static vtkSMPExecutive* New();

  void PrintSelf(ostream &os, vtkIndent indent);

  virtual int ForwardUpstream(vtkInformation* request);
};

#endif // VTKSMPEXECUTIVE_H
