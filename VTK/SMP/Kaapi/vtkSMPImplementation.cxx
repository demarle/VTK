#include "vtkSMPImplementation.h"
#include <kaapi++>

ka::Community* com;

void smpInit(void)
{
  com = new ka::Community(ka::System::join_community());
}
void smpFini(void)
{
  cout << "entering fini" << endl;
  com->leave();
  ka::System::terminate();
  cout << "exiting fini" << endl;
}

void functor( int32_t b, int32_t e, int32_t tid, const vtkFunctor* o, vtkIdType f )
{
  for ( int32_t k = b; k < e; ++k )
  {
    (*o)( f + k, tid );
  }
}

void InternalForEach ( vtkIdType first, vtkIdType last, const vtkFunctor* op )
{
  kaapic_foreach_attr_t attr;
  kaapic_foreach_attr_init(&attr);
  kaapic_foreach_attr_set_grains(&attr, 32, 32);
  kaapic_foreach( 0, last - first, &attr, 2, functor, op, first );
}

