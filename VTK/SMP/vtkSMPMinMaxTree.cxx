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
#include "vtkMutexLock.h"

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

typedef vtkMutexLock* vtkMutexLockPtr;

class DeleteFunctor : public vtkFunctor
{
  DeleteFunctor( const DeleteFunctor& );
  void operator =( const DeleteFunctor& );

protected:
  DeleteFunctor() { }
  ~DeleteFunctor() { }

public:
  vtkTypeMacro(DeleteFunctor,vtkFunctor);
  static DeleteFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  vtkMutexLockPtr* Locks;

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    this->Locks[index]->Delete();
    }
};

vtkStandardNewMacro(DeleteFunctor);

class InitializeFunctor : public vtkFunctor
{
  InitializeFunctor( const InitializeFunctor& );
  void operator =( const InitializeFunctor& );

protected:
  InitializeFunctor() { }
  ~InitializeFunctor() { }

public:
  vtkTypeMacro(InitializeFunctor,vtkFunctor);
  static InitializeFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  vtkMutexLockPtr* Locks;
  vtkScalarRange<double> *Tree;

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    Tree[index].max = -VTK_DOUBLE_MAX;
    Tree[index].min = VTK_DOUBLE_MIN;
    this->Locks[index] = vtkMutexLock::New();
    }
};

vtkStandardNewMacro(InitializeFunctor);

class BuildFunctor : public vtkFunctor
{
  BuildFunctor( const BuildFunctor& );
  void operator =( const BuildFunctor& );

  vtkScalarRange<double>* Tree;
  vtkIdType BF, Level, LeafOffset;
  vtkDataSet* DS;
  vtkDataArray* Scalars;
  vtkMutexLockPtr* Locks;

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
    Level = t->Level;
    LeafOffset = t->LeafOffset;
    DS = t->DataSet;
    Scalars = t->Scalars;
    Locks = t->Locks;
    }

  void operator ()( vtkIdType index, vtkSMPThreadID tid ) const
    {
    vtkIdType pos = (index - this->LeafOffset) * this->BF;
    vtkIdType max = this->DS->GetNumberOfCells() - pos, i, n;
    int level = this->Level;

    double my_min = VTK_DOUBLE_MAX;
    double my_max = -VTK_DOUBLE_MAX;

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

    bool changed = true;
    vtkScalarRange<double> *t;
    while ( changed && level-- )
      {
      changed = false;
      index = (index - 1) / this->BF;

      t = this->Tree + index;

      if ( t->min > my_min )
        {
        t->min = my_min;
        changed = true;
        }
      if ( t->max < my_max )
        {
        t->max = my_max;
        changed = true;
        }
      }
    }
};

vtkStandardNewMacro(BuildFunctor);

vtkStandardNewMacro(vtkSMPMinMaxTree);

vtkSMPMinMaxTree::vtkSMPMinMaxTree()
  {
  this->Locks = 0;
  }

vtkSMPMinMaxTree::~vtkSMPMinMaxTree()
  {
  if ( this->Locks )
    this->DeleteLocks();
  }

void vtkSMPMinMaxTree::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }

void vtkSMPMinMaxTree::DeleteLocks()
  {
  DeleteFunctor* sweep = DeleteFunctor::New();
  sweep->Locks = this->Locks;
  vtkSMP::ForEach( 0, this->LeafOffset, sweep );
  sweep->Delete();
  delete [] this->Locks;
  this->Locks = 0;
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
  if ( this->Locks )
    this->DeleteLocks();

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
  this->Locks = new vtkMutexLockPtr[this->LeafOffset];

  InitializeFunctor* InitTree = InitializeFunctor::New();
  InitTree->Locks = this->Locks;
  InitTree->Tree = TTree;
  vtkSMP::ForEach( 0, this->LeafOffset, InitTree );
  InitTree->Delete();

  BuildFunctor* BuildCells = BuildFunctor::New();
  BuildCells->InitializeData( this );
  vtkSMP::ForEach( this->LeafOffset, this->TreeSize, BuildCells );
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
