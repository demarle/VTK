/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageReader2Factory.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageReader2Factory.h"

#include "vtkBMPReader.h"
#include "vtkGESignaReader.h"
#include "vtkImageReader2.h"
#include "vtkImageReader2Collection.h"
#include "vtkJPEGReader.h"
#include "vtkObjectFactory.h"
#include "vtkObjectFactoryCollection.h"
#include "vtkPNGReader.h"
#include "vtkPNMReader.h"
#include "vtkSLCReader.h"
#include "vtkTIFFReader.h"

vtkCxxRevisionMacro(vtkImageReader2Factory, "1.14");
vtkStandardNewMacro(vtkImageReader2Factory);

class vtkCleanUpImageReader2Factory
{
public:
  inline void Use() 
    {
    }
  ~vtkCleanUpImageReader2Factory()
    {
      if(vtkImageReader2Factory::AvailiableReaders)
        {
        vtkImageReader2Factory::AvailiableReaders->Delete();
        vtkImageReader2Factory::AvailiableReaders = 0;
        }
    }  
};
static vtkCleanUpImageReader2Factory vtkCleanUpImageReader2FactoryGlobal;

vtkImageReader2Collection* vtkImageReader2Factory::AvailiableReaders;

void vtkImageReader2Factory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Availiable Readers : ";
  if(AvailiableReaders)
    {
    AvailiableReaders->PrintSelf(os, indent);
    }
  else
    {
    os << "None.";
    }
}

vtkImageReader2Factory::vtkImageReader2Factory()
{
}

vtkImageReader2Factory::~vtkImageReader2Factory()
{
}

void vtkImageReader2Factory::RegisterReader(vtkImageReader2* r)
{
  vtkImageReader2Factory::InitializeReaders();
  AvailiableReaders->AddItem(r);
}


vtkImageReader2* vtkImageReader2Factory::CreateImageReader2(const char* path)
{ 
  vtkImageReader2Factory::InitializeReaders();
  vtkImageReader2* ret;
  vtkCollection* collection = vtkCollection::New();
  vtkObjectFactory::CreateAllInstance("vtkImageReaderObject",
                                      collection);
  vtkObject* o;
  // first try the current registered object factories to see
  // if one of them can 
  for(collection->InitTraversal(); (o = collection->GetNextItemAsObject()); )
    {
    if(o)
      {
      ret = vtkImageReader2::SafeDownCast(o);
      if(ret && ret->CanReadFile(path))
        {
        return ret;
        }
      }
    }
  // get rid of the collection
  collection->Delete();
  vtkCollectionSimpleIterator sit;
  for(vtkImageReader2Factory::AvailiableReaders->InitTraversal(sit);
      (ret = vtkImageReader2Factory::AvailiableReaders->GetNextImageReader2(sit));)
    {
    if(ret->CanReadFile(path))
      {
      // like a new call
      return ret->NewInstance();
      }
    }
  return 0;
}


void vtkImageReader2Factory::InitializeReaders()
{
  if(vtkImageReader2Factory::AvailiableReaders)
    {
    return;
    }
  vtkCleanUpImageReader2FactoryGlobal.Use();
  vtkImageReader2Factory::AvailiableReaders = vtkImageReader2Collection::New();
  vtkImageReader2* reader;

  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkPNGReader::New()));
  reader->Delete();
  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkPNMReader::New()));
  reader->Delete();
  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkTIFFReader::New()));
  reader->Delete();
  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkBMPReader::New()));
  reader->Delete();
  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkSLCReader::New()));
  reader->Delete();
  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkJPEGReader::New()));
  reader->Delete();
  vtkImageReader2Factory::AvailiableReaders->
    AddItem((reader = vtkGESignaReader::New()));
  reader->Delete();
}


void vtkImageReader2Factory::GetRegisteredReaders(vtkImageReader2Collection* collection)
{
  vtkImageReader2Factory::InitializeReaders();
  // get all dynamic readers
  vtkObjectFactory::CreateAllInstance("vtkImageReaderObject",
                                      collection);
  // get the current registered readers
  vtkImageReader2* ret;
  vtkCollectionSimpleIterator sit;
  for(vtkImageReader2Factory::AvailiableReaders->InitTraversal(sit);
      (ret = vtkImageReader2Factory::AvailiableReaders->GetNextImageReader2(sit));)
    {
    collection->AddItem(ret);
    }
}


