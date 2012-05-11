#include "vtkSMPMinMaxTree.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkSMP.h"
#include "vtkIdList.h"
#include "vtkGenericCell.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"

class vtkScalarNode {};

template <class TScalar>
class vtkScalarRange : public vtkScalarNode
{
public:
  TScalar min;
  TScalar max;
};

class TreeFunctor : public vtkFunctor
{
  TreeFunctor( const TreeFunctor& );
  void operator =( const TreeFunctor& );

  vtkSMPMinMaxTree* Tree;
  int level;
  vtkIdType offset;

protected:
  TreeFunctor() { }
  ~TreeFunctor() { }

public:

  vtkTypeMacro(TreeFunctor,vtkFunctor);
  static TreeFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void InitializeData( vtkSMPMinMaxTree* t, int l, vtkIdType o )
    {
    Tree = t;
    level = l;
    offset = o;
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    Tree->ComputeOverlapingCells( offset + index, level );
    }
};

vtkStandardNewMacro(TreeFunctor);

class BuildFunctor : public vtkFunctor
{
  BuildFunctor( const BuildFunctor& );
  void operator =( const BuildFunctor& );

  vtkSMPMinMaxTree* Tree;
  int level;
  vtkIdType offset;

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

  void InitializeData( vtkSMPMinMaxTree* t, int l, vtkIdType o )
    {
    Tree = t;
    level = l;
    offset = o;
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    Tree->InternalBuildTree( offset + index, level );
    }
};

vtkStandardNewMacro(BuildFunctor);

vtkStandardNewMacro(vtkSMPMinMaxTree);

vtkSMPMinMaxTree::vtkSMPMinMaxTree()
  {
  TraversedCells = vtkIdList::New();
  }

vtkSMPMinMaxTree::~vtkSMPMinMaxTree()
  {
  TraversedCells->Delete();
  }

void vtkSMPMinMaxTree::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }

double vtkSMPMinMaxTree::GetTraversedCell( vtkIdType callNumber, vtkIdType& realCellId, vtkGenericCell *cell, vtkDataArray* cellScalars )
  {
  realCellId = TraversedCells->GetId( callNumber );
  this->DataSet->GetCell( realCellId, cell );
  vtkIdList* cellPts = cell->GetPointIds();
  cellScalars->SetNumberOfTuples( cellPts->GetNumberOfIds() );
  this->Scalars->GetTuples( cellPts, cellScalars );
  return this->ScalarValue;
  }

vtkIdType vtkSMPMinMaxTree::ComputeNumberOfTraversedCells( double value )
  {
  this->BuildTree();
  this->ScalarValue = value;
  this->TreeIndex = this->TreeSize;

  this->NumberOfTraversedCells = 0;
  this->TraversedCells->Allocate(this->DataSet->GetNumberOfCells());

  vtkSMPThreadID numThreads = vtkSMP::GetNumberOfThreads();
  vtkIdType idx = 0;
  int lvl = 0;
  while ( idx < numThreads )
    {
    idx = this->BranchingFactor * idx + 1;
    ++lvl;
    }
  TreeFunctor* ComputeCells = TreeFunctor::New();
  ComputeCells->InitializeData( this, lvl, idx );
  vtkSMP::ForEach(0, this->BranchingFactor * idx + 1, ComputeCells);
  ComputeCells->Delete();

  this->TraversedCells->SetNumberOfIds(this->NumberOfTraversedCells);
  return this->NumberOfTraversedCells;
  }

