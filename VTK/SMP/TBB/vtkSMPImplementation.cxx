#include "vtkSMP.h"
#include <task_scheduler_init.h>
#include <parallel_for.h>
#include <blocked_range.h>

class FuncCall
{
  const vtkFunctor* o;

public:
  void operator() ( const tbb::blocked_range<vtkIdType>& r ) const
    {
    vtkSMPThreadID tid = 0;
    for ( vtkIdType k = r.begin(); k < r.end(); ++k )
      {
      (*o)( k, tid );
      }
    }

  FuncCall ( const vtkFunctor* _o ) : o(_o) { }
  ~FuncCall () { }
};

class FuncCallInit
{
  const vtkFunctorInitialisable* o;

public:
  void operator() ( const tbb::blocked_range<vtkIdType>& r ) const
    {
    vtkSMPThreadID tid = 0;
    if ( o->ShouldInitialize(tid) )
      {
      o->Init( tid );
      }
    for ( vtkIdType k = r.begin(); k < r.end(); ++k )
      {
      (*o)( k, tid );
      }
    }

  FuncCallInit ( const vtkFunctorInitialisable* _o ) : o(_o) { }
  ~FuncCallInit () { }
};

class ParallelCall
{
  const vtkSMPCommand* function;
  const vtkObject* data;

public:
  void operator() ( tbb::blocked_range<vtkSMPThreadID>& r ) const
    {
    for ( vtkSMPThreadID tid = r.begin(); tid < r.end(); ++tid )
      {
      function->Execute( tid, data );
      }
    }

  ParallelCall ( const vtkSMPCommand* _f, const vtkObject* _o ) : function(_f), data(_o) { }
  ~ParallelCall ( ) { }
};

const int GRAIN = 1024;

//--------------------------------------------------------------------------------
namespace vtkSMP
{

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
    {
    tbb::task_scheduler_init init;
    tbb::parallel_for( tbb::blocked_range<vtkIdType>( first, last, GRAIN ), FuncCall( op ) );
    }

  void ForEach ( vtkIdType first, vtkIdType last, const vtkFunctorInitialisable* op )
    {
    tbb::task_scheduler_init init;
    tbb::parallel_for( tbb::blocked_range<vtkIdType>( first, last, GRAIN ), FuncCallInit( op ) );
    }

  vtkSMPThreadID GetNumberOfThreads()
    {
    return tbb::task_scheduler_init::default_num_threads();
    }

  void Parallel( const vtkSMPCommand* function, const vtkObject* data, vtkSMPThreadID skipThreads )
    {
    tbb::task_scheduler_init init;
    tbb::parallel_for( tbb::blocked_range<vtkSMPThreadID>( skipThreads, init.default_num_threads(), 1 ), ParallelCall( function, data ) );
    }

}
