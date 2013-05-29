#include "vtkSMPPipeline.h"
#include "vtkSMP.h"

#include "vtkAlgorithm.h"
#include "vtkSmartPointer.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiPieceDataSet.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationExecutivePortKey.h"

#include "vtkBenchTimer.h"

vtkInformationKeyMacro(vtkSMPPipeline, DATA_OBJECT_CONCRETE_TYPE, String);

class ParallelFilterExecutor : public vtkFunctorInitialisable
{
    ParallelFilterExecutor(const ParallelFilterExecutor&);
    void operator =(const ParallelFilterExecutor&);

  protected:
    ParallelFilterExecutor() : inInfoVec(0), outInfoVec(0), inLocalInfo(0), outLocalInfo(0), request(0), requests(0), Executive(0) {}
    ~ParallelFilterExecutor()
      {
      if (Executive)
        {
        for (vtkIdType i=0; i<this->Executive->GetNumberOfInputPorts(); ++i)
          {
          inInfoVec[i]->Delete();
          inLocalInfo[i]->Delete();
          }
        delete [] inInfoVec;
        delete [] inLocalInfo;
        }
      if (outLocalInfo) outLocalInfo->Delete();
      if (outInfoVec) outInfoVec->Delete();
      if (requests) requests->Delete();
      }

    mutable vtkstd::vector<vtkDataObject*> inObjs;
    mutable vtkstd::vector<vtkDataObject*> outObjs;
    vtkInformationVector** inInfoVec;
    vtkInformationVector* outInfoVec;
    vtkSMP::vtkThreadLocal<vtkInformationVector> **inLocalInfo, *outLocalInfo;
    vtkInformation* request;
    vtkSMP::vtkThreadLocal<vtkInformation>* requests;

    int compositePort, numTimeSteps;
    double* times;
    vtkSMPPipeline* Executive;

  public:
    vtkTypeMacro(ParallelFilterExecutor, vtkFunctorInitialisable);
    static ParallelFilterExecutor* New();
    void PrintSelf(ostream &os, vtkIndent indent)
      {
      this->Superclass::PrintSelf(os,indent);
      }

    void Init() const
      {
      vtkIdType numPorts = this->Executive->GetNumberOfInputPorts();
      for (vtkIdType i = 0; i < numPorts; ++i)
        {
        vtkInformationVector* info = this->inLocalInfo[i]->NewLocal();
        info->Copy(inInfoVec[i], 1);
        }
      this->outLocalInfo->NewLocal()->Copy(outInfoVec, 1);
      this->requests->NewLocal()->Copy(request, 1);

      this->Initialized();
      }

    void operator ()(vtkIdType id, vtkIdType vtkNotUsed(id1), vtkIdType vtkNotUsed(id2)) const
      {
      vtkDataObject* dobj = this->inObjs[id];
      if (dobj)
        {
        vtkInformation* r = this->requests->GetLocal();
        vtkIdType numPorts = this->Executive->GetNumberOfInputPorts();
        vtkInformationVector** inInfoVector = new vtkInformationVector*[numPorts];
        for (vtkIdType i=0; i<numPorts; ++i)
          {
          inInfoVector[i] = this->inLocalInfo[i]->GetLocal();
          }
        vtkInformation* inInfo = this->inLocalInfo[this->compositePort]->GetLocal()->GetInformationObject(0);
        vtkInformation* outInfo = this->outLocalInfo->GetLocal()->GetInformationObject(0);
        // if it is a temporal input, set the time for each piece
        if (times)
          {
          outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS(), times, numTimeSteps);
          }

        // Note that since VisitOnlyLeaves is ON on the iterator,
        // this method is called only for leaves, hence, we are assured that
        // neither dobj nor outObj are vtkCompositeDataSet subclasses.
        this->outObjs[id] =
            this->Executive->ExecuteSimpleAlgorithmForBlock(
                                                 inInfoVector,
                                                 outLocalInfo->GetLocal(),
                                                 inInfo,
                                                 outInfo,
                                                 r,
                                                 dobj);
        delete [] inInfoVector;
        }
      else
        {
        this->outObjs[id] = 0;
        }
      }

    void PrepareData(vtkCompositeDataIterator* iter, vtkInformationVector** _inInfoVec,
                     vtkInformationVector* _outInfoVec, vtkInformation* _request,
                     vtkSMPPipeline* self, int _compositePort, double* _times, int _n)
      {
      this->compositePort = _compositePort;
      this->times = _times;
      this->numTimeSteps = _n;
      this->Executive = self;

      vtkIdType numPorts = self->GetNumberOfInputPorts();
      this->inInfoVec = new vtkInformationVector*[numPorts];
      this->outInfoVec = vtkInformationVector::New();
      this->outInfoVec->Copy(_outInfoVec, 1);

      this->inLocalInfo = new vtkSMP::vtkThreadLocal<vtkInformationVector>*[numPorts];
      for (vtkIdType i = 0; i < numPorts; ++i)
        {
        this->inInfoVec[i] = vtkInformationVector::New();
        this->inInfoVec[i]->Copy(_inInfoVec[i], 1);
        this->inLocalInfo[i] = vtkSMP::vtkThreadLocal<vtkInformationVector>::New();
        }

      this->outLocalInfo = vtkSMP::vtkThreadLocal<vtkInformationVector>::New();

      this->request = _request;
      this->requests = vtkSMP::vtkThreadLocal<vtkInformation>::New();

      this->inObjs.resize(0);
      for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
        {
        this->inObjs.push_back(iter->GetCurrentDataObject());
        }
      this->outObjs.resize(this->inObjs.size(), NULL);

      this->Init();
      }

    size_t GetInputSize()
      {
      return this->inObjs.size();
      }

    void FinalizeData(vtkCompositeDataIterator* iter, vtkCompositeDataSet* compositeOutput)
      {
      size_t i = 0;
      for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), ++i)
        {
        vtkDataObject* outObj = this->outObjs[i];
        if (outObj)
          {
          compositeOutput->SetDataSet(iter, outObj);
          outObj->FastDelete();
          }
        }
      }
};

