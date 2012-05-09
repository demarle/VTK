#include "vtkSMP.h"
#include "vtkCommand.h"
#include <cilk.h>

extern "Cilk++" void func_call ( vtkIdType begin, vtkIdType end, const vtkFunctor* o )
  {
  cilk_for ( vtkIdType k = begin; k < end; ++k )
    {
    (*o)( k, cilk::current_worker_id() );
    }
  }

extern "Cilk++" void func_call_init ( vtkIdType begin, vtkIdType end, const vtkFunctorInitialisable* o )
  {
  cilk_for ( vtkIdType k = begin; k < end; ++k )
    {
    vtkSMPThreadID tid = cilk::current_worker_id();
    if ( o->ShouldInitialize(tid) )
      o->Init( tid );
    (*o)( k, tid );
    }
  }

//--------------------------------------------------------------------------------
namespace vtkSMP
{

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
    {
    cilk::run(&func_call, first, last, op);
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op )
    {
    cilk::run(&func_call_init, first, last, op);
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    cilk::context ctx;
    return ctx.get_worker_count();
    }

  void Parallel( const vtkTask* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    }

}
