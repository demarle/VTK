/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAssembly.cxx
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
#include "vtkAssembly.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkAssembly* vtkAssembly::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkAssembly");
  if(ret)
    {
    return (vtkAssembly*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkAssembly;
}




// Construct object with no children.
vtkAssembly::vtkAssembly()
{
  this->Paths = NULL;
  this->Parts = vtkActorCollection::New();
}

vtkAssembly::~vtkAssembly()
{
  this->DeletePaths();
  this->Parts->Delete();
  this->Parts = NULL;
}

// Add a part to the list of Parts.
void vtkAssembly::AddPart(vtkActor *actor)
{
  if ( ! this->Parts->IsItemPresent(actor) )
    {
    this->Parts->AddItem(actor);
    this->Modified();
    } 
}

// Remove a part from the list of parts,
void vtkAssembly::RemovePart(vtkActor *actor)
{
  if ( this->Parts->IsItemPresent(actor) )
    {
    this->Parts->RemoveItem(actor);
    this->Modified();
    } 
}

// Shallow copy another assembly.
void vtkAssembly::ShallowCopy(vtkAssembly *assembly)
{
  this->vtkActor::ShallowCopy(assembly);
  
  this->Parts->RemoveAllItems();
  assembly->Parts->InitTraversal();
  for (int i=0; i<0; i++)
    {
    this->Parts->AddItem(assembly->Parts->GetNextActor());
    }
  
  this->DeletePaths();
}

// Render this assembly and all its Parts. The rendering process is recursive.
// Note that a mapper need not be defined. If not defined, then no geometry 
// will be drawn for this assembly. This allows you to create "logical"
// assemblies; that is, assemblies that only serve to group and transform
// its Parts.
int vtkAssembly::RenderTranslucentGeometry(vtkViewport *ren)
{
  vtkActor *actor;
  vtkActorCollection *path;
  float fraction;
  int   renderedSomething = 0;

  this->UpdatePaths();

  // for allocating render time between components
  // simple equal allocation
  fraction = this->AllocatedRenderTime 
    / (float)(this->Paths->GetNumberOfItems());
  
  // render the Paths
  for ( this->Paths->InitTraversal(); (path = this->Paths->GetNextItem()); )
    {
    actor = path->GetLastActor();
    if ( actor->GetVisibility() )
      {
      actor->SetAllocatedRenderTime(fraction);
      renderedSomething += actor->RenderTranslucentGeometry(ren);
      }
    }

  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}


// Render this assembly and all its Parts. The rendering process is recursive.
// Note that a mapper need not be defined. If not defined, then no geometry 
// will be drawn for this assembly. This allows you to create "logical"
// assemblies; that is, assemblies that only serve to group and transform
// its Parts.
int vtkAssembly::RenderOpaqueGeometry(vtkViewport *ren)
{
  vtkActor *actor;
  vtkActorCollection *path;
  float fraction;
  int   renderedSomething = 0;

  this->UpdatePaths();

  // for allocating render time between components
  // simple equal allocation
  fraction = this->AllocatedRenderTime 
    / (float)(this->Paths->GetNumberOfItems());
  
  // render the Paths
  for ( this->Paths->InitTraversal(); (path = this->Paths->GetNextItem()); )
    {
    actor = path->GetLastActor();
    if ( actor->GetVisibility() )
      {
      actor->SetAllocatedRenderTime(fraction);
      renderedSomething += actor->RenderOpaqueGeometry(ren);
      }
    }

  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}

void vtkAssembly::ReleaseGraphicsResources(vtkWindow *renWin)
{
  vtkActor *actor;

  vtkActor::ReleaseGraphicsResources(renWin);

  // broadcast the message down the Paths
  for ( this->InitPartTraversal(); (actor  = this->GetNextPart()); )
    {
    actor->ReleaseGraphicsResources(renWin);
    }
}

void vtkAssembly::InitPartTraversal()
{
  this->UpdatePaths();
  this->Paths->InitTraversal();
}

// Return the next part in the hierarchy of assembly Parts.  This method 
// returns a properly transformed and updated actor.
vtkActor *vtkAssembly::GetNextPart()
{
  vtkActorCollection *path;

  if ( (path = this->Paths->GetNextItem()) == NULL ) 
    {
    return NULL;
    }
  else
    {
    return path->GetLastActor();
    }
}

int vtkAssembly::GetNumberOfParts()
{
  this->UpdatePaths();
  return this->Paths->GetNumberOfItems();
}

// Build the assembly paths if necessary.
void vtkAssembly::UpdatePaths()
{
  if ( this->GetMTime() > this->PathTime )
    {
    this->DeletePaths();
    }

  if ( ! this->Paths )
    {
    this->Paths = vtkAssemblyPaths::New();
    vtkActorCollection *path = vtkActorCollection::New();
    this->Paths->AddItem(path);

    this->BuildPaths(this->Paths,path);
    this->PathTime.Modified();
    }
}

// Build assembly paths from this current assembly. Paths consist of
// an ordered sequence of actors, with transformations properly concatenated.
void vtkAssembly::BuildPaths(vtkAssemblyPaths *paths, vtkActorCollection *path)
{
  vtkActor *part, *actor, *copy=vtkActor::New(), *previous;
  vtkMatrix4x4 *matrix;
  vtkActorCollection *childPath;

  copy->ShallowCopy(this);

  if ( path->GetNumberOfItems() < 1 ) //we're starting at the top of the hierarchy
    {
    if ( this->GetUserMatrix() )
      {
      matrix = vtkMatrix4x4::New();
      matrix->DeepCopy(this->GetUserMatrix());
      copy->SetUserMatrix(matrix);
      matrix->Delete();
      }
    }
  else //somewhere in the middle of the assembly hierarchy
    {
    previous = path->GetLastActor();
    matrix = vtkMatrix4x4::New();
    previous->GetMatrix(matrix);
    copy->SetUserMatrix(matrix);
    matrix->Delete();
    }

  path->AddItem(copy);

  // add our children to the path (if we're visible). 
  if ( this->Visibility )
    {
    for ( this->Parts->InitTraversal(); (part = this->Parts->GetNextActor()); )
      {
      //a new path is created for each child
      childPath = vtkActorCollection::New();
      paths->AddItem(childPath);

      // copy incoming path
      for ( path->InitTraversal(); (actor = path->GetNextActor()); )
        {
        copy = vtkActor::New();
        copy->ShallowCopy(actor);

        if ( actor->GetUserMatrix() )
          {
          matrix = vtkMatrix4x4::New();
          matrix->DeepCopy(actor->GetUserMatrix());
          copy->SetUserMatrix(matrix);
          matrix->Delete();
          }

        childPath->AddItem(copy);
        }

      // create new paths
      part->BuildPaths(paths,childPath);
      }
    }//if visible
}

// Delete the paths asscoiated with this assembly hierarchy.
void vtkAssembly::DeletePaths()
{
  if ( this->Paths )
    {
    vtkActor *actor;
    vtkActorCollection *path;

    for ( this->Paths->InitTraversal(); (path = this->Paths->GetNextItem()); )
      {
      for ( path->InitTraversal(); (actor = path->GetNextActor()); )
        {
        actor->Delete();
        }
      path->Delete();
      }
    this->Paths->Delete();
    this->Paths = NULL;
    }
}

// Recursive application of properties to all parts composing assembly.
void vtkAssembly::ApplyProperties()
{
  vtkActor *part;
  vtkProperty *prop=this->GetProperty();
  vtkProperty *actorProp;

  for (this->Parts->InitTraversal(); (part = this->Parts->GetNextActor()); )
    {
    actorProp = part->GetProperty();
    actorProp->DeepCopy(prop);
    part->ApplyProperties();
    }
}

// Get the bounds for the assembly as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
float *vtkAssembly::GetBounds()
{
  vtkActor *actor;
  vtkMapper *mapper;
  vtkActorCollection *path;
  int i,n;
  float *bounds, bbox[24], *fptr;
  float *result;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  int actorVisible=0;

  this->UpdatePaths();

  // now calculate the new bounds
  if ( this->Mapper ) 
    {
    vtkActor::GetBounds();
    }    
  else
    {
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = VTK_LARGE_FLOAT;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_LARGE_FLOAT;
    }

  this->Transform->PostMultiply();  
  for ( this->Paths->InitTraversal(); (path = this->Paths->GetNextItem()); )
    {
    actor = path->GetLastActor();

    if ( actor->GetVisibility() && (mapper = actor->GetMapper()) )
      {
      actorVisible = 1;
      bounds = mapper->GetBounds();

      // fill out vertices of a bounding box
      bbox[ 0] = bounds[1]; bbox[ 1] = bounds[3]; bbox[ 2] = bounds[5];
      bbox[ 3] = bounds[1]; bbox[ 4] = bounds[2]; bbox[ 5] = bounds[5];
      bbox[ 6] = bounds[0]; bbox[ 7] = bounds[2]; bbox[ 8] = bounds[5];
      bbox[ 9] = bounds[0]; bbox[10] = bounds[3]; bbox[11] = bounds[5];
      bbox[12] = bounds[1]; bbox[13] = bounds[3]; bbox[14] = bounds[4];
      bbox[15] = bounds[1]; bbox[16] = bounds[2]; bbox[17] = bounds[4];
      bbox[18] = bounds[0]; bbox[19] = bounds[2]; bbox[20] = bounds[4];
      bbox[21] = bounds[0]; bbox[22] = bounds[3]; bbox[23] = bounds[4];

      // save the old transform
      actor->GetMatrix(matrix);
      this->Transform->Push(); 
      this->Transform->Identity();
      this->Transform->Concatenate(matrix);

      // and transform into actors coordinates
      fptr = bbox;
      for (n = 0; n < 8; n++) 
        {
        this->Transform->SetPoint(fptr[0],fptr[1],fptr[2],1.0);

        // now store the result
        result = this->Transform->GetPoint();
        fptr[0] = result[0] / result[3];
        fptr[1] = result[1] / result[3];
        fptr[2] = result[2] / result[3];
        fptr += 3;
        }

      this->Transform->Pop();  

      for (i = 0; i < 8; i++)
        {
        for (n = 0; n < 3; n++)
          {
          if (bbox[i*3+n] < this->Bounds[n*2])
            {
            this->Bounds[n*2] = bbox[i*3+n];
            }
          if (bbox[i*3+n] > this->Bounds[n*2+1])
            {
            this->Bounds[n*2+1] = bbox[i*3+n];
            }
          }
        }//for each point of box
      }//if mapper
    }//for each path

  this->Transform->PreMultiply();  

  if ( ! actorVisible )
    {
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -1.0;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] =  1.0;
    }

  matrix->Delete();
  return this->Bounds;
}

unsigned long int vtkAssembly::GetMTime()
{
  unsigned long mTime=this->vtkActor::GetMTime();
  unsigned long time;
  vtkActor *part;

  for (this->Parts->InitTraversal(); (part = this->Parts->GetNextActor()); )
    {
    time = part->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

void vtkAssembly::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkActor::PrintSelf(os,indent);

  os << indent << "There are: " << this->Parts->GetNumberOfItems()
     << " parts in this assembly\n";
}

