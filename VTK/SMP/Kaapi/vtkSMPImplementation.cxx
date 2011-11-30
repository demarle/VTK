#include "vtkSMPImplementation.h"

void smpInit(void)
{
  kaapic_init(1);
  kaapic_set_grains(32,32);
}
void smpFini(void)
{
  kaapic_finalize();
}

void functor( int32_t* b, int32_t* e, int32_t* tid, const vtkFunctor* o, vtkIdType f )
{
  for ( int32_t k = *b; k <= *e; ++k )
  {
    (*o)( f + k );
  }
}

void InternalForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
{
  kaapic_foreach( functor, 0, last - first - 1, 2, op, first );
}

