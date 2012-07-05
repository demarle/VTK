#include "vtkSMPMinMaxTree.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkSMP.h"
#include "vtkIdList.h"
#include "vtkGenericCell.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"

class vtkScalarNode {};

template <class TScalar>
class vtkScalarRange : public vtkScalarNode
{
public:
  TScalar min;
  TScalar max;
};

class MyAtomic
{
  vtkIdType atomic_counter;

public:
  MyAtomic()
    {
    atomic_counter = 0;
    }

  vtkIdType GetMyNumber()
    {
    return __sync_add_and_fetch( &(this->atomic_counter), 1 );
    }
};

class BuildFunctor : public vtkFunctor
{
  BuildFunctor( const BuildFunctor& );
  void operator =( const BuildFunctor& );

  vtkScalarRange<double>* Tree;
  vtkIdType BF, Level, LeafOffset, Size;
  vtkDataSet* DS;
  vtkDataArray* Scalars;
  MyAtomic* counters;

protected:
  BuildFunctor() { }
  ~BuildFunctor() { }

public:

  vtkTypeMacro(BuildFunctor,vtkFunctor);
  static BuildFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void InitializeData( vtkSMPMinMaxTree* t )
    {
    Tree = static_cast<vtkScalarRange<double>*>(t->Tree);
    BF = t->BranchingFactor;
    Size = t->TreeSize;
    Level = t->Level;
    LeafOffset = t->LeafOffset;
    DS = t->DataSet;
    Scalars = t->Scalars;
    counters = t->counters;
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    vtkIdType pos = (index - this->LeafOffset) * this->BF;
    vtkIdType max = this->DS->GetNumberOfCells() - pos, i, n;
    int level = this->Level;
    vtkScalarRange<double> *t, *tt;

    if ( index < this->Size )
      {
      t = this->Tree + index;
      t->min = VTK_DOUBLE_MAX;
      t->max = -VTK_DOUBLE_MAX;

      vtkGenericCell* cell = vtkGenericCell::New();
      vtkDoubleArray* cellScalars = vtkDoubleArray::New();
      double* s;
      for ( i = 0; i < this->BF && i < max; ++i )
        {
        this->DS->GetCell( pos + i, cell );
        vtkIdList* cellPts = cell->GetPointIds();
        n = cellPts->GetNumberOfIds();
        cellScalars->SetNumberOfTuples( n );
        this->Scalars->GetTuples( cellPts, cellScalars );
        s = cellScalars->GetPointer( 0 );

        while ( n != 0 )
          {
          --n;
          if ( s[n] < t->min )
            {
            t->min = s[n];
            }
          if ( s[n] > t->max )
            {
            t->max = s[n];
            }
          }
        }
      }

    while ( level >= 0 )
      {
      index = (index - 1) / this->BF;
      if ( this->counters[index].GetMyNumber() != this->BF )
        return;
      t = this->Tree + index;
      t->min = VTK_DOUBLE_MAX;
      t->max = -VTK_DOUBLE_MAX;

      i = index * this->BF + 1;
      tt = this->Tree + i;

      --level;
      for ( vtkIdType j = 0; j < this->BF && i < this->Size; ++i, ++j )
        {
        if ( t->min > tt[j].min )
          t->min = tt[j].min;
        if ( t->max < tt[j].max )
          t->max = tt[j].max;
        }
      }
    }
};

vtkStandardNewMacro(BuildFunctor);

vtkStandardNewMacro(vtkSMPMinMaxTree);

vtkSMPMinMaxTree::vtkSMPMinMaxTree()
  {
  this->counters = 0;
  }

vtkSMPMinMaxTree::~vtkSMPMinMaxTree()
  {
  if ( this->counters )
    delete [] this->counters;
  }

void vtkSMPMinMaxTree::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }

void vtkSMPMinMaxTree::BuildTree()
  {
  vtkIdType numCells;
  int offset, prod;
  vtkIdType numNodes, numLeafs;

  // Check input...see whether we have to rebuild
  //
  if ( !this->DataSet || (numCells = this->DataSet->GetNumberOfCells()) < 1 )
    {
    vtkErrorMacro( << "No data to build tree with");
    return;
    }

  if ( this->Tree != NULL && this->BuildTime > this->MTime
       && this->BuildTime > this->DataSet->GetMTime() )
    {
    return;
    }

  vtkDebugMacro( << "Building scalar tree..." );

  this->Scalars = this->DataSet->GetPointData()->GetScalars();
  if ( ! this->Scalars )
    {
    vtkErrorMacro( << "No scalar data to build trees with");
    return;
    }

  this->Initialize();

  // Compute the number of levels in the tree
  //
  numLeafs = static_cast<int>(
        ceil(static_cast<double>(numCells)/this->BranchingFactor));
  for (prod=1, numNodes=1, this->Level=0;
       prod < numLeafs && this->Level <= this->MaxLevel; this->Level++ )
    {
    prod *= this->BranchingFactor;
    numNodes += prod;
    }

  this->LeafOffset = offset = numNodes - prod;
  vtkScalarRange<double> *TTree;
  this->TreeSize = numNodes - (prod - numLeafs);
  this->Tree = TTree = new vtkScalarRange<double>[this->TreeSize];
  if ( this->counters )
    delete [] this->counters;
  this->counters = new MyAtomic[this->LeafOffset];

  BuildFunctor* BuildCells = BuildFunctor::New();
  BuildCells->InitializeData( this );
  vtkSMP::ForEach( this->LeafOffset, this->LeafOffset * this->BranchingFactor + 1, BuildCells );
  BuildCells->Delete();

  this->BuildTime.Modified();
  }

void vtkSMPMinMaxTree::InitTraversal(double scalarValue)
  {
  this->BuildTree();
  vtkScalarRange<double> *TTree =
      static_cast< vtkScalarRange<double> * > (this->Tree);

  this->ScalarValue = scalarValue;
  this->TreeIndex = this->TreeSize;
  }

void vtkSMPMinMaxTree::TraverseNode( vtkIdType id, int lvl, vtkTreeTraversalHelper* th, vtkFunctor* function, vtkSMPThreadID tid ) const
  {
  if ( id >= this->TreeSize )
    {
    return;
    }

  vtkScalarRange<double> *t = static_cast<vtkScalarRange<double>*>(this->Tree) + id;
  if ( t->min > this->ScalarValue || t->max < this->ScalarValue )
    {
    return;
    }

  if ( lvl == this->Level ) //leaf
    {
    vtkIdType cell_id = ( id - this->LeafOffset ) * this->BranchingFactor;
    vtkIdType max_id = this->DataSet->GetNumberOfCells();
    for ( vtkIdType i = 0; i < this->BranchingFactor && cell_id < max_id; ++i, ++cell_id )
      {
      (*function)( cell_id, tid );
      }
    }
  else //node
    {
    vtkIdType index = ( id + 1 ) * this->BranchingFactor;
    int level = lvl + 1;
    for ( vtkIdType i = 0; i < this->BranchingFactor; ++i, --index )
      {
      th->push_tail( index, level );
      }
    }
  }

vtkIdType vtkSMPMinMaxTree::GetTreeSize() const
  {
  return (this->BranchingFactor - 1) * this->Level;
  }
