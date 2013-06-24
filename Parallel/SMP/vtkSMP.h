#ifndef __vtkSMP_h__
#define __vtkSMP_h__

#include "vtkParallelSMPModule.h" // For export macro
#include <vector>

#include "vtkFunctorInitializable.h"
#include "vtkThreadLocal.h"

class vtkCellArray;
class vtkCellData;
class vtkFunctor;
class vtkFunctorIntializable;
class vtkParallelTree;
class vtkPointData;
class vtkPoints;
class vtkSMPMergePoints;
class vtkTask;

int vtkSMPInternalGetNumberOfThreads();
int vtkSMPInternalGetTid();

// ForEach template : parallel loop over an iterator
void VTKPARALLELSMP_EXPORT vtkSMPForEachOp(
  vtkIdType first, vtkIdType last, const vtkFunctor* op, int grain = 0
  );

void VTKPARALLELSMP_EXPORT vtkSMPForEachOp(
  vtkIdType first, vtkIdType last, const vtkFunctorInitializable* f, int grain = 0
  );

#define vtkSMPStaticForEachOp vtkSMPForEachOp

template<class T>
void VTKPARALLELSMP_EXPORT vtkSMPParallelOp(
  const vtkTask* function,
  typename vtkThreadLocal<T>::iterator data1,
  vtkIdType skipThreads = 1
  );

template<class T1, class T2, class T3, class T4, class T5, class T6>
void VTKPARALLELSMP_EXPORT vtkSMPParallelOp(
  const vtkTask* function,
  typename vtkThreadLocal<T1>::iterator data1,
  typename vtkThreadLocal<T2>::iterator data2,
  typename vtkThreadLocal<T3>::iterator data3,
  typename vtkThreadLocal<T4>::iterator data4,
  typename vtkThreadLocal<T5>::iterator data5,
  typename vtkThreadLocal<T6>::iterator data6,
  std::vector<vtkIdType>::iterator offset1,
  std::vector<vtkIdType>::iterator offset2,
  std::vector<vtkIdType>::iterator offset3,
  std::vector<vtkIdType>::iterator offset4,
  std::vector<vtkIdType>::iterator offset5,
  std::vector<vtkIdType>::iterator offset6,
  std::vector<vtkIdType>::iterator offset7,
  std::vector<vtkIdType>::iterator offset8,
  vtkIdType skipThreads = 1 );

void VTKPARALLELSMP_EXPORT vtkSMPTraverseOp( const vtkParallelTree* Tree, vtkFunctor* func );

#endif //__vtkSMP_h__
