#ifndef _vtkLockPointMerger_h_
#define _vtkLockPointMerger_h_

//extra synchonization locks for case when you lack allocators per CPU

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkFunctor.h"

class vtkPoints;
class vtkPointData;
class vtkDummyMergeFunctor;

class VTKPARALLELSMP_EXPORT vtkLockPointMerger : public vtkFunctor
{
public:
  vtkDummyMergeFunctor* Functor;
  vtkIdType NumberOfPointsFirstThread;

  vtkTypeMacro(vtkLockPointMerger,vtkFunctor);
  static vtkLockPointMerger* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void operator()( vtkIdType id ) const;

protected:
  vtkLockPointMerger() {}
  ~vtkLockPointMerger() {}
private:
  vtkLockPointMerger(const vtkLockPointMerger&); // Not implemented
  void operator=(const vtkLockPointMerger&); // Not implemented
};

#endif //_vtkLockPointMerger_h_
