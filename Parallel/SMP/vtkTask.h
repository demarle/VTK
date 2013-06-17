#ifndef __vtkTask_h__
#define __vtkTask_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"

class vtkCellArray;
class vtkCellData;
class vtkIdList;
class vtkSMPMergePoints;

//======================================================================================
class VTKPARALLELSMP_EXPORT vtkTask : public vtkObjectBase
{
  vtkTask(const vtkTask&);
  void operator =(const vtkTask&);

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
};

#endif // __vtkTask_h__
