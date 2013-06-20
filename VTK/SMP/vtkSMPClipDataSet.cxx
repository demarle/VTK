#include "vtkSMPClipDataSet.h"
#include "vtkSMP.h"

#include "vtkCallbackCommand.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkClipVolume.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImageData.h"
#include "vtkImplicitFunction.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkPolyhedron.h"

#include <math.h>

#include "vtkBenchTimer.h"

class GenerateClipValueExecutor : public vtkFunctor<vtkIdType>
{
  GenerateClipValueExecutor(const GenerateClipValueExecutor&);
  void operator =(const GenerateClipValueExecutor&);

protected:
  GenerateClipValueExecutor(){}
  ~GenerateClipValueExecutor(){}

  vtkFloatArray* scalars;
  vtkDataSet* input;
  vtkImplicitFunction* clipFunction;

public:
  vtkTypeMacro(GenerateClipValueExecutor,vtkFunctor<vtkIdType>);
  static GenerateClipValueExecutor* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void SetData(vtkFloatArray* s, vtkDataSet* i, vtkImplicitFunction* f)
    {
    scalars = s;
    input = i;
    clipFunction = f;
    }

  void operator ()(vtkIdType i) const
    {
    double p[3];
    this->input->GetPoint(i,p);
    double s = this->clipFunction->FunctionValue(p);
    this->scalars->SetTuple1(i,s);
    }
};

class DoClip : public vtkFunctorInitialisable<vtkIdType>
{
  DoClip(const DoClip&);
  void operator =(const DoClip&);

protected:
  DoClip() : input(0), clipScalars(0), inPD(0), inCD(0), estimatedSize(0),
             value(0.0), insideOut(0), numOutputs(0), refLocator(0)
    {
    cell = vtkSMP::vtkThreadLocal<vtkGenericCell>::New();
    cellScalars = vtkSMP::vtkThreadLocal<vtkFloatArray>::New();
    newPoints = vtkSMP::vtkThreadLocal<vtkPoints>::New();
    outPD = vtkSMP::vtkThreadLocal<vtkPointData>::New();
    locator = vtkSMP::vtkThreadLocal<vtkIncrementalPointLocator>::New();

    conn[0] = conn[1] = 0;
    outCD[0] = outCD[1] = 0;
    types[0] = types[1] = 0;
    locs[0] = locs[1] = 0;
    }
  ~DoClip()
    {
    cell->Delete();
    cellScalars->Delete();
    newPoints->Delete();
    outPD->Delete();
    locator->Delete();
    for (int i = 0; i < numOutputs; ++i)
      {
      conn[i]->Delete();
      outCD[i]->Delete();
      types[i]->Delete();
      locs[i]->Delete();
      }
    }

  vtkDataSet* input;
  vtkDataArray* clipScalars;
  vtkPointData* inPD;
  vtkCellData* inCD;
  double value;
  int insideOut;
  int numOutputs;
  vtkIdType estimatedSize;
  vtkIncrementalPointLocator* refLocator;

public:
  vtkSMP::vtkThreadLocal<vtkGenericCell>* cell;
  vtkSMP::vtkThreadLocal<vtkFloatArray>* cellScalars;
  vtkSMP::vtkThreadLocal<vtkPoints>* newPoints;
  vtkSMP::vtkThreadLocal<vtkPointData>* outPD;
  vtkSMP::vtkThreadLocal<vtkIncrementalPointLocator>* locator;

  vtkSMP::vtkThreadLocal<vtkCellArray>* conn[2];
  vtkSMP::vtkThreadLocal<vtkCellData>* outCD[2];
  vtkSMP::vtkThreadLocal<vtkUnsignedCharArray>* types[2];
  vtkSMP::vtkThreadLocal<vtkIdTypeArray>* locs[2];

  vtkTypeMacro(DoClip,vtkFunctorInitialisable<vtkIdType>);
  static DoClip* New();
  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void SetData(vtkDataSet* i, vtkDataArray* c, vtkPointData* pd, vtkCellData* cd,
               vtkIdType e, double v, int inside, int num, vtkPoints* pts,
               vtkPointData* out_pd, vtkIncrementalPointLocator* l, vtkCellArray** _conn,
               vtkCellData** _out, vtkUnsignedCharArray** _types, vtkIdTypeArray** _locs)
    {
    input = i; clipScalars = c; inPD = pd; inCD = cd; refLocator = l;
    estimatedSize = e; value = v; insideOut = inside; numOutputs = num;

    input->GetCell(0,cell->NewLocal());
    cellScalars->NewLocal()->Allocate(VTK_CELL_SIZE);
    newPoints->SetLocal(pts);
    locator->SetLocal(l);
    outPD->SetLocal(out_pd);
    for (int i = 0; i < num; ++i)
      {
      conn[i] = vtkSMP::vtkThreadLocal<vtkCellArray>::New();
      conn[i]->SetLocal(_conn[i]);
      outCD[i] = vtkSMP::vtkThreadLocal<vtkCellData>::New();
      outCD[i]->SetLocal(_out[i]);
      types[i] = vtkSMP::vtkThreadLocal<vtkUnsignedCharArray>::New();
      types[i]->SetLocal(_types[i]);
      locs[i] = vtkSMP::vtkThreadLocal<vtkIdTypeArray>::New();
      locs[i]->SetLocal(_locs[i]);
      }

    Initialized();
    }

