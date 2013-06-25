#ifndef __vtkParallelOperators_h__
#define __vtkParallelOperators_h__

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkObject.h"
#include <vector>

class vtkFunctor;
class vtkFunctorIntializable;
class vtkParallelTree;

class VTKPARALLELSMP_EXPORT vtkParallelOperators : public vtkObject
{
    vtkParallelOperators(const vtkParallelOperators&);
    void operator=(const vtkParallelOperators&);

  protected:
    vtkParallelOperators();
    ~vtkParallelOperators();

  public:
    vtkTypeMacro(vtkParallelOperators,vtkObject);
    static vtkParallelOperators* New();
    void PrintSelf(ostream& os, vtkIndent indent);

    // ForEach template : parallel loop over an iterator
    void ForEach(
      vtkIdType first, vtkIdType last,
      const vtkFunctor* op, int grain = 0
    );

    void ForEach(
      vtkIdType first, vtkIdType last,
      const vtkFunctorInitializable* f, int grain = 0
    );
 
    // Same as ForEach but with a guaranteed static partitioning
    void StaticForEach(
        vtkIdType first, vtkIdType last,
        const vtkFunctor* op, int grain = 0)
      {
      ForEach(first,last,op,grain);
      } 

    void StaticForEach(
        vtkIdType first, vtkIdType last,
        const vtkFunctorInitializable* op, int grain = 0)
      {
      ForEach(first,last,op,grain);
      } 

    void Traverse(const vtkParallelTree* Tree, vtkFunctor* func);
};

#endif //__vtkParallelOperators_h__
