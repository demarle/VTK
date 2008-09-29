#include "vtkConditionVariable.h"
#include "vtkMultiThreader.h"

#include <stdlib.h>

// For vtkSleep
#include "vtkWindows.h"
#include <ctype.h>
#include <time.h>

// Cross platform sleep (horked from vtkGeoTerrain.cxx)
inline void vtkSleep( double duration )
{
  duration = duration; // avoid warnings
  // sleep according to OS preference
#ifdef _WIN32
  Sleep( (int)( 1000 * duration ) );
#elif defined(__FreeBSD__) || defined(__linux__) || defined(sgi)
  struct timespec sleep_time, dummy;
  sleep_time.tv_sec = static_cast<int>( duration );
  sleep_time.tv_nsec = static_cast<int>( 1000000000 * ( duration - sleep_time.tv_sec ) );
  nanosleep( &sleep_time, &dummy );
#endif
}

typedef struct {
  vtkMutexLock* Lock;
  vtkConditionVariable* Condition;
  int Done;
  int NumberOfWorkers;
} vtkThreadUserData;

static void vtkDoNothingButEliminateCompilerWarnings()
{
}

VTK_THREAD_RETURN_TYPE vtkTestCondVarThread( void* arg )
{
  int threadId = static_cast<vtkMultiThreader::ThreadInfo*>(arg)->ThreadID;
  int threadCount = static_cast<vtkMultiThreader::ThreadInfo*>(arg)->NumberOfThreads;
  vtkThreadUserData* td = static_cast<vtkThreadUserData*>(
    static_cast<vtkMultiThreader::ThreadInfo*>(arg)->UserData );
  if ( td )
    {
    if ( threadId == 0 )
      {
      td->Done = 0;
      td->Lock->Lock();
      cout << "Thread " << ( threadId + 1 ) << " of " << threadCount << " initializing.\n";
      cout.flush();
      td->Lock->Unlock();

      for ( int i = 0; i < 2 * threadCount; ++ i )
        {
        td->Lock->Lock();
        cout << "Signaling (count " << i << ")...";
        cout.flush();
        td->Lock->Unlock();
        td->Condition->Signal();

        //sleep( 1 );
        }

      int i = 0;
      do
        {
        td->Lock->Lock();
        td->Done = 1;
        cout << "Broadcasting...\n";
        cout.flush();
        td->Lock->Unlock();
        td->Condition->Broadcast();
        vtkSleep( 0.2 ); // 0.2 s between broadcasts
        }
      while ( td->NumberOfWorkers > 0 && ( i ++ < 1000 ) );
      if ( i >= 1000 )
        {
        exit( 2 );
        }
      }
    else
      {
      // Wait for thread 0 to initialize... Ugly but effective
      while ( td->Done < 0 )
        vtkDoNothingButEliminateCompilerWarnings();

      // Wait for the condition and then note we were signaled.
      while ( td->Done <= 0 )
        {
        td->Lock->Lock();
        cout << " Thread " << ( threadId + 1 ) << " waiting.\n";
        cout.flush();
        td->Condition->Wait( td->Lock );
        cout << " Thread " << ( threadId + 1 ) << " responded.\n";
        cout.flush();
        td->Lock->Unlock();
        }
      -- td->NumberOfWorkers;
      }

    td->Lock->Lock();
    cout << "  Thread " << ( threadId + 1 ) << " of " << threadCount << " exiting.\n";
    cout.flush();
    td->Lock->Unlock();
    }
  else
    {
    cout << "No thread data!\n";
    cout << "  Thread " << ( threadId + 1 ) << " of " << threadCount << " exiting.\n";
    -- td->NumberOfWorkers;
    cout.flush();
    }

  return VTK_THREAD_RETURN_VALUE;
}

int TestConditionVariable( int, char*[] )
{
  vtkMultiThreader* threader = vtkMultiThreader::New();
  int numThreads = threader->GetNumberOfThreads();

  vtkThreadUserData data;
  data.Lock = vtkMutexLock::New();
  data.Condition = vtkConditionVariable::New();
  data.Done = -1;
  data.NumberOfWorkers = numThreads - 1;

  threader->SetNumberOfThreads( numThreads );
  threader->SetSingleMethod( vtkTestCondVarThread, &data );
  threader->SingleMethodExecute();

  cout << "Done with threader.\n";
  cout.flush();

  vtkIndent indent;
  indent = indent.GetNextIndent();
  data.Condition->PrintSelf( cout, indent );

  data.Lock->Delete();
  data.Condition->Delete();
  threader->Delete();
  return 1; // Always fail so the messages we print will show up on the dashboard.
}
