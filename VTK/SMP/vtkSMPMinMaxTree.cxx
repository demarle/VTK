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

#include "vtkBenchTimer.h"

class vtkScalarNode {};

template <class TScalar>
class vtkScalarRange : public vtkScalarNode
{
public:
  TScalar min;
  TScalar max;
};

class InitializeFunctor : public vtkFunctor
{
  InitializeFunctor( const InitializeFunctor& );
  void operator =( const InitializeFunctor& );

protected:
  InitializeFunctor()
    {
    this->TLS_Cell = vtkSMP::vtkThreadLocal<vtkGenericCell>::New();
    this->TLS_CellScalars = vtkSMP::vtkThreadLocal<vtkDoubleArray>::New();
    }
  ~InitializeFunctor()
    {
    this->TLS_Cell->Delete();
    this->TLS_CellScalars->Delete();
    }

  vtkScalarRange<double> *Tree;
  vtkIdType Size, BF;
  vtkDataSet* DS;
  vtkDataArray* Scalars;
  vtkSMP::vtkThreadLocal<vtkGenericCell>* TLS_Cell;
  vtkSMP::vtkThreadLocal<vtkDoubleArray>* TLS_CellScalars;

public:
  vtkTypeMacro(InitializeFunctor,vtkFunctor);
  static InitializeFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void InitializeData( vtkSMPMinMaxTree* t )
    {
    Tree = static_cast<vtkScalarRange<double>*>(t->Tree) + t->LeafOffset;
    BF = t->BranchingFactor;
    DS = t->DataSet;
    Scalars = t->Scalars;
    Size = DS->GetNumberOfCells();
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    double my_min = VTK_DOUBLE_MAX;
    double my_max = -VTK_DOUBLE_MAX;

    vtkGenericCell* cell = this->TLS_Cell->GetLocal( tid );
    if ( !cell )
      cell = this->TLS_Cell->GetLocal( tid );
    vtkDoubleArray* cellScalars = this->TLS_CellScalars->NewLocal( tid );
    if ( !cellScalars )
      cellScalars = this->TLS_CellScalars->NewLocal( tid );
    double* s;
    for ( vtkIdType i = 0, cellId = index * this->BF; i < this->BF && cellId < this->Size; ++i, ++cellId )
      {
      this->DS->GetCell( cellId, cell );
      vtkIdList* cellPts = cell->GetPointIds();
      vtkIdType n = cellPts->GetNumberOfIds();
      cellScalars->SetNumberOfTuples( n );
      this->Scalars->GetTuples( cellPts, cellScalars );
      s = cellScalars->GetPointer( 0 );

      while ( n-- )
        {
        if ( s[n] < my_min )
          {
          my_min = s[n];
          }
        if ( s[n] > my_max )
          {
          my_max = s[n];
          }
        }
      }
    this->Tree[index].max = my_max;
    this->Tree[index].min = my_min;
    }
};

vtkStandardNewMacro(InitializeFunctor);

class BuildFunctor : public vtkFunctor
{
  BuildFunctor( const BuildFunctor& );
  void operator =( const BuildFunctor& );

  vtkScalarRange<double>* Tree;
  vtkIdType BF, Size;

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
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    double my_min = VTK_DOUBLE_MAX;
    double my_max = -VTK_DOUBLE_MAX;
    vtkIdType branch = index * this->BF + 1;
    vtkScalarRange<double> *t = this->Tree + branch;

    for ( vtkIdType i = 0; i < this->BF && branch < this->Size; ++i, ++branch, ++t )
      {
      if ( t->min < my_min )
        {
        my_min = t->min;
        }
      if ( t->max > my_max )
        {
        my_max = t->max;
        }
      }

    this->Tree[index].max = my_max;
    this->Tree[index].min = my_min;
    }
};

vtkStandardNewMacro(BuildFunctor);

vtkStandardNewMacro(vtkSMPMinMaxTree);

vtkSMPMinMaxTree::vtkSMPMinMaxTree()
  {
  }

vtkSMPMinMaxTree::~vtkSMPMinMaxTree()
  {
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

  vtkBenchTimer::New()->start_bench_timer();
  this->Initialize();

  // Compute the number of levels in the tree
  //
  numLeafs = ((numCells - 1) / this->BranchingFactor) + 1;
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

  InitializeFunctor* InitTree = InitializeFunctor::New();
  InitTree->InitializeData( this );
  vtkSMP::ForEach( 0, numLeafs, InitTree );
  InitTree->Delete();

  BuildFunctor* BuildCells = BuildFunctor::New();
  BuildCells->InitializeData( this );
  vtkIdType start, end = this->LeafOffset;
  for ( int i = this->Level; i > 0; --i )
    {
    start = ( end - 1 ) / this->BranchingFactor;
    vtkSMP::ForEach( start, end, BuildCells );
    end = start;
    }
  BuildCells->Delete();

  this->BuildTime.Modified();
  vtkBenchTimer::New()->end_bench_timer();
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
