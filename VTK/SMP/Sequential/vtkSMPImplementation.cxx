#include "vtkSMPImplementation.h"

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  vtkIdType i = first;
  while ( i < last )
  {
    (*op)(i);
    ++i;
  }
}
