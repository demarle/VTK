#include "vtkSMP.h"
#include "vtkCommand.h"
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

void my_parallel ( int32_t b, int32_t e, int32_t tid, const vtkFunctor* f, const vtkSMPCommand* callback )
{
  for ( int32_t k = b; k < e; ++k )
  {
    callback->Execute( f, vtkCommand::UserEvent + 42, &k );
  }
}

void my_init ( int32_t b, int32_t e, int32_t tid, const vtkFunctorInitialisable* f )
{
  for ( int32_t k = b; k < e; ++k )
  {
    f->Init( k );
  }
}

//--------------------------------------------------------------------------------
namespace vtkSMP
{

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
    {
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, 32, 32);
    kaapic_foreach( 0, last - first, &attr, 2, func_call, op, first );
    kaapic_foreach_attr_destroy(&attr);
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op )
    {
    if (op->CheckAndSetInitialized())
      {
      kaapic_foreach_attr_t att;
      kaapic_foreach_attr_init(&att);
      kaapic_foreach_attr_set_grains(&att, 1, 1);
      kaapic_foreach( 0, kaapic_get_concurrency(), &att, 1, my_init, op );
      kaapic_foreach_attr_destroy(&att);
      }
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, 32, 32);
    kaapic_foreach( 0, last - first, &attr, 2, func_call, op, first );
    kaapic_foreach_attr_destroy(&attr);
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return kaapic_get_concurrency();
    }

  void Parallel( const vtkFunctor* f, const vtkSMPCommand* callback , vtkSMPThreadID skipThreads )
    {
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, 1, 1);
    kaapic_foreach( skipThreads, kaapic_get_concurrency(), &attr, 2, my_parallel, f, callback );
    kaapic_foreach_attr_destroy(&attr);
    }

}
