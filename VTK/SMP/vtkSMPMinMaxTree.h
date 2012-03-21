#ifndef VTKSMPMINMAXTREE_H
#define VTKSMPMINMAXTREE_H

#include "vtkSimpleScalarTree.h"

class vtkCell;
class SumFunctor;

class VTK_SMP_EXPORT vtkSMPMinMaxTree : public vtkSimpleScalarTree
{
  friend class SumFunctor;
  vtkSMPMinMaxTree( const vtkSMPMinMaxTree& );
  void operator =( const vtkSMPMinMaxTree& );

protected:
  vtkSMPMinMaxTree();
  ~vtkSMPMinMaxTree();

public:
  vtkTypeMacro(vtkSMPMinMaxTree, vtkSimpleScalarTree);
  static vtkSMPMinMaxTree* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  double GetTraversedCell( vtkIdType callNumber, vtkIdType& realCellId, vtkCell* cell, vtkDataArray* cellScalars );
  vtkIdType GetNumberOfTraversedCells( double value );

protected:
  vtkIdType SumOverlapingCells( vtkIdType index, int level );
  vtkIdType GetFirstCell( vtkIdType skip );
};

#endif // VTKSMPMINMAXTREE_H
