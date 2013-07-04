#ifndef __vtkParallelOperators_h__
#define __vtkParallelOperators_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"
#include <vector> //TODO remove

class vtkFunctor;
class vtkFunctorInitializable;
class vtkParallelTree;

class VTKPARALLELSMP_EXPORT vtkParallelOperators : public vtkObject
{
  protected:
    vtkParallelOperators();
    ~vtkParallelOperators();

  public:
    vtkTypeMacro(vtkParallelOperators,vtkObject);
    static vtkParallelOperators* New();
    void PrintSelf(ostream& os, vtkIndent indent);

    // ForEach template : parallel loop over an iterator
    static void ForEach(
      vtkIdType first, vtkIdType last,
      const vtkFunctor* op, int grain = 0
    );

    static void ForEach(
      vtkIdType first, vtkIdType last,
      const vtkFunctorInitializable* f, int grain = 0
    );

    // Same as ForEach but with a guaranteed static partitioning
    static void StaticForEach(
        vtkIdType first, vtkIdType last,
        const vtkFunctor* op, int grain = 0)
      {
      ForEach(first,last,op,grain);
      }

    static void StaticForEach(
        vtkIdType first, vtkIdType last,
        const vtkFunctorInitializable* op, int grain = 0)
      {
      ForEach(first,last,op,grain);
      }

    static void Traverse(const vtkParallelTree* Tree, vtkFunctor* func);
  private:
    vtkParallelOperators(const vtkParallelOperators&); // Not implemented
    void operator=(const vtkParallelOperators&); // Not implemented

};

#endif //__vtkParallelOperators_h__
