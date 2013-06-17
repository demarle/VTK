#include "vtkFunctorInitializable.h"
#include "vtkSMP.h"

//======================================================================================
vtkFunctorInitializable::vtkFunctorInitializable() :
    vtkFunctor(), IsInitialized(vtkSMP::InternalGetNumberOfThreads(), 0)
  {
  }

//--------------------------------------------------------------------------------
vtkFunctorInitializable::~vtkFunctorInitializable()
  {
  IsInitialized.clear();
  }

//--------------------------------------------------------------------------------
bool vtkFunctorInitializable::ShouldInitialize( ) const
  {
  return !IsInitialized[vtkSMP::InternalGetTid()];
  }

//--------------------------------------------------------------------------------
void vtkFunctorInitializable::Initialized( ) const
  {
  IsInitialized[vtkSMP::InternalGetTid()] = 1;
  }

//--------------------------------------------------------------------------------
void vtkFunctorInitializable::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Is initialized: " << endl;
  for ( vtkstd::vector<vtkIdType>::size_type i = 0; i < IsInitialized.size(); ++i )
    {
    os << indent.GetNextIndent() << "Id " << i << ": ";
    if ( IsInitialized[i] )
      os << "true";
    else
      os << "false";
    os << endl;
    }
  }
