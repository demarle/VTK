#include "vtkSMPObjectFactory.h"
#include "vtkVersion.h"

#include "vtkSMPWarpVector.h"
#include "vtkSMPContourFilter.h"
#include "vtkSMPTransform.h"
#include "vtkSMPMinMaxTree.h"

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
VTK_CREATE_CREATE_FUNCTION(vtkSMPContourFilter);
VTK_CREATE_CREATE_FUNCTION(vtkSMPTransform);
VTK_CREATE_CREATE_FUNCTION(vtkSMPMinMaxTree);

vtkSMPObjectFactory::vtkSMPObjectFactory()
{
  this->RegisterOverride("vtkWarpVector",
                         "vtkSMPWarpVector",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPWarpVector);
  this->RegisterOverride("vtkContourFilter",
                         "vtkSMPContourFilter",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPContourFilter);
  this->RegisterOverride("vtkTransform",
                         "vtkSMPTransform",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPTransform);
  this->RegisterOverride("vtkSimpleScalarTree",
                         "vtkSMPMinMaxTree",
                         "SMP",
                         1,
                         vtkObjectFactoryCreatevtkSMPMinMaxTree);
}

vtkSMPObjectFactory::~vtkSMPObjectFactory()
{
}
