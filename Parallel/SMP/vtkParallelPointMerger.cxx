#include "vtkParallelPointMerger.h"

#include "vtkDummyMergeFunctor.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSMP.h"
#include "vtkSMPMergePoints.h"
#include "vtkTask.h"

extern vtkIdType** TreatedTable;

int MustTreatBucket( vtkIdType idx )
  {
  if ( !TreatedTable ) return 0;
  vtkIdType one = 1;
  return !__sync_fetch_and_add(&(TreatedTable[idx]), &one);
  }

//------------------------------------------------------------------------------
vtkParallelPointMerger* vtkParallelPointMerger::New()
{
  return new vtkParallelPointMerger;
}

//------------------------------------------------------------------------------
void vtkParallelPointMerger::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//------------------------------------------------------------------------------
void vtkParallelPointMerger::Execute( vtkSMPMergePoints* locator ) const
{
  if ( !locator ) return;

  vtkIdType NumberOfBuckets = self->outputLocator->GetNumberOfBuckets();
  vtkSMPMergePoints* l;

  for ( vtkIdType i = 0; i < NumberOfBuckets; ++i )
    {
    if ( locator->GetNumberOfIdInBucket(i) )
      if ( MustTreatBucket(i) )
        {
        vtkThreadLocal<vtkPointData>::iterator itPd = self->InPd->Begin( 1 );
        vtkThreadLocal<vtkIdList>::iterator itMaps = self->Maps->Begin( 1 );
        for ( vtkThreadLocal<vtkSMPMergePoints>::iterator itLocator = self->Locators->Begin( 1 );
              itLocator != self->Locators->End(); ++itLocator, ++itPd, ++itMaps )
          if ( (l = *itLocator) )
            self->outputLocator->Merge( l, i, self->outputPd, *itPd, *itMaps );
        }
    }
}
