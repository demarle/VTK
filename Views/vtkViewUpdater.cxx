/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkViewUpdater.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkViewUpdater.h"

#include "vtkAnnotationLink.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkView.h"
#include "vtkRenderView.h"

#include <vtksys/stl/vector>

vtkStandardNewMacro(vtkViewUpdater);

class vtkViewUpdater::vtkViewUpdaterInternals : public vtkCommand
{
public:

  virtual void Execute(vtkObject*, unsigned long, void*)
  {
    for (unsigned int i = 0; i < this->Views.size(); ++i)
      {
      vtkRenderView* rv = vtkRenderView::SafeDownCast(this->Views[i]);
      if (rv)
        {
        rv->Render();
        }
      else
        {
        this->Views[i]->Update();
        }
      }
  }

  vtksys_stl::vector<vtkView*> Views;
};

vtkViewUpdater::vtkViewUpdater()
{
  this->Internals = new vtkViewUpdaterInternals();
}

vtkViewUpdater::~vtkViewUpdater()
{
  this->Internals->Delete();
}

void vtkViewUpdater::AddView(vtkView* view)
{
  this->Internals->Views.push_back(view);
  //view->AddObserver(vtkCommand::SelectionChangedEvent, this->Internals);
}

void vtkViewUpdater::AddAnnotationLink(vtkAnnotationLink* link)
{
  link->AddObserver(vtkCommand::AnnotationChangedEvent, this->Internals);
}

void vtkViewUpdater::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
