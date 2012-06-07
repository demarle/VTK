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

  virtual vtkIdType GetAncestor( vtkIdType id, int lvl, int desiredLvl ) const;
  virtual vtkIdType GetLastDescendant( vtkIdType id, int lvl ) const;
  // In place transformation of indices and levels
  virtual void GetNextStealableNode( vtkIdType* stealedId, int* stealedLvl ) const;
  virtual void TraverseNode( vtkIdType* index, int* level, vtkFunctor* function, vtkSMPThreadID tid ) const;

};

#endif // VTKSMPMINMAXTREE_H
