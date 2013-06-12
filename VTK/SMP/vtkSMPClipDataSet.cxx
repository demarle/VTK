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
  vtkFloatArray *cellScalars;
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
  cellScalars = vtkFloatArray::New();
  cellScalars->Allocate(VTK_CELL_SIZE);
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
      cellScalars->Delete();
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
  int abort=0;
  vtkIdType updateTime = numCells/20 + 1;  // update roughly every 5%
  vtkGenericCell *cell = vtkGenericCell::New();
  int num[2]; num[0]=num[1]=0;
  int numNew[2]; numNew[0]=numNew[1]=0;
  for (vtkIdType cellId=0; cellId < numCells && !abort; cellId++)
    {
    if ( !(cellId % updateTime) )
      {
      this->UpdateProgress(static_cast<double>(cellId) / numCells);
      abort = this->GetAbortExecute();
      }

    input->GetCell(cellId,cell);
    cellPts = cell->GetPoints();
    cellIds = cell->GetPointIds();
    npts = cellPts->GetNumberOfPoints();

    // evaluate implicit cutting function
    for ( i=0; i < npts; i++ )
      {
      s = clipScalars->GetComponent(cellIds->GetId(i), 0);
      cellScalars->InsertTuple(i, &s);
      }

    double value = 0.0;
    if (this->UseValueAsOffset || !this->ClipFunction)
      {
      value = this->Value;
      }

    // perform the clipping
    cell->Clip(value, cellScalars, this->Locator, conn[0],
               inPD, outPD, inCD, cellId, outCD[0], this->InsideOut);
    numNew[0] = conn[0]->GetNumberOfCells() - num[0];
    num[0] = conn[0]->GetNumberOfCells();

    if ( this->GenerateClippedOutput )
      {
      cell->Clip(value, cellScalars, this->Locator, conn[1],
                 inPD, outPD, inCD, cellId, outCD[1], !this->InsideOut);
      numNew[1] = conn[1]->GetNumberOfCells() - num[1];
      num[1] = conn[1]->GetNumberOfCells();
      }

    for (i=0; i<numOutputs; i++) //for both outputs
      {
      for (j=0; j < numNew[i]; j++)
        {
        if (cell->GetCellType() == VTK_POLYHEDRON)
          {
          //Polyhedron cells have a special cell connectivity format
          //(nCell0Faces, nFace0Pts, i, j, k, nFace1Pts, i, j, k, ...).
          //But we don't need to deal with it here. The special case is handled
          //by vtkUnstructuredGrid::SetCells(), which will be called next.
          types[i]->InsertNextValue(VTK_POLYHEDRON);
          }
        else
          {
          locs[i]->InsertNextValue(conn[i]->GetTraversalLocation());
          conn[i]->GetNextCell(npts,pts);

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

          types[i]->InsertNextValue(cellType);
          }
        } //for each new cell
      } //for both outputs
    } //for each cell

  cell->Delete();
  cellScalars->Delete();

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
