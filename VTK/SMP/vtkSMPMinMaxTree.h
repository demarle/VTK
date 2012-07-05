#ifndef VTKSMPMINMAXTREE_H
#define VTKSMPMINMAXTREE_H

#include "vtkSimpleScalarTree.h"
#include "vtkSMP.h"

class vtkGenericCell;
class BuildFunctor;
class MyAtomic;

class VTK_SMP_EXPORT vtkSMPMinMaxTree : public vtkSimpleScalarTree, public vtkParallelTree
{
  friend class TreeFunctor;
  friend class BuildFunctor;
  friend class BuildLeafFunctor;
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

  virtual void TraverseNode( vtkIdType id, int lvl, vtkTreeTraversalHelper* th, vtkFunctor* function, vtkSMPThreadID tid ) const;
  virtual vtkIdType GetTreeSize() const;

private:
  MyAtomic* counters;
};

#endif // VTKSMPMINMAXTREE_H
