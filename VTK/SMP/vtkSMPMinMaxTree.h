#ifndef VTKSMPMINMAXTREE_H
#define VTKSMPMINMAXTREE_H

#include "vtkSimpleScalarTree.h"
#include "vtkSMP.h"

class vtkGenericCell;
class TreeFunctor;
class BuildFunctor;

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

  virtual vtkTreeIndex GetAncestor( vtkTreeIndex id, int desiredLvl ) const;
  virtual vtkTreeIndex GetLastDescendant( vtkTreeIndex id ) const;
  virtual int GetMaximumSplittableLevel() const;
  // In place transformation of indices and levels
  virtual vtkTreeIndex GetNextStealableNode( vtkTreeIndex id ) const;
  virtual vtkTreeIndex TraverseNode( vtkTreeIndex id, vtkFunctor* function, vtkSMPThreadID tid ) const;

};

#endif // VTKSMPMINMAXTREE_H