  void Init() const
    {
    cell->NewLocal();
    cellScalars->NewLocal()->Allocate(VTK_CELL_SIZE);
    vtkPoints* pts = newPoints->NewLocal();
    pts->Allocate(estimatedSize,estimatedSize/2);
    outPD->NewLocal()->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
    locator->NewLocal(refLocator)->InitPointInsertion (pts, input->GetBounds());
    for (int i = 0; i < numOutputs; ++i)
      {
      vtkCellArray* _conn = conn[i]->NewLocal();
      _conn->Allocate(estimatedSize,estimatedSize/2);
      _conn->InitTraversal();
      outCD[i]->NewLocal()->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
      types[i]->NewLocal()->Allocate(estimatedSize,estimatedSize/2);
      locs[i]->NewLocal()->Allocate(estimatedSize,estimatedSize/2);
      }
    Initialized();
    }

  void operator ()(vtkIdType cellId) const
    {
    vtkGenericCell* cell = this->cell->GetLocal();
    input->GetCell(cellId,cell);
    vtkPoints* cellPts = cell->GetPoints();
    vtkIdList* cellIds = cell->GetPointIds();
    vtkIdType npts = cellPts->GetNumberOfPoints();

    // evaluate implicit cutting function
    for (int i=0; i < npts; i++ )
      {
      double s = clipScalars->GetComponent(cellIds->GetId(i), 0);
      cellScalars->GetLocal()->InsertTuple(i, &s);
      }

    // perform the clipping
    int num[2];
    num[0] = conn[0]->GetLocal()->GetNumberOfCells();
    cell->Clip(value, cellScalars->GetLocal(), this->locator->GetLocal(), conn[0]->GetLocal(),
               inPD, outPD->GetLocal(), inCD, cellId, outCD[0]->GetLocal(), this->insideOut);
    num[0] = conn[0]->GetLocal()->GetNumberOfCells() - num[0];

    if ( numOutputs > 1 )
      {
      num[1] = conn[1]->GetLocal()->GetNumberOfCells();
      cell->Clip(value, cellScalars->GetLocal(), this->locator->GetLocal(), conn[1]->GetLocal(),
                 inPD, outPD->GetLocal(), inCD, cellId, outCD[1]->GetLocal(), this->insideOut);
      num[1] = conn[1]->GetLocal()->GetNumberOfCells() - num[0];
      }

    for (int i=0; i<numOutputs; i++) //for both outputs
      {
      for (int j=0; j < num[i]; j++)
        {
        if (cell->GetCellType() == VTK_POLYHEDRON)
          {
          //Polyhedron cells have a special cell connectivity format
          //(nCell0Faces, nFace0Pts, i, j, k, nFace1Pts, i, j, k, ...).
          //But we don't need to deal with it here. The special case is handled
          //by vtkUnstructuredGrid::SetCells(), which will be called next.
          types[i]->GetLocal()->InsertNextValue(VTK_POLYHEDRON);
          }
        else
          {
          locs[i]->GetLocal()->InsertNextValue(conn[i]->GetLocal()->GetTraversalLocation());
          vtkIdType* pts;
          conn[i]->GetLocal()->GetNextCell(npts,pts);

          int cellType = 0;
          //For each new cell added, got to set the type of the cell
          switch ( cell->GetCellDimension() )
            {
            case 0: //points are generated--------------------------------
              cellType = (npts > 1 ? VTK_POLY_VERTEX : VTK_VERTEX);
              break;

            case 1: //lines are generated---------------------------------
              cellType = (npts > 2 ? VTK_POLY_LINE : VTK_LINE);
              break;

            case 2: //polygons are generated------------------------------
              cellType = (npts == 3 ? VTK_TRIANGLE :
                                      (npts == 4 ? VTK_QUAD : VTK_POLYGON));
              break;

            case 3: //tetrahedra or wedges are generated------------------
              cellType = (npts == 4 ? VTK_TETRA : VTK_WEDGE);
              break;
            } //switch

          types[i]->GetLocal()->InsertNextValue(cellType);
          }
        } //for each new cell
      } //for both outputs
    }
};