vtkStandardNewMacro(ParallelFilterExecutor);
vtkStandardNewMacro(vtkSMPPipeline);

vtkSMPPipeline::vtkSMPPipeline() : vtkCompositeDataPipeline()
  {
  }

vtkSMPPipeline::~vtkSMPPipeline()
  {
  }

void vtkSMPPipeline::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

int vtkSMPPipeline::ExecuteData(vtkInformation *request, vtkInformationVector **inInfoVec, vtkInformationVector *outInfoVec)
  {
  vtkDebugMacro(<< "ExecuteData");
  int result = 1;

  int compositePort;
  bool composite = this->ShouldIterateOverInput(compositePort);
  bool temporal =
    this->ShouldIterateTemporalData(request, inInfoVec, outInfoVec);
  if (temporal || composite)
    {
    if (this->Algorithm->IsA("vtkSMPAlgorithm"))
      {
      vtkDebugMacro(<< "vtkSMPAlgorithm will produce a composite data output from each input bloc")
      this->Superclass::ExecuteSimpleAlgorithm(request, inInfoVec, outInfoVec, compositePort);
      }
    else
      {
      vtkDebugMacro(<< "Iterates over each input bloc in parallel")
      this->ExecuteSimpleAlgorithm(request, inInfoVec, outInfoVec, compositePort);
      }
    }
  else
    {
    vtkDebugMacro(<< "  Superclass::ExecuteData");
    result = this->Superclass::ExecuteData(request,inInfoVec,outInfoVec);
    }

  return result;
  }

