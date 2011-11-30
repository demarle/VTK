#include "vtkSMP.h"

#include "vtkPoints.h"
#include "vtkFloatArray.h"

namespace vtkSMP
{
  void ForEach(vtkIdType first, vtkIdType last, const vtkFunctor& op)
    {
    InternalForEach( first, last, &op );
    }

//  void ForEachCoordinates(vtkPoints* data, void(*op)(float&))
//    {
//    float* ptr = vtkFloatArray::SafeDownCast( data->GetData() )->GetPointer( 0 );
//    InternalForEach( ptr, ptr + (data->GetNumberOfPoints() * 3), op );
//    }

  vtkMutexLocker::vtkMutexLocker(vtkMutexLock* lock)
    {
    this->Lock = lock;
    this->Lock->Lock();
    }
  vtkMutexLocker::~vtkMutexLocker()
    {
    this->Lock->Unlock();
    }

}