vtkStandardNewMacro(DoClip);
vtkStandardNewMacro(GenerateClipValueExecutor);
vtkStandardNewMacro(vtkSMPClipDataSet);

vtkSMPClipDataSet::vtkSMPClipDataSet() : vtkClipDataSet() { }
vtkSMPClipDataSet::~vtkSMPClipDataSet() { }

void vtkSMPClipDataSet::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

int vtkSMPClipDataSet::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet *realInput = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  // We have to create a copy of the input because clip requires being
  // able to InterpolateAllocate point data from the input that is
  // exactly the same as output. If the input arrays and output arrays
  // are different vtkCell3D's Clip will fail. By calling InterpolateAllocate
  // here, we make sure that the output will look exactly like the output
  // (unwanted arrays will be eliminated in InterpolateAllocate). The
  // last argument of InterpolateAllocate makes sure that arrays are shallow
  // copied from realInput to input.
  vtkSmartPointer<vtkDataSet> input;
  input.TakeReference(realInput->NewInstance());
  input->CopyStructure(realInput);
  input->GetCellData()->PassData(realInput->GetCellData());
  input->GetPointData()->InterpolateAllocate(realInput->GetPointData(), 0, 0, 1);

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkUnstructuredGrid *clippedOutput = this->GetClippedOutput();

  vtkIdType numPts = input->GetNumberOfPoints();
  vtkIdType numCells = input->GetNumberOfCells();
  vtkPointData *inPD=input->GetPointData(), *outPD = output->GetPointData();
  vtkCellData *inCD=input->GetCellData();
  vtkCellData *outCD[2];
  vtkPoints *newPoints;
  vtkDataArray *clipScalars;
  vtkPoints *cellPts;
  vtkIdList *cellIds;
  double s;
  vtkIdType npts;
  vtkIdType *pts;
  int cellType = 0;
  vtkIdType i;
  int j;
  vtkIdType estimatedSize;
  vtkUnsignedCharArray *types[2];
  types[0] = types[1] = 0;
  vtkIdTypeArray *locs[2];
  locs[0] = locs[1] = 0;
  int numOutputs = 1;

  outCD[0] = 0;
  outCD[1] = 0;

  vtkDebugMacro(<< "Clipping dataset");

  int inputObjectType = input->GetDataObjectType();

  // if we have volumes
  if (inputObjectType == VTK_STRUCTURED_POINTS ||
      inputObjectType == VTK_IMAGE_DATA)
    {
    int dimension;
    int *dims = vtkImageData::SafeDownCast(input)->GetDimensions();
    for (dimension=3, i=0; i<3; i++)
      {
      if ( dims[i] <= 1 )
        {
        dimension--;
        }
      }
    if ( dimension >= 3 )
      {
      this->ClipVolume(input, output);
      return 1;
      }
     }

  // Initialize self; create output objects
  //
  if ( numPts < 1 )
    {
    vtkDebugMacro(<<"No data to clip");
    return 1;
    }

  if ( !this->ClipFunction && this->GenerateClipScalars )
    {
    vtkErrorMacro(<<"Cannot generate clip scalars if no clip function defined");
    return 1;
    }

  if ( numCells < 1 )
    {
    return this->ClipPoints(input, output, inputVector);
    }

  // allocate the output and associated helper classes
  estimatedSize = numCells;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }
  vtkCellArray *conn[2];
  conn[0] = conn[1] = 0;
  conn[0] = vtkCellArray::New();
  conn[0]->Allocate(estimatedSize,estimatedSize/2);
  conn[0]->InitTraversal();
  types[0] = vtkUnsignedCharArray::New();
  types[0]->Allocate(estimatedSize,estimatedSize/2);
  locs[0] = vtkIdTypeArray::New();
  locs[0]->Allocate(estimatedSize,estimatedSize/2);
  if ( this->GenerateClippedOutput )
    {
    numOutputs = 2;
    conn[1] = vtkCellArray::New();
    conn[1]->Allocate(estimatedSize,estimatedSize/2);
    conn[1]->InitTraversal();
    types[1] = vtkUnsignedCharArray::New();
    types[1]->Allocate(estimatedSize,estimatedSize/2);
    locs[1] = vtkIdTypeArray::New();
    locs[1]->Allocate(estimatedSize,estimatedSize/2);
    }
  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts,numPts/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  // Determine whether we're clipping with input scalars or a clip function
  // and do necessary setup.
  if ( this->ClipFunction )
    {
    vtkFloatArray *tmpScalars = vtkFloatArray::New();
    tmpScalars->SetNumberOfTuples(numPts);
    tmpScalars->SetName("ClipDataSetScalars");
    inPD = vtkPointData::New();
    inPD->ShallowCopy(input->GetPointData());//copies original
    if ( this->GenerateClipScalars )
      {
      inPD->SetScalars(tmpScalars);
      }
    GenerateClipValueExecutor* generateClipScalarFunctor =
        GenerateClipValueExecutor::New();
    generateClipScalarFunctor->SetData(tmpScalars,input,this->ClipFunction);
    vtkSMP::ForEach(0,numPts,generateClipScalarFunctor);
    generateClipScalarFunctor->Delete();
    clipScalars = tmpScalars;
    }
  else //using input scalars
    {
    clipScalars = this->GetInputArrayToProcess(0,inputVector);
    if ( !clipScalars )
      {
      for ( i=0; i<2; i++ )
        {
        if (conn[i])
          {
          conn[i]->Delete();
          }
        if (types[i])
          {
          types[i]->Delete();
          }
        if (locs[i])
          {
          locs[i]->Delete();
          }
        }
      newPoints->Delete();
      // When processing composite datasets with partial arrays, this warning is
      // not applicable, hence disabling it.
      // vtkErrorMacro(<<"Cannot clip without clip function or input scalars");
      return 1;
      }
    }

  // Refer to BUG #8494 and BUG #11016. I cannot see any reason why one would
  // want to turn CopyScalars Off. My understanding is that this was done to
  // avoid copying of "ClipDataSetScalars" to the output when
  // this->GenerateClipScalars is false. But, if GenerateClipScalars is false,
  // then "ClipDataSetScalars" is not added as scalars to the input at all
  // (refer to code above) so it's a non-issue. Leaving CopyScalars untouched
  // i.e. ON avoids dropping of arrays (#8484) as well as segfaults (#11016).
  //if ( !this->GenerateClipScalars &&
  //  !this->GetInputArrayToProcess(0,inputVector))
  //  {
  //  outPD->CopyScalarsOff();
  //  }
  //else
  //  {
  //  outPD->CopyScalarsOn();
  //  }
  vtkDataSetAttributes* tempDSA = vtkDataSetAttributes::New();
  tempDSA->InterpolateAllocate(inPD, 1, 2);
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
  tempDSA->Delete();
  outCD[0] = output->GetCellData();
  outCD[0]->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  if ( this->GenerateClippedOutput )
    {
    outCD[1] = clippedOutput->GetCellData();
    outCD[1]->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
    }

  //Process all cells and clip each in turn
  //
  DoClip* functor = DoClip::New();
  functor->SetData(input, clipScalars, inPD, inCD, estimatedSize,
                   this->UseValueAsOffset || !this->ClipFunction ? this->Value : 0.0,
                   this->InsideOut, numOutputs, newPoints, outPD, this->Locator,
                   conn, outCD, types, locs);
  vtkSMP::ForEach(0,numCells,functor);