void vtkSMPPipeline::ExecuteSimpleAlgorithm(
    vtkInformation *request, vtkInformationVector **inInfoVec,
    vtkInformationVector *outInfoVec, int compositePort)
  {
  vtkDebugMacro(<< "ExecuteSimpleAlgorithm");

  this->ExecuteDataStart(request,inInfoVec,outInfoVec);

  vtkInformation* outInfo = 0;

  if (this->GetNumberOfOutputPorts() > 0)
    {
    outInfo = outInfoVec->GetInformationObject(0);
    }

  // Make sure a valid composite data object exists for all output ports.
  for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
    {
    this->CheckCompositeData(request, i, inInfoVec, outInfoVec);
    }

  // if we have no composite inputs then we are looping over time on a source
  if (compositePort==-1)
    {
    this->ExecuteSimpleAlgorithmTime(request, inInfoVec, outInfoVec);
    return;
    }

  // Loop using the first input on the first port.
  // This might not be valid for all cases but it is a decent
  // assumption to start with.
  // TODO: Loop over all inputs
  vtkInformation* inInfo = 0;
  inInfo = this->GetInputInformation(compositePort, 0);
  vtkCompositeDataSet* input = vtkCompositeDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkCompositeDataSet> compositeOutput =
    vtkCompositeDataSet::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // do we have a request for multiple time steps?
  int numTimeSteps = 0;
  double *times = 0;
  numTimeSteps = outInfo->Length(UPDATE_TIME_STEPS());
  if (numTimeSteps)
    {
    times = new double [numTimeSteps];
    memcpy(times,outInfo->Get(UPDATE_TIME_STEPS()),
           sizeof(double)*numTimeSteps);
    }

  if (input && compositeOutput)
    {
    compositeOutput->PrepareForNewData();
    compositeOutput->CopyStructure(input);

    vtkSmartPointer<vtkInformation> r =
      vtkSmartPointer<vtkInformation>::New();

    r->Set(FROM_OUTPUT_PORT(), PRODUCER()->GetPort(outInfo));

    // The request is forwarded upstream through the pipeline.
    r->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);

    // Algorithms process this request after it is forwarded.
    r->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);


    // Store the information (whole_extent and maximum_number_of_pieces)
    // before looping. Otherwise, executeinformation will cause
    // changes (because we pretend that the max. number of pieces is
    // one to process the whole block)
    this->PushInformation(inInfo);

    vtkDebugMacro(<< "EXECUTING " << this->Algorithm->GetClassName());;

    // True when the pipeline is iterating over the current (simple)
    // filter to produce composite output. In this case,
    // ExecuteDataStart() should NOT Initialize() the composite output.
    this->InLocalLoop = 1;

    vtkSmartPointer<vtkCompositeDataIterator> iter;
    iter.TakeReference(input->NewIterator());
    iter->VisitOnlyLeavesOn();

    vtkBenchTimer::Deactivate();
    ParallelFilterExecutor* functor = ParallelFilterExecutor::New();
    functor->PrepareData(iter,inInfoVec,outInfoVec,r,this,compositePort,times,numTimeSteps);
    vtkSMP::ForEach(0,functor->GetInputSize(),functor);
    functor->FinalizeData(iter,compositeOutput);
    functor->Delete();
    vtkBenchTimer::Activate();

    // True when the pipeline is iterating over the current (simple)
    // filter to produce composite output. In this case,
    // ExecuteDataStart() should NOT Initialize() the composite output.
    this->InLocalLoop = 0;
    // Restore the extent information and force it to be
    // copied to the output. Composite sources should set
    // MAXIMUM_NUMBER_OF_PIECES to -1 anyway (and handle
    // piece requests properly).
    this->PopInformation(inInfo);
    if (times)
      {
      outInfo->Set(UPDATE_TIME_STEPS(), times, numTimeSteps);
      compositeOutput->GetInformation()->Set(
        vtkDataObject::DATA_TIME_STEPS(), times, numTimeSteps);
      delete [] times;
      }
    r->Set(REQUEST_INFORMATION());
    this->CopyDefaultInformation(r, vtkExecutive::RequestDownstream,
                                 this->GetInputInformation(),
                                 this->GetOutputInformation());

    vtkDataObject* curInput = inInfo->Get(vtkDataObject::DATA_OBJECT());
    if (curInput != input)
      {
      inInfo->Remove(vtkDataObject::DATA_OBJECT());
      inInfo->Set(vtkDataObject::DATA_OBJECT(), input);
      }
    vtkDataObject* curOutput = outInfo->Get(vtkDataObject::DATA_OBJECT());
    if (curOutput != compositeOutput.GetPointer())
      {
      compositeOutput->SetPipelineInformation(outInfo);
      }
    }
  this->ExecuteDataEnd(request,inInfoVec,outInfoVec);
  }

int vtkSMPPipeline::CheckDataObject(int port, vtkInformationVector* outInfoVec)
  {
  if (!this->Superclass::CheckDataObject(port, outInfoVec))
    {
    return 0;
    }

  // SMP Algorithm declare some type for their output but
  // actually produce a multi pieces dataset. Each piece
  // being of that type.
  if (this->Algorithm->IsA("vtkSMPAlgorithm"))
    {
    vtkInformation* outInfo = outInfoVec->GetInformationObject(port);
    // For now, only create a Multi Piece Dataset. Checks
    // will come later.
    vtkMultiPieceDataSet* data = vtkMultiPieceDataSet::New();
    outInfo->Set(vtkSMPPipeline::DATA_OBJECT_CONCRETE_TYPE(),
                  outInfo->Get(vtkDataObject::DATA_OBJECT())->GetClassName());
    this->SetOutputData(port, data, outInfo);
    }

  return 1;
  }
