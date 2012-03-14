#include "vtkSMPMergePoints.h"
#include "vtkPoints.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkMutexLock.h"

vtkStandardNewMacro(vtkSMPMergePoints)

vtkSMPMergePoints::vtkSMPMergePoints() : vtkMergePoints(), LockTable( 0 )
  {
  this->CreatorLock = 0;
  }

vtkSMPMergePoints::~vtkSMPMergePoints()
  {
  if ( this->CreatorLock )
    this->CreatorLock->Delete();
  }

void vtkSMPMergePoints::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }

int vtkSMPMergePoints::InitLockInsertion(vtkPoints *newPts, const double bounds[], vtkIdType estSize)
  {
  if ( !this->InitPointInsertion(newPts, bounds, estSize) )
    return 0;
  this->LockTable = new vtkMutexLockPtr[this->NumberOfBuckets];
  memset( this->LockTable, 0, this->NumberOfBuckets * sizeof(vtkMutexLockPtr) );
  this->InsertionPointId = newPts->GetNumberOfPoints();
  if ( !this->CreatorLock ) this->CreatorLock = vtkMutexLock::New();
  return 1;
  }

void vtkSMPMergePoints::FixSizeOfPointArray()
  {
  this->Points->SetNumberOfPoints(this->InsertionPointId);
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
  vtkMutexLock *lock;
  vtkIdType i;

  if ( this->HashTable )
    {
    for (i=0; i<this->NumberOfBuckets; i++)
      {
      if ( (ptIds = this->HashTable[i]) )
        {
        ptIds->Delete();
        }
      if ( this->LockTable && (lock = this->LockTable[i]) )
        {
        lock->Delete();
        }
      }
    delete [] this->HashTable;
    if ( this->LockTable ) delete [] this->LockTable;
    this->LockTable = NULL;
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