//  vtkSMPMergePoints* parallelLocator = vtkSMPMergePoints::SafeDownCast(this->Locator);
//  if (parallelLocator)
//    {
//    vtkSMP::vtkThreadLocal<vtkSMPMergePoints>* partialMeshes;
//    functor->locator->FillDerivedThreadLocal(partialMeshes);
//    vtkSMP::MergePoints(parallelLocator, partialMeshes, outPD, functor->outPD, conn[0], functor->conn[0], 0, 0, 0, 0, 0, 0, outCD[0], functor->outCD[0], 1);
//    partialMeshes->Delete();
//    }
//  else
//    {
//    vtkSMP::MergePoints(newPoints, functor->newPoints, input->GetBounds(), outPD, functor->outPD, conn[0], functor->conn[0], 0, 0, 0, 0, 0, 0, outCD[0], functor->outCD[0], 1);
//    }
  functor->Delete();

  if ( this->ClipFunction )
    {
    clipScalars->Delete();
    inPD->Delete();
    }

  output->SetPoints(newPoints);
  output->SetCells(types[0], locs[0], conn[0]);
  conn[0]->Delete();
  types[0]->Delete();
  locs[0]->Delete();

  if ( this->GenerateClippedOutput )
    {
    clippedOutput->SetPoints(newPoints);
    clippedOutput->SetCells(types[1], locs[1], conn[1]);
    conn[1]->Delete();
    types[1]->Delete();
    locs[1]->Delete();
    }

  newPoints->Delete();
  this->Locator->Initialize();//release any extra memory
  output->Squeeze();

  return 1;
}
