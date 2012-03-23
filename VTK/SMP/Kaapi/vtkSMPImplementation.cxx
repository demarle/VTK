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

void my_parallel ( int32_t b, int32_t e, int32_t tid, const vtkObject* data, const vtkSMPCommand* function )
  {
  for ( int32_t k = b; k < e; ++k )
    {
    function->Execute( k, data );
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
    kaapic_foreach_attr_t attr;
    kaapic_foreach_attr_init(&attr);
    kaapic_foreach_attr_set_grains(&attr, 1, 1);
    kaapic_foreach( skipThreads, kaapic_get_concurrency(), &attr, 2, my_parallel, data, function );
    kaapic_foreach_attr_destroy(&attr);
    }

}
