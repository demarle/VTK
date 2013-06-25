#include "vtkParallelOperators.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkParallelOperators);

vtkParallelOperators::vtkParallelOperators()
  {
  }

vtkParallelOperators::~vtkParallelOperators()
  {
  }

void vtkParallelOperators::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  } 
