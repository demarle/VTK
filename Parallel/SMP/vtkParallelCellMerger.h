#ifndef _vtkParallelCellMerger_h_
#define _vtkParallelCellMerger_h_

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkTask.h"

class vtkCellData;
class vtkCellArray;
class vtkDummyMergeFunctor;

class VTKPARALLELSMP_EXPORT vtkParallelCellMerger : public vtkTask
{
public:
  vtkDummyMergeFunctor* self;

  vtkTypeMacro(vtkParallelCellMerger,vtkTask);
  static vtkParallelCellMerger* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void Execute( vtkIdList* map,
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
                vtkIdType stripTupleOffset ) const;
protected:
  vtkParallelCellMerger() {}
  ~vtkParallelCellMerger() {}
private:
  vtkParallelCellMerger(const vtkParallelCellMerger&); // Not implemented
  void operator =(const vtkParallelCellMerger&); // Not implemented
};

#endif //_vtkParallelCellMerger_h_
