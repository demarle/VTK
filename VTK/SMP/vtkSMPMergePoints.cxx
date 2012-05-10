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

int vtkSMPMergePoints::InitLockInsertion(vtkPoints *newPts, const double bounds[6], vtkIdType estSize)
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

vtkIdType vtkSMPMergePoints::LocateBucketThatPointIsIn(double x, double y, double z)
  {
  vtkIdType ijk0, ijk1, ijk2;

  ijk0 = static_cast<vtkIdType>(
    static_cast<double> ((x - this->Bounds[0]) /
                         (this->Bounds[1] - this->Bounds[0]))
    * (this->Divisions[0] - 1));
  ijk1 = static_cast<vtkIdType>(
    static_cast<double> ((y - this->Bounds[2]) /
                         (this->Bounds[3] - this->Bounds[2]))
    * (this->Divisions[1] - 1));
  ijk2 = static_cast<vtkIdType>(
    static_cast<double> ((z - this->Bounds[4]) /
                         (this->Bounds[5] - this->Bounds[4]))
    * (this->Divisions[2] - 1));

  return ijk0 + ijk1*this->Divisions[0] + ijk2*this->Divisions[0]*this->Divisions[1];
  }

void vtkSMPMergePoints::AddPointIdInBucket(vtkIdType ptId)
  {
  double x[3];
  this->Points->GetPoint( ptId, x );

  vtkIdType idx = LocateBucketThatPointIsIn( x[0], x[1], x[2] );

  vtkIdList* bucket;
  vtkMutexLock* lock;
  if ( !(bucket = this->HashTable[idx]) )
    {
    this->CreatorLock->Lock();
    if ( !(lock = this->LockTable[idx]) )
      {
      this->LockTable[idx] = lock = vtkMutexLock::New();
      bucket = vtkIdList::New();
      bucket->Allocate(this->NumberOfPointsPerBucket/2, this->NumberOfPointsPerBucket/3);
      this->HashTable[idx] = bucket;
      }
    this->CreatorLock->Unlock();
    bucket = this->HashTable[idx];
    }
  else
    {
    lock = this->LockTable[idx];
    }

  lock->Lock();
  bucket->InsertNextId( ptId );
  lock->Unlock();
  }

int vtkSMPMergePoints::SetUniquePoint(const double x[3], vtkIdType &id)
  {
  vtkIdType i, idx = LocateBucketThatPointIsIn( x[0], x[1], x[2] );
  vtkIdList *bucket;

  vtkMutexLock* lock = this->LockTable[idx];
  if ( !lock )
    {
    CreatorLock->Lock();
    if ( !(lock = this->LockTable[idx]) ) // If no other thread created it before
      {
      this->HashTable[idx] = vtkIdList::New();
      this->HashTable[idx]->Allocate(this->NumberOfPointsPerBucket/2, this->NumberOfPointsPerBucket/3);
      this->LockTable[idx] = lock = vtkMutexLock::New();;
      }
    CreatorLock->Unlock();
    }

  bucket = this->HashTable[idx];

  //
  // Check the list of points in that bucket.
  //
  vtkIdType ptId, maxId = this->Points->GetNumberOfPoints();
  int nbOfIds = bucket->GetNumberOfIds();

  // For efficiency reasons, we break the data abstraction for points
  // and ids (we are assuming vtkPoints stores a vtkIdList
  // is storing ints).
  vtkDataArray *dataArray = this->Points->GetData();
  vtkIdType *idArray = bucket->GetPointer(0);

  if (dataArray->GetDataType() == VTK_FLOAT)
    {
    // Using the float interface
    float f[3];
    f[0] = static_cast<float>(x[0]);
    f[1] = static_cast<float>(x[1]);
    f[2] = static_cast<float>(x[2]);
    vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
    float *pt;
    for (i=0; i < nbOfIds; ++i)
      {
      ptId = idArray[i];
      if ( ptId > maxId ) break;
      pt = floatArray->GetPointer(0) + 3*ptId;
      if ( f[0] == pt[0] && f[1] == pt[1] && f[2] == pt[2] )
        {
        // point is already in the list, return 0 and set the id parameter
        id = ptId;
        return 0;
        }
      }

    // point has to be added
    lock->Lock();
    if ( bucket->GetNumberOfIds() != nbOfIds )
      {
      // Check again
      idArray = bucket->GetPointer(0);
      for ( ; i < bucket->GetNumberOfIds(); ++i)
        {
        ptId = idArray[i];
        pt = floatArray->GetPointer(0) + 3*ptId;
        if ( f[0] == pt[0] && f[1] == pt[1] && f[2] == pt[2] )
          {
          // point is already in the list, return 0 and set the id parameter
          id = ptId;
          lock->Unlock();
          return 0;
          }
        }
      }
    id = __sync_fetch_and_add(&(this->InsertionPointId), 1);
    bucket->InsertNextId(id);
    lock->Unlock();
    }
  else
    {
    // Using the double interface
    double *pt;
    for (i=0; i < nbOfIds; ++i)
      {
      ptId = idArray[i];
      if ( ptId > maxId ) break;
      pt = dataArray->GetTuple(ptId);
      if ( x[0] == pt[0] && x[1] == pt[1] && x[2] == pt[2] )
        {
        // point is already in the list, return 0 and set the id parameter
        id = ptId;
        return 0;
        }
      }

    // point has to be added
    lock->Lock();
    if ( bucket->GetNumberOfIds() != nbOfIds )
      {
      // Check again
      idArray = bucket->GetPointer(0);
      for ( ; i < bucket->GetNumberOfIds(); ++i )
        {
        ptId = idArray[i];
        pt = dataArray->GetTuple(ptId);
        if ( x[0] == pt[0] && x[1] == pt[1] && x[2] == pt[2] )
          {
          // point is already in the list, return 0 and set the id parameter
          id = ptId;
          lock->Unlock();
          return 0;
          }
        }
      }
    id = __sync_fetch_and_add(&(this->InsertionPointId), 1);
    bucket->InsertNextId(id);
    lock->Unlock();
    }

  this->Points->SetPoint( id, x );

  return 1;
  }

void vtkSMPMergePoints::PrintSizeOfThis()
  {
  cout << "this: " << sizeof(*this) << endl;
  cout << "doubles et int: " << 9 * sizeof(double) + 3 * sizeof(int) << endl;
  cout << "number of buckets: " << this->NumberOfBuckets << " (" << sizeof(vtkIdList*) << ")" << endl;
  cout << "vtkMutexLock: " << sizeof(*(this->CreatorLock)) << endl;
  int numOfEffectiveBuckets = 0;
  for (int i = 0; i < this->NumberOfBuckets; ++i)
    if (this->HashTable[i])
      ++numOfEffectiveBuckets;
  cout << "number of vtkIdList and vtkMutexLock: " << numOfEffectiveBuckets << endl;
  }
