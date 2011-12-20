#include "vtkSMPImplementation.h"

void InternalForEach(vtkIdType first, vtkIdType last, const vtkFunctor* op)
{
  #pragma omp default(none)
  #pragma omp parallel for
  for ( vtkIdType i = first ; i < last ; ++i )
    (*op)(i);
}
