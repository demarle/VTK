#include "vtkSMPZCurve.h"
#include "vtkSMP.h"
#include "vtkObjectFactory.h"
#include "vtkOctreePointLocator.h"
#include "vtkIdList.h"
#include "vtkDataSet.h"
#include "vtkOctreePointLocatorNode.h"
#include "vtkMergePoints.h"
#include "vtkPointSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkGenericCell.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

class MyOctreeLocator : public vtkOctreePointLocator
{
  public:
    static MyOctreeLocator* New();
    vtkTypeMacro(MyOctreeLocator, vtkOctreePointLocator);
    void PrintSelf(ostream &os, vtkIndent indent)
      {
      this->Superclass::PrintSelf(os, indent);
      }

    int GetSortedPoints( vtkIdType id, double pt[3] )
      {
      int pointId = this->LocatorIds[id];
      this->DataSet->GetPoint( pointId, pt );
      return pointId;
      }

    int GetSortedPoints( vtkIdType id )
      {
      return this->LocatorIds[id];
      }

  protected:
    MyOctreeLocator()
      {
      }
    ~MyOctreeLocator()
      {
      }

  private:
    MyOctreeLocator(const MyOctreeLocator&);
    void operator =(const MyOctreeLocator&);
};

class PointsFunctor : public vtkFunctor
{
protected:
  PointsFunctor()
    {
    newPointsLayout = vtkPoints::New();
    oldToNew = vtkIdList::New();
    }
  ~PointsFunctor()
    {
    newPointsLayout->Delete();
    oldToNew->Delete();
    }

  vtkIdList* oldToNew;
  vtkPoints* newPointsLayout;

  MyOctreeLocator* locator;
  vtkPointData* newPd;
  vtkPointData* oldPd;

public:
  static PointsFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os, indent);
    }

  void Initialize( MyOctreeLocator* locator, vtkPointData* oldPointData, vtkPointData* newPointData )
    {
    vtkIdType num_points = locator->GetDataSet()->GetNumberOfPoints();
    this->oldPd = oldPointData;
    this->newPd = newPointData;

    this->newPointsLayout->Allocate( num_points );
    this->newPointsLayout->SetNumberOfPoints( num_points );
    this->oldToNew->Resize( num_points );
    this->locator = locator;
    }

  virtual void operator ()( vtkIdType i, vtkIdType vtkNotUsed(index1), vtkIdType vtkNotUsed(index2) ) const
    {
    double pt[3];
    vtkIdType id = locator->GetSortedPoints( i, pt );
    newPointsLayout->SetPoint( i, pt );
    oldToNew->SetId( id, i );
    newPd->SetTuple( i, id, oldPd );
    }

  vtkPoints* GetNewPoints()
    {
    return newPointsLayout;
    }

  vtkIdList* GetOldToNew()
    {
    return oldToNew;
    }

private:
  PointsFunctor(const PointsFunctor&);
  void operator =(const PointsFunctor&);
};

vtkStandardNewMacro(MyOctreeLocator);
vtkStandardNewMacro(vtkSMPZCurve);
vtkStandardNewMacro(PointsFunctor);

vtkSMPZCurve::vtkSMPZCurve()
  {
  }

vtkSMPZCurve::~vtkSMPZCurve()
  {
  }

int vtkSMPZCurve::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
  {
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPointSet *input = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet *output = vtkPointSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->DeepCopy( input );
  if ( !(input->IsA("vtkPolyData")) && !(input->IsA("vtkUnstructuredGrid")) )
    {
    return 1;
    }

  vtkPolyData* pointsContainer = vtkPolyData::New();
  pointsContainer->SetPoints( input->GetPoints() );

  MyOctreeLocator* locator = MyOctreeLocator::New();
  locator->SetMaximumPointsPerRegion(1);
  locator->SetMaxLevel(50);
//  locator->SetCreateCubicOctants( 0 );
  locator->SetDataSet( pointsContainer );
  locator->BuildLocator();

  PointsFunctor* pf = PointsFunctor::New();
  pf->Initialize( locator, input->GetPointData(), output->GetPointData() );
  vtkSMP::ForEach( 0, input->GetNumberOfPoints(), pf );
  output->SetPoints( pf->GetNewPoints() );

  vtkIdList* oldToNew = pf->GetOldToNew();
  oldToNew->Register(this);
  pf->Delete();

  vtkIdType numberOfCells = input->GetNumberOfCells();
  vtkGenericCell* c = vtkGenericCell::New();
  vtkPoints* cellBarycentrum = vtkPoints::New();
  cellBarycentrum->Allocate( numberOfCells );
  cellBarycentrum->SetNumberOfPoints( numberOfCells );
  for ( vtkIdType i = 0; i < numberOfCells; ++i )
    {
    double pt[3];
    input->GetCell( i, c );
    vtkIdType n = c->GetNumberOfPoints();
    input->GetPoint( c->GetPointId(0), pt );
    for ( vtkIdType j = 1; j < n; ++j )
      {
      double pt2[3];
      input->GetPoint( c->GetPointId(j), pt2 );
      pt[0] += pt2[0]; pt[1] += pt2[1]; pt[2] += pt2[2];
      }
    cellBarycentrum->SetPoint( i, pt[0]/n, pt[1]/n, pt[2]/n );
    }

  pointsContainer->SetPoints( cellBarycentrum );
  cellBarycentrum->Delete();

  locator->BuildLocator();

  vtkPolyData* polyOutput = vtkPolyData::SafeDownCast(output);
  vtkUnstructuredGrid* usgOutput = vtkUnstructuredGrid::SafeDownCast(output);
  if ( polyOutput )
    polyOutput->Allocate( numberOfCells );
  else
    usgOutput->Allocate( numberOfCells );
  output->GetCellData()->Allocate( numberOfCells );
  output->GetCellData()->SetNumberOfTuples( numberOfCells );
  for ( vtkIdType i = 0; i < numberOfCells; ++i )
    {
    vtkIdType newCellId = locator->GetSortedPoints( i );
    input->GetCell( newCellId, c );
    vtkIdType newPtIds[c->GetNumberOfPoints()];
    for ( vtkIdType j = 0; j < c->GetNumberOfPoints(); ++j )
      {
      newPtIds[j] = oldToNew->GetId( c->GetPointId(j) );
      }
    if (polyOutput)
      polyOutput->InsertNextCell( c->GetCellType(), c->GetNumberOfPoints(), newPtIds );
    else
      usgOutput->InsertNextCell( c->GetCellType(), c->GetNumberOfPoints(), newPtIds );
    output->GetCellData()->SetTuple( i, newCellId, input->GetCellData() );
    }
  oldToNew->UnRegister(this);
  c->Delete();

  return 1;
  }

void vtkSMPZCurve::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }
