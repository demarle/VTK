#include "vtkSMPMinMaxTree.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkSMP.h"
#include "vtkIdList.h"
#include "vtkCell.h"
#include "vtkDataArray.h"

class vtkScalarNode {};

template <class TScalar>
class vtkScalarRange : public vtkScalarNode
{
public:
  TScalar min;
  TScalar max;
};

class SumFunctor : public vtkFunctor
{
  SumFunctor( const SumFunctor& );
  void operator =( const SumFunctor& );

  vtkSMPMinMaxTree* Tree;
  int level;
  vtkIdType offset;

protected:
  SumFunctor() { Sum = 0; }
  ~SumFunctor() { }

public:
  vtkIdType* Sum;

  vtkTypeMacro(SumFunctor,vtkFunctor);
  static SumFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void InitializeData( vtkSMPMinMaxTree* t, int l, vtkIdType o, vtkIdType* s )
    {
    Tree = t;
    level = l;
    offset = o;
    Sum = s;
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    vtkIdType res = Tree->SumOverlapingCells( offset + index, level );
    __sync_fetch_and_add(this->Sum, res);
    }
};

vtkStandardNewMacro(SumFunctor);

vtkStandardNewMacro(vtkSMPMinMaxTree);

vtkSMPMinMaxTree::vtkSMPMinMaxTree() { }

vtkSMPMinMaxTree::~vtkSMPMinMaxTree() { }

void vtkSMPMinMaxTree::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }

double vtkSMPMinMaxTree::GetTraversedCell( vtkIdType callNumber, vtkIdType& realCellId, vtkCell *cell, vtkDataArray* cellScalars )
  {
  vtkIdType SkipLeaf = callNumber/this->BranchingFactor;
  vtkIdType TraversedLeafOffset = this->GetFirstCell( SkipLeaf );
  realCellId = TraversedLeafOffset ? TraversedLeafOffset + (callNumber - SkipLeaf*this->BranchingFactor) : callNumber;
  cell = this->DataSet->GetCell( realCellId );
  vtkIdList* cellPts = cell->GetPointIds();
  cellScalars->SetNumberOfTuples( cellPts->GetNumberOfIds() );
  this->Scalars->GetTuples( cellPts, cellScalars );
  return this->ScalarValue;
  }

vtkIdType vtkSMPMinMaxTree::GetFirstCell( vtkIdType skip )
  {
  vtkIdType index = this->LeafOffset;

  for ( ; skip >= 0; ++index )
    {
    vtkScalarRange<double> *tree = static_cast<vtkScalarRange<double>*>(this->Tree) + index;
    if ( tree->min <= this->ScalarValue && tree->max >= this->ScalarValue )
      --skip;
    }

  return index;//( index - 1) * this->BranchingFactor + 1;
  }

vtkIdType vtkSMPMinMaxTree::GetNumberOfTraversedCells( double value )
  {
  this->InitTraversal( value ); //TODO rewrite
  return this->SumOverlapingCells( 0, 0 );
  }

vtkIdType vtkSMPMinMaxTree::SumOverlapingCells( vtkIdType index, int level )
  {
  vtkScalarRange<double> *tree = static_cast<vtkScalarRange<double>*>(this->Tree) + index;

  if ( tree->min > this->ScalarValue || tree->max < this->ScalarValue )
    {
    return 0;
    }
  else
    {
    if ( level < this->Level )
      {
      vtkIdType res = 0;
      SumFunctor* my_sum = SumFunctor::New();
      my_sum->InitializeData( this, level+1, this->BranchingFactor*index+1, &res);
      vtkSMP::ForEach( 0, this->BranchingFactor, my_sum );
      my_sum->Delete();
      return res;
      }
    else
      {
      vtkIdType cellId = (index - this->LeafOffset) * this->BranchingFactor;
      vtkIdType maxCells = this->DataSet->GetNumberOfCells();
      return cellId + this->BranchingFactor > maxCells ? maxCells - cellId : this->BranchingFactor;
      }
    }
  }
