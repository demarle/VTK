/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkComputingResources.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright (c) 2008, 2009 by SCI Institute, University of Utah.
  
  This is part of the Parallel Dataflow System originally developed by
  Huy T. Vo and Claudio T. Silva. For more information, see:

  "Parallel Dataflow Scheme for Streaming (Un)Structured Data" by Huy
  T. Vo, Daniel K. Osmari, Brian Summa, Joao L.D. Comba, Valerio
  Pascucci and Claudio T. Silva, SCI Institute, University of Utah,
  Technical Report #UUSCI-2009-004, 2009.

  "Multi-Threaded Streaming Pipeline For VTK" by Huy T. Vo and Claudio
  T. Silva, SCI Institute, University of Utah, Technical Report
  #UUSCI-2009-005, 2009.
-------------------------------------------------------------------------*/
#include "vtkComputingResources.h"

#include "vtkInformation.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkThreadedImageAlgorithm.h"
#include "vtkThreadedStreamingPipeline.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkComputingResources, "1.1");
vtkStandardNewMacro(vtkComputingResources);

//----------------------------------------------------------------------------
class vtkCPUResource: public vtkProcessingUnitResource {
public:
  virtual int ProcessingUnit() {
    return vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU;
  }
  
  virtual bool HasResource() {
    return this->NumberOfThreads>0;
  }
  
  virtual void Clear() {
    this->NumberOfThreads = 0;
  }
  
  virtual void ObtainMinimum() {
    this->NumberOfThreads = 1;
  }
  
  virtual void ObtainMaximum() {
    this->NumberOfThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
  }
  
  virtual void IncreaseByRatio(float ratio, vtkProcessingUnitResource *refResource) {
    vtkCPUResource *other = static_cast<vtkCPUResource*>(refResource);
    int thisNThread = (int)(ratio*other->NumberOfThreads+0.5);
    if (thisNThread<1)
      thisNThread = 1;
    this->NumberOfThreads += thisNThread;
  }
  
  virtual void AllocateFor(vtkThreadedStreamingPipeline *exec) {
    if (exec->GetAlgorithm()->IsA("vtkThreadedImageAlgorithm")) {
      vtkThreadedImageAlgorithm::SafeDownCast(exec->GetAlgorithm())
        ->SetNumberOfThreads(this->NumberOfThreads);
    }
  }

  virtual bool CanAccommodate(vtkProcessingUnitResource *refResource) {
    vtkCPUResource *other = static_cast<vtkCPUResource*>(refResource);
    return this->NumberOfThreads>=other->NumberOfThreads;
  }  
  
  virtual void Reserve(vtkProcessingUnitResource *refResource) {
    vtkCPUResource *other = static_cast<vtkCPUResource*>(refResource);    
    this->NumberOfThreads -= other->NumberOfThreads;
  }

  virtual void Collect(vtkProcessingUnitResource *refResource) {
    vtkCPUResource *other = static_cast<vtkCPUResource*>(refResource);    
    this->NumberOfThreads += other->NumberOfThreads;
  }

private:
  int NumberOfThreads;
};

//----------------------------------------------------------------------------
// This needs to reimplement
class vtkGPUResource: public vtkProcessingUnitResource {
public:
  virtual int ProcessingUnit() {
    return vtkThreadedStreamingPipeline::PROCESSING_UNIT_GPU;
  }
  
  virtual bool HasResource() {
    return false;
  }
  
  virtual void Clear() {}
  
  virtual void ObtainMinimum() {}
  
  virtual void ObtainMaximum() {}
  
  virtual void IncreaseByRatio(float ratio, vtkProcessingUnitResource *refResource) {}
  
  virtual void AllocateFor(vtkThreadedStreamingPipeline *exec) {
    fprintf(stderr, "vtkGPUResource NEEDS TO BE IMPLEMENTED!!!!\n");
  }

  bool CanAccommodate(vtkProcessingUnitResource *refResource) {
    return false;
  }
  
  void Reserve(vtkProcessingUnitResource *refResource) {}
  
  void Collect(vtkProcessingUnitResource *refResource) {}
  
private:
};

//----------------------------------------------------------------------------
vtkComputingResources::vtkComputingResources() {
  this->ResourceMap[vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU] = new vtkCPUResource();
  this->ResourceMap[vtkThreadedStreamingPipeline::PROCESSING_UNIT_GPU] = new vtkGPUResource();
  this->ObtainMinimumResources();
}

//----------------------------------------------------------------------------
vtkComputingResources::~vtkComputingResources() {
  for (ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.begin();
       i!=this->ResourceMap.end(); i++) {
    delete (*i).second;
  }
  this->ResourceMap.clear();
}

//----------------------------------------------------------------------------
void vtkComputingResources::Clear() {
  for (ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.begin();
       i!=this->ResourceMap.end(); i++) {
    (*i).second->Clear();
  }
}

//----------------------------------------------------------------------------
void vtkComputingResources::ObtainMinimumResources() {
  for (ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.begin();
       i!=this->ResourceMap.end(); i++) {
    (*i).second->ObtainMinimum();
  }
}

//----------------------------------------------------------------------------
void vtkComputingResources::ObtainMaximumResources() {
  for (ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.begin();
       i!=this->ResourceMap.end(); i++) {
    (*i).second->ObtainMaximum();
  }
}

//----------------------------------------------------------------------------
void vtkComputingResources::Deploy(vtkThreadedStreamingPipeline *exec, vtkInformation *info) {
  for (ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.begin();
       i!=this->ResourceMap.end(); i++) {
    int resource = vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU;
//     if (exec->GetAlgorithm()->GetInformation()->
//         Has(vtkThreadedStreamingPipeline::PROCESSING_UNIT()))
//       resource = exec->GetAlgorithm()->GetInformation()->
//         Get(vtkThreadedStreamingPipeline::PROCESSING_UNIT());
    if (((*i).first & resource) &&
        (*i).second->HasResource()) {
      (*i).second->AllocateFor(exec);
      exec->Update();
//       exec->ForceUpdateData((*i).first, info);
    }
  }
}

//----------------------------------------------------------------------------
bool vtkComputingResources::Reserve(vtkComputingResources *res) {
  ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.
    find(vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU);
  ProcessingUnitToResourceHashMap::iterator j = res->ResourceMap.
    find(vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU);
  bool ok = (*i).second->CanAccommodate((*j).second);
  if (ok)
    (*i).second->Reserve((*j).second);
  return ok;
}

//----------------------------------------------------------------------------
void vtkComputingResources::Collect(vtkComputingResources *res) {
  ProcessingUnitToResourceHashMap::iterator i = this->ResourceMap.
    find(vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU);
  ProcessingUnitToResourceHashMap::iterator j = res->ResourceMap.
    find(vtkThreadedStreamingPipeline::PROCESSING_UNIT_CPU);
  (*i).second->Collect((*j).second);
}
