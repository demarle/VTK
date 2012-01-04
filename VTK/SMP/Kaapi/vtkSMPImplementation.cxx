#include "vtkSMP.h"
#include <kaapic.h>

void smpInit(void)
{
  kaapic_init( 1 );
}
void smpFini(void)
{
  kaapic_finalize();
  cout << "fini" << endl;
}

void func_call ( int32_t b, int32_t e, int32_t tid, const vtkFunctor* o, vtkIdType f )
{
  for ( int32_t k = b; k < e; ++k )
    (*o)( f + k, tid );
}

void InternalForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
{
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 32, 32);
  kaapic_foreach( 0, last - first, &attr, 2, func_call, op, first );
  kaapic_foreach_attr_destroy(&attr);
}

void my_init ( int32_t b, int32_t e, int32_t tid, const vtkFunctorInitialisable* f )
{
  for ( int32_t k = b; k < e; ++k )
    f->init( k );
}

void InternalInit( const vtkFunctorInitialisable* f )
{
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 1, 1);
  kaapic_foreach( 0, kaapic_get_concurrency(), &attr, 1, my_init, f );
  kaapic_foreach_attr_destroy(&attr);
}

void InternalGetThreadsIDs(vtkstd::vector<vtkSMPThreadID>& result)
{
  vtkSMPThreadID numThreads = kaapic_get_concurrency();

  for (vtkSMPThreadID i = 0; i < numThreads; ++i)
    result.push_back(i);
}

void my_merge ( int32_t b, int32_t e, int32_t tid, const vtkMergeable* f )
{
  for ( int32_t k = b; k < e; ++k )
    f->merge( k );
}

void InternalMerge(const vtkMergeable *f)
{
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 1, 1);
  kaapic_foreach( 0, kaapic_get_concurrency(), &attr, 1, my_merge, f );
  kaapic_foreach_attr_destroy(&attr);
}


void my_pre_merge ( int32_t b, int32_t e, int32_t tid, const vtkMergeableInitialisable* f )
{
  for ( int32_t k = b; k < e; ++k )
    f->pre_merge( k );
}

void InternalPreMerge(const vtkMergeableInitialisable *f)
{
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 1, 1);
  kaapic_foreach( 0, kaapic_get_concurrency(), &attr, 1, my_pre_merge, f );
  kaapic_foreach_attr_destroy(&attr);
}
