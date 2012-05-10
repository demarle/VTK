#ifndef VTKSMPMINMAXTREE_H
#define VTKSMPMINMAXTREE_H

#include "vtkSimpleScalarTree.h"
#include "vtkSMP.h"

class vtkGenericCell;
class TreeFunctor;
class BuildFunctor;

class VTK_SMP_EXPORT vtkSMPMinMaxTree : public vtkSimpleScalarTree
{
  friend class TreeFunctor;
  friend class BuildFunctor;
  vtkSMPMinMaxTree( const vtkSMPMinMaxTree& );
  void operator =( const vtkSMPMinMaxTree& );

protected:
  vtkSMPMinMaxTree();
  ~vtkSMPMinMaxTree();

public:
  vtkTypeMacro(vtkSMPMinMaxTree, vtkSimpleScalarTree);
  static vtkSMPMinMaxTree* New();
  void PrintSelf(ostream &os, vtkIndent indent);

//  double GetTraversedCell( vtkIdType callNumber, vtkIdType& realCellId, vtkGenericCell* cell, vtkDataArray* cellScalars );
  vtkSetMacro(RemainingCellsOp, vtkFunctor*);
  void ComputeOperationOverCellsOfInterest( double value );
  void BuildTree();

protected:
  void ComputeOverlapingCells( vtkIdType index, int level );
  void InternalBuildTree( vtkIdType index, int level );

  vtkFunctor* RemainingCellsOp;
};

#endif // VTKSMPMINMAXTREE_H
