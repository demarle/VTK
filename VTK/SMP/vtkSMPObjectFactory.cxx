#include "vtkSMPObjectFactory.h"
#include "vtkVersion.h"

#include "vtkSMPWarpVector.h"
//#include "vtkSMPTransformFilter.h"
#include "vtkSMPTransform.h"

vtkStandardNewMacro(vtkSMPObjectFactory);

VTK_FACTORY_INTERFACE_IMPLEMENT(vtkSMPObjectFactory);

void vtkSMPObjectFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

const char* vtkSMPObjectFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}

const char* vtkSMPObjectFactory::GetDescription()
{
  return "Factory for smp-enabled algorithms";
}


VTK_CREATE_CREATE_FUNCTION(vtkSMPWarpVector);
//VTK_CREATE_CREATE_FUNCTION(vtkSMPTransformFilter);
VTK_CREATE_CREATE_FUNCTION(vtkSMPTransform);

vtkSMPObjectFactory::vtkSMPObjectFactory()
{
  this->RegisterOverride("vtkWarpVector",
                         "vtkSMPWarpVector",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPWarpVector);
/*  this->RegisterOverride("vtkTransformFilter",
                         "vtkSMPTransformFilter",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPTransformFilter);*/
  this->RegisterOverride("vtkTransform",
                         "vtkSMPTransform",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPTransform);
}

vtkSMPObjectFactory::~vtkSMPObjectFactory()
{
}
