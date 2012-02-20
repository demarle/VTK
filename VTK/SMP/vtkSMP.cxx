#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"

//--------------------------------------------------------------------------------
vtkFunctor::vtkFunctor() { }

vtkFunctor::~vtkFunctor() { }

void vtkFunctor::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  }

//--------------------------------------------------------------------------------
vtkFunctorInitialisable::vtkFunctorInitialisable() : vtkFunctor()
  {
  IsInitialized = false;
  }

vtkFunctorInitialisable::~vtkFunctorInitialisable() { }

bool vtkFunctorInitialisable::CheckAndSetInitialized() const
  {
  bool ret = IsInitialized;
  IsInitialized = true;
  return ret;
  }

void vtkFunctorInitialisable::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Is initialized: " << IsInitialized << endl;
  }

//--------------------------------------------------------------------------------
vtkSMPCommand::vtkSMPCommand() { }

vtkSMPCommand::~vtkSMPCommand() { }

void vtkSMPCommand::Execute(vtkObject *caller, unsigned long eventId, void *callData) { }

void vtkSMPCommand::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }
