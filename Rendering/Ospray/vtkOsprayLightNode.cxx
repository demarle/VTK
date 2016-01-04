/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOsprayLightNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOsprayLightNode.h"

#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkLight.h"
#include "vtkViewNodeCollection.h"

#include "ospray/ospray.h"

//============================================================================
vtkStandardNewMacro(vtkOsprayLightNode);

//----------------------------------------------------------------------------
vtkOsprayLightNode::vtkOsprayLightNode()
{
}

//----------------------------------------------------------------------------
vtkOsprayLightNode::~vtkOsprayLightNode()
{
}

//----------------------------------------------------------------------------
void vtkOsprayLightNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkOsprayLightNode::ORender(void *renderer)
{
  OSPRenderer oRenderer = (OSPRenderer) renderer;

  std::vector<OSPLight> lights;

  OSPLight ospLight = ospNewLight(oRenderer, "AmbientLight");
  ospSetString(ospLight, "name", "ambient" );
  ospSet3f(ospLight, "color", 1,1,1);
  ospSet1f(ospLight, "intensity", 0.7);
  ospCommit(ospLight);
  lights.push_back(ospLight);

  if (this->Positional)
    {
    OSPLight ospLight = ospNewLight(oRenderer, "PointLight");
    ospSetString(ospLight, "name", "point" );
    ospSet3f(ospLight, "color",
             this->DiffuseColor[0],
             this->DiffuseColor[1],
             this->DiffuseColor[2]);
    ospSet3f(ospLight, "position",
             this->Position[0],
             this->Position[1],
             this->Position[2]);
    ospCommit(ospLight);
    lights.push_back(ospLight);
    }
  else
    {
    double direction[3];
    direction[0] = this->Position[0] - this->FocalPoint[0];
    direction[1] = this->Position[1] - this->FocalPoint[1];
    direction[2] = this->Position[2] - this->FocalPoint[2];
    OSPLight ospLight = ospNewLight(oRenderer, "DirectionalLight");
    ospSetString(ospLight, "name", "sun" );
    ospSet3f(ospLight, "color",
             this->DiffuseColor[0],
             this->DiffuseColor[1],
             this->DiffuseColor[2]);
    osp::vec3f dir(-direction[0],-direction[1],-direction[2]);
    dir = normalize(dir);
    ospSet3f(ospLight, "direction", dir.x,dir.y,dir.z);
    ospCommit(ospLight);
    lights.push_back(ospLight);
    }

  OSPData lightArray = ospNewData(lights.size(),
                                  OSP_OBJECT, &lights[0], 0);
  ospSetData(oRenderer, "lights", lightArray);

}
