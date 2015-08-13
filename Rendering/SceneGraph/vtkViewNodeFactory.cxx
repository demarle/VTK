/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkViewNodeFactory.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkViewNodeFactory.h"
#include "vtkObjectFactory.h"
#include "vtkViewNode.h"

#include <map>
#include <string>

//============================================================================
class vtkViewNodeFactory::vtkInternals
{
public:
  std::map<std::string, vtkViewNode *(*)()> Overrides;

  vtkInternals()
    {
    }

  ~vtkInternals()
    {
      this->Overrides.clear();
    }
};

//============================================================================
vtkStandardNewMacro(vtkViewNodeFactory);

//----------------------------------------------------------------------------
vtkViewNodeFactory::vtkViewNodeFactory()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Internals = new vtkInternals;
}

//----------------------------------------------------------------------------
vtkViewNodeFactory::~vtkViewNodeFactory()
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkViewNodeFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkViewNode *vtkViewNodeFactory::CreateNode(vtkObject *who)
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  const char *forwhom = who->GetClassName();
  vtkViewNode *ret = this->CreateNode(forwhom);
  ret->SetRenderable(who);
  return ret;
}

//----------------------------------------------------------------------------
vtkViewNode *vtkViewNodeFactory::CreateNode(const char *forwhom)
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  vtkViewNode *ret = NULL;
  if (this->Internals->Overrides.find(forwhom) == this->Internals->Overrides.end())
    {
    vtkWarningMacro("no override found for " << forwhom);
    ret = vtkViewNode::New();
    }
  else
    {
    vtkViewNode *(*func)() = this->Internals->Overrides.find(forwhom)->second;
    ret = func();
    }

  ret->SetMyFactory(this);
  return ret;
}

//----------------------------------------------------------------------------
void vtkViewNodeFactory::RegisterOverride
  (const char *name, vtkViewNode *(*func)())
{
  //cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
  this->Internals->Overrides[name] = func;
}
