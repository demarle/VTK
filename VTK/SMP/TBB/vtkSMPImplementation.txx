#include <tbb/tbb.h>
#include <functional>
#include "vtkMutexLock.h"

static vtkMutexLock* globLock = vtkMutexLock::New();

template <class T>
T incr (T data) { return ++data; }

template <class T>
typename TaskHelper<T>::type getPtr (typename IteratorHelper<T>::type it) { return *it; }

template <class... T> void PrintVaArgs(T... t) {}
template <class T, class... R> void PrintVaArgs(T t, R... r) { cout << " " << t; PrintVaArgs(r...); }
template <class T> void PrintVaArgs(T t) { cout << " " << t; }

template <class... T>
class TaskParallel : public tbb::task {
    std::function< void() > _task;
  public:
    TaskParallel (const vtkTask<T...>* t, typename TaskHelper<T>::type... d) :
          _task( [&](){t->Execute(std::forward(d)...);} )
      {
      globLock->Lock();
      cout << "Execute:";
      PrintVaArgs(d...);
      cout << endl;
      globLock->Unlock();
      }

    tbb::task* execute()
      {
      _task();
      return NULL;
      }
};

template <class... T>
inline void ParallelDoImpl( tbb::task_list* list, const vtkTask<T...>* function, vtkIdType numThread, vtkIdType lastThread, typename IteratorHelper<T>::type... data )
  {
  if (numThread < lastThread)
    {
    list->push_back(*new(tbb::task::allocate_root()) TaskParallel<T...>(function, getPtr<T>(data)...));
    ParallelDoImpl( list, function, numThread + 1, lastThread, incr(data)... );
    }
  }

template <class... T>
void ParallelDo( const vtkTask<T...>* function, vtkIdType numThread, vtkIdType lastThread, typename IteratorHelper<T>::type... data )
  {
  if (numThread < lastThread)
    {
    tbb::task_list list;
    ParallelDoImpl( &list, function, numThread, lastThread, data... );
    tbb::task::spawn_root_and_wait(list);
    }
  }

template <class... T>
void ParallelSkip( const vtkTask<T...>* function, vtkIdType numThread, vtkIdType skipThreads, typename IteratorHelper<T>::type... data )
  {
  if (skipThreads > numThread)
    ParallelSkip( function, numThread + 1, skipThreads, incr(data)... );
  else
    ParallelDo( function, 0, tbb::task_scheduler_init::default_num_threads() - skipThreads, data... );
  }
