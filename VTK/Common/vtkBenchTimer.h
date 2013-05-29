#ifndef VTKBENCHTIMER_H
#define VTKBENCHTIMER_H

#include "vtkObject.h"
#include <time.h>
#include <sys/time.h>
#include <vector>

class VTK_COMMON_EXPORT vtkBenchTimer : public vtkObject
{
  vtkstd::vector<struct timespec> t0, t1;
  vtkstd::vector<int> ret_value;
  size_t depth_level;

  vtkBenchTimer (const vtkBenchTimer&);
  void operator = (const vtkBenchTimer&);

  static vtkBenchTimer* instance;
  static bool isActive;

protected:
  vtkBenchTimer ();
  ~vtkBenchTimer ();

public:
  vtkTypeMacro(vtkBenchTimer,vtkObject);
  static vtkBenchTimer* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void start_bench_timer();
  void end_bench_timer();

  static void Deactivate();
  static void Activate();
};

#endif // VTKBENCHTIMER_H
