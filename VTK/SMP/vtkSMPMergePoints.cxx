#include "vtkSMPMergePoints.h"
#include "vtkPoints.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkMutexLock.h"

vtkStandardNewMacro(vtkSMPMergePoints)

vtkSMPMergePoints::vtkSMPMergePoints() : vtkMergePoints()
{
  Lock = vtkMutexLock::New();
  Lock->Register(this);
  Lock->Delete();
}

vtkSMPMergePoints::~vtkSMPMergePoints()
{
  Lock->UnRegister(this);
}

void vtkSMPMergePoints::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkSMPMergePoints::InitPointInsertion(vtkPoints *newPts, const double bounds[], vtkIdType estSize)
{
  if (newPts)
  {
    return this->Superclass::InitPointInsertion(newPts, bounds, estSize);
  }

  if ( estSize > 0 )
  {
    for ( vtkIdType i = 0; i < this->NumberOfBuckets; ++i )
    {
      if ( !this->HashTable[i] )
      {
        vtkIdList* bucket = vtkIdList::New();
        bucket->Allocate( estSize );
        this->HashTable[i] = bucket;
      }
      else
      {
        this->HashTable[i]->Resize( estSize );
      }
    }
    this->Points->GetData()->Resize( estSize );
    this->Points->SetNumberOfPoints( estSize );
  }
  else
  {
    this->Points->SetNumberOfPoints(this->InsertionPointId);
  }
  return 1;
}

int vtkSMPMergePoints::SetUniquePoint(const double x[], vtkIdType &id)
{
  vtkIdType i, ijk0, ijk1, ijk2;
  vtkIdType idx;
  vtkIdList *bucket;

  //
  //  Locate bucket that point is in.
  //
  ijk0 = static_cast<vtkIdType>(
    static_cast<double> ((x[0] - this->Bounds[0]) /
                         (this->Bounds[1] - this->Bounds[0]))
    * (this->Divisions[0] - 1));
  ijk1 = static_cast<vtkIdType>(
    static_cast<double> ((x[1] - this->Bounds[2]) /
                         (this->Bounds[3] - this->Bounds[2]))
    * (this->Divisions[1] - 1));
  ijk2 = static_cast<vtkIdType>(
    static_cast<double> ((x[2] - this->Bounds[4]) /
                         (this->Bounds[5] - this->Bounds[4]))
    * (this->Divisions[2] - 1));

  idx = ijk0 + ijk1*this->Divisions[0] +
        ijk2*this->Divisions[0]*this->Divisions[1];

  bucket = this->HashTable[idx];

  if (bucket) // see whether we've got duplicate point
    {
    //
    // Check the list of points in that bucket.
    //
    vtkIdType ptId;
    int nbOfIds = bucket->GetNumberOfIds ();

    // For efficiency reasons, we break the data abstraction for points
    // and ids (we are assuming vtkPoints stores a vtkIdList
    // is storing ints).
    vtkDataArray *dataArray = this->Points->GetData();
    vtkIdType *idArray = bucket->GetPointer(0);

    if (dataArray->GetDataType() == VTK_FLOAT)
      {
      float f[3];
      f[0] = static_cast<float>(x[0]);
      f[1] = static_cast<float>(x[1]);
      f[2] = static_cast<float>(x[2]);
      vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
      float *pt;
      for (i=0; i < nbOfIds; i++)
        {
        ptId = idArray[i];
        pt = floatArray->GetPointer(0) + 3*ptId;
        if ( f[0] == pt[0] && f[1] == pt[1] && f[2] == pt[2] )
          {
          // point is already in the list, return 0 and set the id parameter
          id = ptId;
          return 0;
          }
        }
      }
    else
      {
      // Using the double interface
      double *pt;
      for (i=0; i < nbOfIds; i++)
        {
        ptId = idArray[i];
        pt = dataArray->GetTuple(ptId);
        if ( x[0] == pt[0] && x[1] == pt[1] && x[2] == pt[2] )
          {
          // point is already in the list, return 0 and set the id parameter
          id = ptId;
          return 0;
          }
        }
      }
    }
  else
    {
    vtkErrorMacro(<<"InitPointInsertion should have been called before SetUniquePoint");
    bucket = vtkIdList::New();
    bucket->Allocate(this->NumberOfPointsPerBucket/2,
                     this->NumberOfPointsPerBucket/3);
    this->HashTable[idx] = bucket;
    }

  // point has to be added
  this->Lock->Lock();
  id = this->InsertionPointId++;
  bucket->InsertNextId(id);
  this->Lock->Unlock();
  this->Points->SetPoint(id,x);

  return 1;
}