void vtkSMPMinMaxTree::ComputeOverlapingCells( vtkIdType index, int level )
  {
//  printf( "Computing cells: index %d and level %d\n", index, level);
  vtkScalarRange<double> *tree = static_cast<vtkScalarRange<double>*>(this->Tree) + index;

  if ( tree->min <= this->ScalarValue && tree->max >= this->ScalarValue )
    {
    if ( level < this->Level )
      {
      vtkIdType offset = this->BranchingFactor * index + 1;
      vtkIdType NbSons = this->TreeSize - offset;
      if ( NbSons > 0 )
        {
        NbSons = NbSons > this->BranchingFactor ? this->BranchingFactor : NbSons;
        for ( vtkIdType i = 0; i < NbSons; ++i )
          {
          this->ComputeOverlapingCells( offset, level + 1 );
          }
        }
      }
    else
      {
      vtkIdType cellId = (index - this->LeafOffset) * this->BranchingFactor;
      vtkIdType maxCells = this->DataSet->GetNumberOfCells();
      vtkIdType NumberOfCellsInLeaf = cellId + this->BranchingFactor < maxCells ? this->BranchingFactor : maxCells - cellId;
      vtkIdType insertionId = __sync_fetch_and_add(&(this->NumberOfTraversedCells), NumberOfCellsInLeaf);
      for ( ; NumberOfCellsInLeaf > 0; --NumberOfCellsInLeaf, ++cellId, ++insertionId )
        {
        this->TraversedCells->SetId(insertionId, cellId);
        }
      }
    }
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

  vtkSMPThreadID numThreads = vtkSMP::GetNumberOfThreads();
  vtkIdType idx = 0;
  int lvl = 0;
  while ( idx < numThreads )
    {
    idx = this->BranchingFactor * idx + 1;
    ++lvl;
    }
  cout << "this->Level=" << this->Level << " start: " << idx << ", " << lvl << endl;
  BuildFunctor* BuildCells = BuildFunctor::New();
  BuildCells->InitializeData( this, lvl, idx );
  vtkSMP::ForEach(0, this->BranchingFactor * idx + 1, BuildCells);
  BuildCells->Delete();

  vtkIdType old_idx;
  while ( lvl )
    {
    --lvl;
    old_idx = idx;
    idx = ( idx - 1 ) / this->BranchingFactor;
    for ( offset = idx; offset < old_idx; ++offset )
      {
      TTree[offset].min = VTK_DOUBLE_MAX;
      TTree[offset].max = -VTK_DOUBLE_MAX;
      vtkIdType firstSon = offset * this->BranchingFactor + 1;
      for ( int i = 0; i < this->BranchingFactor; ++i )
        {
        if ( TTree[firstSon + i].min < TTree[offset].min ) TTree[offset].min = TTree[firstSon].min;
        if ( TTree[firstSon + i].max > TTree[offset].max ) TTree[offset].max = TTree[firstSon].max;
        }
      }
    }
  cout << "End building: min " << TTree->min << ", max " << TTree->max << endl;

  this->BuildTime.Modified();
}

void vtkSMPMinMaxTree::InternalBuildTree( vtkIdType index, int level )
  {
  vtkScalarRange<double> *tree = static_cast<vtkScalarRange<double>*>(this->Tree) + index;
  tree->min = VTK_DOUBLE_MAX;
  tree->max = -VTK_DOUBLE_MAX;
  vtkIdType j;

  if ( level < this->Level )
    {
    vtkIdType offset = this->BranchingFactor * index + 1;
    vtkIdType NbSons = this->TreeSize - offset;
    if ( NbSons > 0 )
      {
      NbSons = NbSons > this->BranchingFactor ? this->BranchingFactor : NbSons;
      vtkScalarRange<double> *son = static_cast<vtkScalarRange<double>*>(this->Tree) + offset;
      for ( j = 0; j < NbSons; ++j )
        {
        this->InternalBuildTree( offset + j, level + 1 );

        if ( son[j].min < tree->min )
          {
          tree->min = son[j].min;
          }
        if ( son[j].max > tree->max )
          {
          tree->max = son[j].max;
          }
        }
      }
    }
  else
    {
    vtkIdType cellId = (index - this->LeafOffset) * this->BranchingFactor;
    vtkIdType maxCells = this->DataSet->GetNumberOfCells();
    vtkDoubleArray* cellScalars = vtkDoubleArray::New();
    vtkCell* cell;
    vtkIdList* cellPts;
    vtkIdType numScalars;
    double* s;
    for ( vtkIdType NumberOfCells = 0; NumberOfCells < this->BranchingFactor && cellId < maxCells; ++NumberOfCells, ++cellId )
      {
      cell = this->DataSet->GetCell(cellId);
      cellPts = cell->GetPointIds();
      numScalars = cellPts->GetNumberOfIds();
      cellScalars->SetNumberOfTuples(numScalars);
      this->Scalars->GetTuples(cellPts, cellScalars);
      s = cellScalars->GetPointer(0);

      for ( j=0; j < numScalars; j++ )
        {
        if ( s[j] < tree->min )
          {
          tree->min = s[j];
          }
        if ( s[j] > tree->max )
          {
          tree->max = s[j];
          }
        }
      }
    cellScalars->Delete();
    }
  }
