/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkActorCollection.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkActorCollection - a list of actors
// .SECTION Description
// vtkActorCollection represents and provides methods to manipulate a list of
// actors (i.e., vtkActor and subclasses). The list is unsorted and duplicate
// entries are not prevented.

// .SECTION see also
// vtkActor vtkCollection 

#ifndef __vtkActorC_h
#define __vtkActorC_h

#include "vtkPropCollection.h"
#include "vtkActor.h"
class vtkProperty;

class VTK_EXPORT vtkActorCollection : public vtkPropCollection
{
public:
  static vtkActorCollection *New();
  vtkTypeMacro(vtkActorCollection,vtkPropCollection);

  // Description:
  // Add an actor to the list.
  void AddItem(vtkActor *a);

  // Description:
  // Get the next actor in the list.
  vtkActor *GetNextActor();

  // Description:
  // Get the last actor in the list.
  vtkActor *GetLastActor();

  // Description:
  // Access routines that are provided for compatibility with previous
  // version of VTK.  Please use the GetNextActor(), GetLastActor() variants
  // where possible.
  vtkActor *GetNextItem();
  vtkActor *GetLastItem();

  // Description:
  // Apply properties to all actors in this collection.
  void ApplyProperties(vtkProperty *p); 

protected:
  vtkActorCollection() {};
  ~vtkActorCollection() {};
  vtkActorCollection(const vtkActorCollection&) {};
  void operator=(const vtkActorCollection&) {};
    

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };
  void AddItem(vtkProp *o) { this->vtkPropCollection::AddItem(o); };

};

inline void vtkActorCollection::AddItem(vtkActor *a) 
{
  this->vtkCollection::AddItem((vtkObject *)a);
}

inline vtkActor *vtkActorCollection::GetNextActor() 
{ 
  return vtkActor::SafeDownCast(this->GetNextItemAsObject());
}

inline vtkActor *vtkActorCollection::GetLastActor() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return vtkActor::SafeDownCast(this->Bottom->Item);
    }
}

inline vtkActor *vtkActorCollection::GetNextItem() 
{ 
  return this->GetNextActor();
}

inline vtkActor *vtkActorCollection::GetLastItem() 
{
  return this->GetLastActor();
}

#endif





