#include "vtkSMPMergePoints.h"
#include "vtkPoints.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkSMPMergePoints)

vtkSMPMergePoints::vtkSMPMergePoints() : vtkMergePoints(), TreatedTable( 0 )
{
}

vtkSMPMergePoints::~vtkSMPMergePoints()
{
}

void vtkSMPMergePoints::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkSMPMergePoints::InitPointInsertion(vtkPoints *newPts, const double bounds[], vtkIdType estSize)
{
  if (newPts)
  {
    if ( !this->Superclass::InitPointInsertion(newPts, bounds, estSize) )
      return 0;
    this->TreatedTable = new vtkIdType[this->NumberOfBuckets];
    memset( this->TreatedTable, 0, this->NumberOfBuckets * sizeof(vtkIdType) );
    return 1;
  }

  if ( estSize > 0 )
  {
    this->Points->GetData()->Resize( estSize );
    this->Points->SetNumberOfPoints( estSize );
  }
  else
  {
    this->Points->SetNumberOfPoints(this->InsertionPointId);
  }
  return 1;
}

void vtkSMPMergePoints::Merge( vtkSMPMergePoints* locator, vtkIdType idx, vtkPointData* outPd, vtkPointData* ptData, vtkIdList* idList )
  {
  if ( !locator->HashTable[idx] ) return;

  vtkIdType i;
  vtkIdList *bucket, *oldIdToMerge;
  vtkFloatArray* floatOldDataArray = 0;

  if ( !(bucket = this->HashTable[idx]) )
    {
    this->HashTable[idx] = bucket = vtkIdList::New();
    bucket->Allocate( this->NumberOfPointsPerBucket/2, this->NumberOfPointsPerBucket/3 );
    oldIdToMerge = locator->HashTable[idx];
    oldIdToMerge->Register( this );
    if ( this->Points->GetData()->GetDataType() == VTK_FLOAT )
      floatOldDataArray = static_cast<vtkFloatArray*>( locator->Points->GetData() );
    }
  else
    {
    oldIdToMerge = vtkIdList::New();
    oldIdToMerge->Register( this );
    oldIdToMerge->Delete();
  
    int nbOfIds = bucket->GetNumberOfIds (), nbOfOldIds = locator->HashTable[idx]->GetNumberOfIds();
    oldIdToMerge->Allocate( nbOfOldIds );

    vtkDataArray *dataArray = this->Points->GetData(), *oldDataArray = locator->Points->GetData();
    vtkIdType *idArray = bucket->GetPointer(0), *idOldArray = locator->HashTable[idx]->GetPointer(0);

    bool found;
    
    if (dataArray->GetDataType() == VTK_FLOAT)
      {
      vtkFloatArray* floatDataArray = static_cast<vtkFloatArray*>(dataArray);
      floatOldDataArray = static_cast<vtkFloatArray*>(oldDataArray);

      for ( int oldIdIdx = 0; oldIdIdx < nbOfOldIds; ++oldIdIdx )
        {
        found = false;
        vtkIdType oldId = idOldArray[oldIdIdx];
        float *x = floatOldDataArray->GetPointer(0) + 3*oldId;
        float *pt;
        for ( i=0; i < nbOfIds; i++ )
          {
          vtkIdType existingId = idArray[i];
          pt = floatDataArray->GetPointer(0) + 3*existingId;
          if ( x[0] == pt[0] && x[1] == pt[1] && x[2] == pt[2] )
            {
            // point is already in the list, return 0 and set the id parameter
            found = true;
            idList->SetId( oldId, existingId );
            break;
            }
          }
        if ( !found ) oldIdToMerge->InsertNextId( oldId );
        }
      }
    else
      {
      for ( int oldIdIdx = 0; oldIdIdx < nbOfOldIds; ++oldIdIdx )
        {
        found = false;
        vtkIdType oldId = idOldArray[oldIdIdx];
        double *x = oldDataArray->GetTuple( oldId );
        double *pt;
        for ( i=0; i < nbOfIds; i++ )
          {
          vtkIdType existingId = idArray[i];
          pt = dataArray->GetTuple( existingId );
          if ( x[0] == pt[0] && x[1] == pt[1] && x[2] == pt[2] )
            {
            // point is already in the list, return 0 and set the id parameter
            found = true;
            idList->SetId( oldId, existingId );
            break;
            }
          }
        if ( !found ) oldIdToMerge->InsertNextId( oldId );
        }
      }
    }

  // points have to be added
  vtkIdType NumberOfInsertions = oldIdToMerge->GetNumberOfIds();
  vtkIdType first_id = __sync_fetch_and_add( &(this->InsertionPointId), NumberOfInsertions );
  bucket->Resize( bucket->GetNumberOfIds() + NumberOfInsertions );
  for ( i = 0; i < NumberOfInsertions; ++i )
    {
    vtkIdType newId = first_id + i, oldId = oldIdToMerge->GetId( i );
    idList->SetId( oldId, newId );
    bucket->InsertNextId( newId );
    if ( floatOldDataArray )
      {
      const float *pt = floatOldDataArray->GetPointer(0) + 3*oldId;
      this->Points->SetPoint( newId, pt );
      }
    else
      {
      this->Points->SetPoint( newId, locator->Points->GetPoint( oldId ) );
      }
    outPd->SetTuple( newId, oldId, ptData );
    }
  oldIdToMerge->UnRegister( this );
  }

void vtkSMPMergePoints::FreeSearchStructure()
{
  this->Superclass::FreeSearchStructure();
  vtkIdList *ptIds;
  vtkIdType i;

  if ( this->HashTable ) // So this->LockTable also exist
    {
    for (i=0; i<this->NumberOfBuckets; i++)
      {
      if ( (ptIds = this->HashTable[i]) )
        {
        ptIds->Delete();
        }
      }
    delete [] this->HashTable;
    delete [] this->TreatedTable;
    this->TreatedTable = NULL;
    this->HashTable = NULL;
    }
}

vtkIdType vtkSMPMergePoints::GetNumberOfBuckets()
  {
  return this->NumberOfBuckets;
  }

vtkIdType vtkSMPMergePoints::GetNumberOfIdInBucket( vtkIdType idx )
  {
  if ( !this->HashTable ) return 0;
  vtkIdList* bucket = this->HashTable[idx];
  return bucket ? bucket->GetNumberOfIds() : 0;
  }

int vtkSMPMergePoints::MustTreatBucket( vtkIdType idx )
  {
  if ( !this->TreatedTable ) return 0;
  return !__sync_fetch_and_add(&(this->TreatedTable[idx]), 1);
  }
