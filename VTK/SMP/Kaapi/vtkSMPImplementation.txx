#include <kaapic.h>
#include "vtkMutexLock.h"

template <class... T>
struct TaskParallel : public ka::Task<(sizeof...(T)+1)>::template Signature<const vtkTask<T...>*, typename TaskHelper<T>::type...> {};

template <class T>
T incr (T data) { return ++data; }

template<> template <class... T>
struct TaskBodyCPU<TaskParallel<T...> > {
  void operator() ( const vtkTask<T...>* function, typename TaskHelper<T>::type... data )
    {
    function->Execute(data...);
    }
};

template <class... T>
void ParallelDo( const vtkTask<T...>* function, vtkIdType numThread, vtkIdType lastThread, typename IteratorHelper<T>::type... data )
  {
  if (!numThread)
    kaapic_begin_parallel(KAAPIC_FLAG_DEFAULT);
  if (numThread < lastThread)
    {
    ka::Spawn<TaskParallel<T...> >()(function, *data...);
    ParallelDo( function, numThread + 1, lastThread, incr(data)... );
    }
  else
    kaapic_end_parallel(KAAPIC_FLAG_DEFAULT);
  }

template <class... T>
void ParallelSkip( const vtkTask<T...>* function, vtkIdType numThread, vtkIdType skipThreads, typename IteratorHelper<T>::type... data )
  {
  if (skipThreads > numThread)
    ParallelSkip( function, numThread + 1, skipThreads, incr(data)... );
  else
    ParallelDo( function, 0, kaapi_getconcurrency() - skipThreads, data... );
  }
