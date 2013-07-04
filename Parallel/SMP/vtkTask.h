#ifndef __vtkTask_h__
#define __vtkTask_h__

//Base class for specifying some code that is to be run on each thread.
//vtkTask is something like a heavy weight vtkFunctor

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObjectBase.h"
#include "vtkObject.h" // TODO multiple includes

class vtkCellArray;
class vtkCellData;
class vtkIdList;
class vtkSMPMergePoints;

//======================================================================================
class VTKPARALLELSMP_EXPORT vtkTask : public vtkObjectBase
{
public:
  vtkTypeMacro(vtkTask, vtkObjectBase);
  void PrintSelf(ostream &os, vtkIndent indent);

  void Execute( ... ) const
    {
    cout << "Shouldn't be invoked." << endl;
    }
  virtual void Execute( vtkSMPMergePoints* ) const {}
  virtual void Execute( vtkIdList* map,
                        vtkCellData* clData,
                        vtkCellArray* verts,
                        vtkCellArray* lines,
                        vtkCellArray* polys,
                        vtkCellArray* strips,
                        vtkIdType vertCellOffset,
                        vtkIdType vertTupleOffset,
                        vtkIdType lineCellOffset,
                        vtkIdType lineTupleOffset,
                        vtkIdType polyCellOffset,
                        vtkIdType polyTupleOffset,
                        vtkIdType stripCellOffset,
                        vtkIdType stripTupleOffset ) const {}

protected:
  vtkTask();
  ~vtkTask();

private:
  vtkTask(const vtkTask&); // Not implemented
  void operator=(const vtkTask&); // Not implemented
};

#endif // __vtkTask_h__
