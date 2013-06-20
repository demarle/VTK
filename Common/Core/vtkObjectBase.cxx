RegisterInternal(0, 0);
}

void vtkObjectBase::Print(ostream& os)
{
  vtkIndent indent;

  this->PrintHeader(os,vtkIndent(0));
  this->PrintSelf(os, indent.GetNextIndent());
  this->PrintTrailer(os,vtkIndent(0));
}

void vtkObjectBase::PrintHeader(ostream& os, vtkIndent indent)
{
  os << indent << this->GetClassName() << " (" << this << ")\n";
}

// Chaining method to print an object's instance variables, as well as
// its superclasses.
void vtkObjectBase::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "Reference Count: " << this->ReferenceCount << "\n";
}

void vtkObjectBase::PrintTrailer(ostream& os, vtkIndent indent)
{
  os << indent << "\n";
}

// Description:
// Sets the reference count (use with care)
void vtkObjectBase::SetReferenceCount(int ref)
{
  this->ReferenceCount = ref;
  vtkBaseDebugMacro(<< "Reference Count set to " << this->ReferenceCount);
}

//----------------------------------------------------------------------------
void vtkObjectBase::Register(vtkObjectBase* o)
{
  // Do not participate in garbage collection by default.
  this->RegisterInternal(o, 0);
}

//----------------------------------------------------------------------------
void vtkObjectBase::UnRegister(vtkObjectBase* o)
{
  // Do not participate in garbage collection by default.
  this->UnRegisterInternal(o, 0);
}

//----------------------------------------------------------------------------
void vtkObjectBase::RegisterInternal(vtkObjectBase*, int check)
{
  // If a reference is available from the garbage collector, use it.
  // Otherwise create a new reference by incrementing the reference
  // count.
  if(!(check &&
       vtkObjectBaseToGarbageCollectorFriendship::TakeReference(this)))
    {
    vtkIdType one = 1;
    __sync_add_and_fetch(&(this->ReferenceCount), &one);
    }
}

//----------------------------------------------------------------------------
void vtkObjectBase::UnRegisterInternal(vtkObjectBase*, int check)
{
  // If the garbage collector accepts a reference, do not decrement
  // the count.
  if(check && this->ReferenceCount > 1 &&
     vtkObjectBaseToGarbageCollectorFriendship::GiveReference(this))
    {
    return;
    }

  // Decrement the reference count, delete object if count goes to zero.
  vtkIdType minusOne = -1;
  if(__sync_add_and_fetch(&(this->ReferenceCount), &minusOne) <= 0)
    {
    // Clear all weak pointers to the object before deleting it.
    if (this->WeakPointers)
      {
      vtkWeakPointerBase **p = this->WeakPointers;
      while (*p)
        {
        vtkObjectBaseToWeakPointerBaseFriendship::ClearPointer(*p++);
        }
      delete [] this->WeakPointers;
      }
#ifdef VTK_DEBUG_LEAKS
    vtkDebugLeaks::DestructClass(this->GetClassName());
#endif
    delete this;
    }
  else if(check)
    {
    // The garbage collector did not accept the reference, but the
    // object still exists and is participating in garbage collection.
    // This means either that delayed garbage collection is disabled
    // or the collector has decided it is time to do a check.
    vtkGarbageCollector::Collect(this);
    }
}

//----------------------------------------------------------------------------
void vtkObjectBase::ReportReferences(vtkGarbageCollector*)
{
  // vtkObjectBase has no references to report.
}
