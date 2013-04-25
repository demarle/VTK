#include "vtkSMPExecutive.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include "vtkSMP.h"
#include "vtkInformation.h"
#include "vtkInformationExecutivePortKey.h"
#include "vtkAlgorithm.h"
#include "vtkInformationVector.h"

class FilterSpawn;

struct FilterTask : public vtkTask {
  vtkInformation* request;
  vtkInformation* info;
  int* result;
  std::string name;

  vtkTypeMacro(FilterTask,vtkTask);
  static FilterTask* New() { return new FilterTask; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void SetInformations( vtkInformation* r, vtkInformation* i, int* res, std::string name )
    {
    if (request)
      {
      request->UnRegister(this);
      info->UnRegister(this);
      }
    request = r;
    info = i;
    if (request)
      {
      request->Register(this);
      info->Register(this);
      }
    result = res;
    this->name = name;
    }

  void Execute( );

protected:
  FilterTask() : request(0), info(0), result(0) {}
  ~FilterTask()
    {
    if (request)
      {
      this->request->UnRegister(this);
      this->info->UnRegister(this);
      }
    }

private:
  FilterTask( const FilterTask& );
  void operator =( const FilterTask& );
};

class FilterSpawn : public vtkSMP::vtkSpawnTasks {
  vtkInformation* request;
  std::string name;

  vtkTypeMacro(FilterSpawn,vtkSpawnTasks);
  static FilterSpawn* New() { return new FilterSpawn; }
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void SetRequest( vtkInformation* request, std::string name )
    {
    if (this->request)
      this->request->UnRegister(this);
    this->request = request;
    if (this->request)
      this->request->Register(this);
    this->name = name;
    }

  void ForwardUpstream( vtkInformation* info, int* result )
    {
    vtkInformation* localRequest = vtkInformation::New();
    localRequest->Copy(request, 1);
    FilterTask* task = FilterTask::New();
    task->SetInformations(request, info, result, name);
    this->InternalSpawn(task);
    task->Delete();
    }

protected:
  FilterSpawn() : request(0) {}
  ~FilterSpawn()
    {
    if (request)
      request->UnRegister(this);
    }
private:
  FilterSpawn( const FilterSpawn& );
  void operator =( const FilterSpawn& );
};

void FilterTask::Execute()
  {
  cout << name << " spawned" << endl;
  if (request)
    {
    // Get the executive producing this input.  If there is none, then
    // it is a NULL input.
    vtkExecutive* e;
    int producerPort;
    vtkExecutive::PRODUCER()->Get(info,e,producerPort);
    if(e)
      {
      request->Set(vtkExecutive::FROM_OUTPUT_PORT(), producerPort);
      if(!e->ProcessRequest(request,
                            e->GetInputInformation(),
                            e->GetOutputInformation()))
        {
        *result = 0;
        }
      }
    }
  }


vtkStandardNewMacro(vtkSMPExecutive);

vtkSMPExecutive::vtkSMPExecutive()
  {
  }

vtkSMPExecutive::~vtkSMPExecutive()
  {
  }

void vtkSMPExecutive::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

int vtkSMPExecutive::ForwardUpstream(vtkInformation* request)
  {
  // Do not forward upstream if the input is shared with another
  // executive.
  if(this->SharedInputInformation)
    {
    return 1;
    }

  if (!this->Algorithm->ModifyRequest(request, BeforeForward))
    {
    return 0;
    }

  // Forward the request upstream through all input connections.
  std::string className(this->Algorithm->GetClassName());
  FilterSpawn* previousFilters = FilterSpawn::New();
  previousFilters->SetRequest(request,className);
//  request->Print(cout);
  int result = 1;
  cout << className << " before ForwardUpstream" << endl;
  for(int i=0; i < this->GetNumberOfInputPorts(); ++i)
    {
    int nic = this->Algorithm->GetNumberOfInputConnections(i);
    vtkInformationVector* inVector = this->GetInputInformation()[i];
    for(int j=0; j < nic; ++j)
      {
      vtkInformation* info = inVector->GetInformationObject(j);
      previousFilters->ForwardUpstream(info, &result);
      }
    }
  previousFilters->Delete();
  cout << className << " after sync of ForwardUpstream" << endl;

  if (!this->Algorithm->ModifyRequest(request, AfterForward))
    {
    return 0;
    }

  return result;
  }
