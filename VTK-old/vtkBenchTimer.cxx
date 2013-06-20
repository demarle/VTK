#include "vtkBenchTimer.h"
#include "vtkObjectFactory.h"

vtkInstantiatorNewMacro(vtkBenchTimer);

vtkBenchTimer::vtkBenchTimer() : vtkObject(), depth_level ( 0 ) { }

bool vtkBenchTimer::isActive = true;
vtkBenchTimer* vtkBenchTimer::instance = 0;
vtkBenchTimer::~vtkBenchTimer()
{
  if ( instance )
    instance = 0;
}

void vtkBenchTimer::Deactivate()
  {
  isActive = false;
  }

void vtkBenchTimer::Activate()
  {
  isActive = true;
  }

void vtkBenchTimer::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << "Depth level " << depth_level << endl;
  os << "Instance " << instance << endl;
}

vtkBenchTimer* vtkBenchTimer::New()
  {
  if ( !instance )
    {
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkBenchTimer");
    if(ret)
      {
      instance = static_cast<vtkBenchTimer*>(ret);
      }
    else
      {
      instance = new vtkBenchTimer();
      }
    }
  return instance;
  }

void vtkBenchTimer::start_bench_timer()
{
  if (!isActive)
    return;
  if ( ret_value.size() == depth_level )
  {
    ret_value.push_back( 0 );
    struct timespec __t0, __t1;
    t0.push_back( __t0 );
    t1.push_back( __t1 );
  }
  ret_value[depth_level] = clock_gettime(CLOCK_REALTIME, &t0[depth_level]);
  ++depth_level;
}

void vtkBenchTimer::end_bench_timer()
{
  if (!isActive)
    {
    cout << ". ";
    return;
    }
  --depth_level;
  ret_value[depth_level] += clock_gettime(CLOCK_REALTIME, &t1[depth_level]);
  int s = t1[depth_level].tv_sec - t0[depth_level].tv_sec;
  int ns = t1[depth_level].tv_nsec - t0[depth_level].tv_nsec;
  if ( ns < 0 ) { s -= 1; ns += 1000000000; }
  if ( s )
    {
    cout << s;
    if ( ns < 100000000 ) cout << 0;
    if ( ns < 10000000 ) cout << 0;
    if ( ns < 1000000 ) cout << 0;
    if ( ns < 100000 ) cout << 0;
    if ( ns < 10000 ) cout << 0;
    if ( ns < 1000 ) cout << 0;
    if ( ns < 100 ) cout << 0;
    if ( ns < 10 ) cout << 0;
    }
  cout << ns << (ret_value[depth_level] ? "! " : " ");
}
