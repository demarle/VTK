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
    {
    (*o)( f + k, tid );
    }
  }

void func_call_init ( int32_t b, int32_t e, int32_t tid, const vtkFunctorInitialisable* o, vtkIdType f )
  {
  if ( o->ShouldInitialize(tid) )
    {
    o->Init( tid );
    }
  for ( int32_t k = b; k < e; ++k )
    {
    (*o)( f + k, tid );
    }
  }

void my_parallel ( const vtkSMPCommand* function, const vtkObject* data, vtkSMPThreadID tid )
  {
  function->Execute( tid, data );
  }

const int GRAIN = 1024;

//--------------------------------------------------------------------------------
namespace vtkSMP
{

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
    {
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, GRAIN, GRAIN);
    kaapic_foreach( 0, last - first, &attr, 2, func_call, op, first );
    kaapic_foreach_attr_destroy(&attr);
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op )
    {
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, GRAIN, GRAIN);
    kaapic_foreach( 0, last - first, &attr, 2, func_call_init, op, first );
    kaapic_foreach_attr_destroy(&attr);
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return kaapic_get_concurrency();
    }

  void Parallel( const vtkSMPCommand* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    for ( vtkSMPThreadID tid = 1; tid < kaapic_get_concurrency(); ++tid )
      {
      kaapic_spawn_attr_set_kproc(&attr, tid);
      kaapic_spawn( &attr, 3, my_parallel,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, data, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_V, tid, 1, KAAPIC_TYPE_INT );
      }
    if ( !skipThreads )
      {
      kaapic_spawn_attr_set_kproc(&attr, 0);
      kaapic_spawn( &attr, 3, my_parallel,
                    KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_R, data, 1, KAAPIC_TYPE_PTR,
                    KAAPIC_MODE_V, 0, 1, KAAPIC_TYPE_INT );
      }
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    kaapic_spawn_attr_destroy(&attr);
    }

}
