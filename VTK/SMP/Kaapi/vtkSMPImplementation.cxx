#include "vtkSMP.h"
#include "vtkCommand.h"
#include <kaapic.h>
#include <cmath>

void smpInit(void)
  {
  kaapic_init( KAAPIC_START_ONLY_MAIN );
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

void my_parallel ( const vtkTask* function, const vtkObject* data, vtkSMPThreadID tid )
  {
  function->Execute( tid, data );
  }

void my_spawn ( const vtkTask* function )
  {
  function->Execute( kaapic_get_thread_num(), 0 );
  }

//--------------------------------------------------------------------------------
namespace vtkSMP
{

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain )
    {
    vtkIdType n = last - first;
    if (!n) return;
    uint32_t granularity = grain ? grain : sqrt(n);
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, granularity, granularity);
    kaapic_foreach( 0, n, &attr, 2, func_call, op, first );
    kaapic_foreach_attr_destroy(&attr);
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op, int grain )
    {
    vtkIdType n = last - first;
    if (!n) return;
    uint32_t granularity = grain ? grain : sqrt(n);
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, granularity, granularity);
    kaapic_foreach( 0, n, &attr, 2, func_call_init, op, first );
    kaapic_foreach_attr_destroy(&attr);
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return kaapic_get_concurrency();
    }

  void Parallel( const vtkTask* function, const vtkObject* data, vtkSMPThreadID skipThreads )
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

  void BeginSpawnRegion()
    {
    kaapic_begin_parallel( KAAPIC_FLAG_DEFAULT );
    }

  void Spawn( const vtkTask *function )
    {
    kaapic_spawn_attr_t attr;
    kaapic_spawn_attr_init(&attr);
    kaapic_spawn( &attr, 1, my_spawn, KAAPIC_MODE_R, function, 1, KAAPIC_TYPE_PTR );
    kaapic_spawn_attr_destroy(&attr);
    }

  void Sync( )
    {
    kaapic_end_parallel( KAAPIC_FLAG_DEFAULT );
    }
}
