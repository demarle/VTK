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
#include "vtkMapper.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

//============================================================================
vtkStandardNewMacro(vtkActorNode);

//----------------------------------------------------------------------------
vtkActorNode::vtkActorNode()
{
  this->Visibility = false;
  this->Opacity = 1.0;
  this->Representation = VTK_POINTS;
  this->Lighting = true;
  this->Interpolation = VTK_FLAT;
  this->ScalarVisibility = false;
  this->Ambient = 0.0;
  this->AmbientColor[0] =
    this->AmbientColor[1] =
    this->AmbientColor[2] = 1.0;
  this->Diffuse = 1.0;
  this->DiffuseColor[0] =
    this->DiffuseColor[1] =
    this->DiffuseColor[2] = 1.0;
  this->Specular = 0.0;
  this->SpecularColor[0] =
    this->SpecularColor[1] =
    this->SpecularColor[2] = 1.0;
  this->SpecularPower = 0.0;
  //array color
  this->ScalarMode = VTK_SCALAR_MODE_DEFAULT;
  this->ColorMode = VTK_COLOR_MODE_DEFAULT;
  this->InterpolateScalarsBeforeMapping = false;
  this->ScalarRange[0] = 0.0;
  this->ScalarRange[1] = -1.0;
  this->UseLookupTableScalarRange = true;
  this->ScalarMaterialMode = VTK_MATERIALMODE_DEFAULT;

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

  /*
    //ACTOR
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
    //PROPERTY
    GetAmbient()        vtkProperty     virtual
    GetAmbientColor()   vtkProperty     virtual
    GetBackfaceCulling()        vtkProperty     virtual
    GetColor()  vtkProperty
    GetDiffuse()        vtkProperty     virtual
    GetDiffuseColor()   vtkProperty     virtual
    GetEdgeColor()      vtkProperty     virtual
    GetEdgeVisibility() vtkProperty     virtual
    GetFrontfaceCulling()       vtkProperty     virtual
    GetInterpolation()  vtkProperty     virtual
    GetLighting()       vtkProperty     virtual
    GetLineStipplePattern()     vtkProperty     virtual
    GetLineStippleRepeatFactor()        vtkProperty     virtual
    GetLineWidth()      vtkProperty     virtual
    GetMaterialName()   vtkProperty     virtual
    GetNumberOfTextures()       vtkProperty
    GetOpacity()        vtkProperty     virtual
    GetPointSize()      vtkProperty     virtual
    GetRepresentation() vtkProperty     virtual
    GetShaderDeviceAdapter2()   vtkProperty     inlinevirtual
    GetShading()        vtkProperty     virtual
    GetSpecular()       vtkProperty     virtual
    GetSpecularColor()  vtkProperty     virtual
    GetSpecularPower()  vtkProperty     virtual
    GetTexture(const char *name)        vtkProperty
    GetTexture(int unit)        vtkProperty
    GetTextureAtIndex(int index)        vtkProperty     protected
    GetTextureUnit(const char *name)    vtkProperty     protected
    GetTextureUnitAtIndex(int index)
    //MAPPER
    GetArrayAccessMode()        vtkMapper       inline
    GetArrayComponent() vtkMapper       inline
    GetArrayId()        vtkMapper       inline
    GetArrayName()      vtkMapper       inline
    GetClippingPlanes() vtkAbstractMapper       virtual
    GetColorMode()      vtkMapper       virtual
    GetFieldDataTupleId()       vtkMapper       virtual
    GetImmediateModeRendering() vtkMapper       virtual
    GetInformation()    vtkAlgorithm    virtual
    GetLookupTable()    vtkMapper
    GetNumberOfClippingPlanes() vtkAbstractMapper3D
    GetResolveCoincidentTopology()      vtkMapper       static
    GetResolveCoincidentTopologyPolygonOffsetFaces()    vtkMapper       static
    GetResolveCoincidentTopologyPolygonOffsetParameters(double &factor, double &units)  vtkMapper       static
    GetResolveCoincidentTopologyZShift()        vtkMapper       static
    GetScalarMaterialMode()     vtkMapper       virtual
    GetScalarMode()     vtkMapper       virtual
    GetScalarRange()    vtkMapper       virtual
    GetScalars(vtkDataSet *input, int scalarMode, int arrayAccessMode, int arrayId, const char *arrayName, int &cellFlag)       vtkAbstractMapper       static
    GetScalarVisibility()       vtkMapper       virtual
    GetStatic() vtkMapper       virtual
    GetSupportsSelection()      vtkMapper       inlinevirtual
    GetUpdateExtent()   vtkAlgorithm    inline
    GetUpdateGhostLevel()       vtkAlgorithm    inline
    GetUpdateNumberOfPieces()   vtkAlgorithm    inline
    GetUpdatePiece()    vtkAlgorithm    inline
    GetUseLookupTableScalarRange()
  */
  this->Visibility = mine->GetVisibility();
  //todo placement
  //todo texture
  this->Opacity = mine->GetProperty()->GetOpacity();
  this->Representation = mine->GetProperty()->GetRepresentation();
  this->Lighting = mine->GetProperty()->GetLighting();
  this->Interpolation = mine->GetProperty()->GetInterpolation();
  //actor or array color
  this->ScalarVisibility = mine->GetMapper()->GetScalarVisibility();
  //actor color
  this->Ambient = mine->GetProperty()->GetAmbient();
  mine->GetProperty()->GetAmbientColor(this->AmbientColor);
  this->Diffuse = mine->GetProperty()->GetDiffuse();
  mine->GetProperty()->GetDiffuseColor(this->DiffuseColor);
  this->Specular = mine->GetProperty()->GetSpecular();
  mine->GetProperty()->GetSpecularColor(this->SpecularColor);
  this->SpecularPower = mine->GetProperty()->GetSpecularPower();
  //array color
  this->ScalarMode = mine->GetMapper()->GetScalarMode(); //array location choice
  this->ColorMode = mine->GetMapper()->GetColorMode(); //direct or lut
  this->InterpolateScalarsBeforeMapping = mine->GetMapper()->GetInterpolateScalarsBeforeMapping();
  mine->GetMapper()->GetScalarRange(this->ScalarRange);
  this->UseLookupTableScalarRange = mine->GetMapper()->GetUseLookupTableScalarRange();
  this->ScalarMaterialMode = mine->GetMapper()->GetScalarMaterialMode();
  //array selection?
  //lookup table selection?

}
