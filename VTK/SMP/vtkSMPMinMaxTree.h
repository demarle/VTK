#ifndef VTKSMPMINMAXTREE_H
#define VTKSMPMINMAXTREE_H

#include "vtkSimpleScalarTree.h"
#include "vtkSMP.h"

class vtkGenericCell;
class InitializeFunctor;

class VTK_SMP_EXPORT vtkSMPMinMaxTree : public vtkSimpleScalarTree, public vtkParallelTree
{
  friend class InitializeFunctor;
  vtkSMPMinMaxTree( const vtkSMPMinMaxTree& );
  void operator =( const vtkSMPMinMaxTree& );

protected:
  vtkSMPMinMaxTree();
  ~vtkSMPMinMaxTree();

public:
  vtkTypeMacro(vtkSMPMinMaxTree, vtkSimpleScalarTree);
  static vtkSMPMinMaxTree* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  void BuildTree();
  void InitTraversal(double scalarValue);

  virtual int TraverseNode( vtkIdType id, int lvl, vtkFunctor* function, vtkSMPThreadID tid ) const;
  virtual void GetTreeSize ( int& max_level, vtkIdType& branching_factor ) const;

};

#endif // VTKSMPMINMAXTREE_H
