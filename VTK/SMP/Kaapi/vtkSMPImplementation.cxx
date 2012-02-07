#include "vtkSMP.h"
#include <kaapic.h>

void smpInit(void)
{
  kaapic_init( 1 );
}
void smpFini(void)
{
  kaapic_finalize();
}

void func_call ( int32_t b, int32_t e, int32_t tid, const vtkFunctor* o, vtkIdType f )
{
  for ( int32_t k = b; k < e; ++k )
    (*o)( f + k, tid );
}

void InternalForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
{
  kaapic_begin_parallel();
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 32, 32);
  kaapic_foreach( 0, last - first, &attr, 2, func_call, op, first );
  kaapic_end_parallel( 0 );
  kaapic_foreach_attr_destroy(&attr);
}

vtkSMPThreadID InternalGetNumberOfThreads()
{
  return kaapic_get_concurrency();
}

void my_parallel ( int32_t b, int32_t e, int32_t tid, const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) )
{
  for ( int32_t k = b; k < e; ++k )
  {
    m( f, k );
  }
}

void InternalParallel( const vtkFunctor* f, void (*m)(const vtkFunctor*, vtkSMPThreadID) , vtkSMPThreadID skipThreads )
{
  kaapic_begin_parallel();
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 1, 1);
  kaapic_foreach( skipThreads, kaapic_get_concurrency(), &attr, 2, my_parallel, f, m );
  kaapic_end_parallel( 0 );
  kaapic_foreach_attr_destroy(&attr);
}
