/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkActorNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkActorNode.h"

#include "vtkActor.h"
#include "vtkObjectFactory.h"

//============================================================================
vtkStandardNewMacro(vtkActorNode);

//----------------------------------------------------------------------------
vtkActorNode::vtkActorNode()
{
  this->Visibility = false;
}

//----------------------------------------------------------------------------
vtkActorNode::~vtkActorNode()
{
}

//----------------------------------------------------------------------------
void vtkActorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkActorNode::SynchronizeSelf()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkActor *mine = vtkActor::SafeDownCast
    (this->GetRenderable());
  if (!mine)
    {
    return;
    }

  //TODO: get state from our renderable
  /*
    GetBackfaceProperty()       vtkActor        virtual
    GetBounds() vtkActor        virtual
    GetCenter() vtkProp3D
    GetConsumer(int i)  vtkProp
    GetDragable()       vtkProp virtual
    GetEstimatedRenderTime()    vtkProp inlinevirtual
    GetIsIdentity()     vtkProp3D       virtual
    GetIsOpaque()       vtkActor        protected
    GetLength() vtkProp3D
    GetMapper() vtkActor        virtual
    GetMatrix() vtkProp3D       inlinevirtual
    GetNextPath()       vtkProp virtual
    GetNumberOfConsumers()      vtkProp virtual
    GetNumberOfPaths()  vtkProp inlinevirtual
    GetOrientation()    vtkProp3D
    GetOrigin() vtkProp3D       virtual
    GetPickable()       vtkProp virtual
    GetPosition()       vtkProp3D       virtual
    GetProperty()       vtkActor
    GetPropertyKeys()   vtkProp virtual
    GetRenderTimeMultiplier()   vtkProp virtual
    GetScale()  vtkProp3D       virtual
    GetSupportsSelection()      vtkActor        virtual
    GetTexture()        vtkActor        virtual
    GetUseBounds()      vtkProp virtual
    GetUserMatrix()     vtkProp3D
    GetUserTransform()  vtkProp3D       virtual
    GetVisibility()     vtkProp virtual
    GetVolumes(vtkPropCollection *)     vtkProp inlinevirtual
    GetXRange() vtkProp3D
    GetYRange() vtkProp3D
    GetZRange() vtkProp3D
  */
  this->Visibility = mine->GetVisibility();

}
