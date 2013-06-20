#include "vtkSMPDistributePolyData.h"

#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkSMP.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"

#include "vtkGenericCell.h"

vtkStandardNewMacro(vtkSMPDistributePolyData);

class PointDistribute : public vtkFunctor {
  PointDistribute( const PointDistribute& );
  void operator =( const PointDistribute& );

public:
  vtkTypeMacro(PointDistribute,vtkFunctor);
  static PointDistribute* New();

  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Setup( vtkPoints* inPts, vtkPoints* outPts, vtkPointData* inPdata, vtkPointData* outPdata )
    {
    if (in) in->UnRegister(this);
    if (out) out->UnRegister(this);
    if (inPd) inPd->UnRegister(this);
    if (outPd) outPd->UnRegister(this);
    in = inPts;
    out = outPts;
    inPd = inPdata;
    outPd = outPdata;
    if (in) in->Register(this);
    if (out) out->Register(this);
    if (inPd) inPd->Register(this);
    if (outPd) outPd->Register(this);
    }

  void operator ()( vtkIdType id ) const
    {
    double x[3];
    in->GetPoint(id, x);
    out->SetPoint( id, x );
    outPd->CopyData( inPd, id, id );
    }

protected:
  vtkPoints *in,*out;
  vtkPointData *inPd, *outPd;

  PointDistribute() : in(0), out(0), inPd(0), outPd(0) {}
  ~PointDistribute()
    {
    if (in) in->UnRegister(this);
    if (out) out->UnRegister(this);
    if (inPd) inPd->UnRegister(this);
    if (outPd) outPd->UnRegister(this);
    }
};

vtkStandardNewMacro(PointDistribute);

class CellDistribute : public vtkFunctor {
  CellDistribute( const CellDistribute& );
  void operator =( const CellDistribute& );

public:
  vtkTypeMacro(CellDistribute,vtkFunctor);
  static CellDistribute* New();

  void PrintSelf(ostream &os, vtkIndent indent)
    {
    this->Superclass::PrintSelf(os,indent);
    }

  void Setup( vtkCellArray* inVerts, vtkCellArray* inLines, vtkCellArray* inPolys, vtkCellArray* inStrips,
              vtkCellArray* outVerts, vtkCellArray* outLines, vtkCellArray* outPolys, vtkCellArray* outStrips,
              vtkCellData* inCdata, vtkCellData* outCdata, vtkPolyData* ci )
    {
    if (inS) inS->UnRegister(this);
    if (outS) outS->UnRegister(this);
    if (inP) inP->UnRegister(this);
    if (outP) outP->UnRegister(this);
    if (inL) inL->UnRegister(this);
    if (outL) outL->UnRegister(this);
    if (inV) inV->UnRegister(this);
    if (outV) outV->UnRegister(this);
    if (inCd) inCd->UnRegister(this);
    if (outCd) outCd->UnRegister(this);
    if (cellsInfo) cellsInfo->UnRegister(this);
    inV = inVerts;
    outV = outVerts;
    inL = inLines;
    outL = outLines;
    inP = inPolys;
    outP = outPolys;
    inS = inStrips;
    outS = outStrips;
    inCd = inCdata;
    outCd = outCdata;
    cellsInfo = ci;
    if (inS) inS->Register(this);
    if (outS) outS->Register(this);
    if (inP) inP->Register(this);
    if (outP) outP->Register(this);
    if (inL) inL->Register(this);
    if (outL) outL->Register(this);
    if (inV) inV->Register(this);
    if (outV) outV->Register(this);
    if (inCd) inCd->Register(this);
    if (outCd) outCd->Register(this);
    if (cellsInfo) cellsInfo->Register(this);
    vtkIdType _offset = 0;
    vtkIdType _num_of_cells = -1;
    if (inV)
      {
      _offset += inV->GetNumberOfConnectivityEntries();
      _num_of_cells += inV->GetNumberOfCells();
      }
    offset[0] = _offset;
    numCells[0] = _num_of_cells;
    if (inL)
      {
      _offset += inL->GetNumberOfConnectivityEntries();
      _num_of_cells += inL->GetNumberOfCells();
      }
    offset[1] = _offset;
    numCells[1] = _num_of_cells;
    if (inP)
      {
      _offset += inP->GetNumberOfConnectivityEntries();
      _num_of_cells += inP->GetNumberOfCells();
      }
    offset[2] = _offset;
    numCells[2] = _num_of_cells;
    if (inS)
      {
      _offset += inS->GetNumberOfConnectivityEntries();
      _num_of_cells += inS->GetNumberOfCells();
      }
    offset[3] = _offset;
    numCells[3] = _num_of_cells;
    }

  void operator ()( vtkIdType id ) const
    {
    int loc = cellsInfo->GetCellLocation( id );
    vtkIdType *pSrc, *pDest, Num;
    if ( id > numCells[0] )
      {
      if ( id > numCells[1] )
        {
        if ( id > numCells[2] )
          {
          if ( id > numCells[3] )
            {
            cout << "Erreur" << endl;
            return;
            }
          else
            {
            loc -= offset[2];
            pSrc = inS->GetPointer() + loc;
            pDest = outS->GetPointer() + loc;
            }
          }
        else
          {
          loc -= offset[1];
          pSrc = inP->GetPointer() + loc;
          pDest = outP->GetPointer() + loc;
          }
        }
      else
        {
        loc -= offset[0];
        pSrc = inL->GetPointer() + loc;
        pDest = outL->GetPointer() + loc;
        }
      }
    else
      {
      pSrc = inV->GetPointer() + loc;
      pDest = outV->GetPointer() + loc;
      }
    Num = *pDest++ = *pSrc++;
    while (Num)
      {
      --Num;
      *pDest++ = *pSrc++;
      }
    outCd->CopyData( inCd, id, id );
    }

protected:
  vtkPolyData* cellsInfo;
  vtkCellArray *inV, *inL, *inP, *inS, *outV, *outL, *outP, *outS;
  vtkCellData *inCd, *outCd;
  vtkIdType offset[4];
  vtkIdType numCells[4];

  CellDistribute() : cellsInfo(0), inV(0), inL(0), inP(0), inS(0), outV(0), outL(0), outP(0), outS(0), inCd(0), outCd(0)
    {
    offset[0] = offset[1] = offset[2] = offset[3] = -1;
    numCells[0] = numCells[1] = numCells[2] = numCells[3] = 0;
    }

  ~CellDistribute()
    {
    if (inS) inS->UnRegister(this);
    if (outS) outS->UnRegister(this);
    if (inP) inP->UnRegister(this);
    if (outP) outP->UnRegister(this);
    if (inL) inL->UnRegister(this);
    if (outL) outL->UnRegister(this);
    if (inV) inV->UnRegister(this);
    if (outV) outV->UnRegister(this);
    if (inCd) inCd->UnRegister(this);
    if (outCd) outCd->UnRegister(this);
    if (cellsInfo) cellsInfo->UnRegister(this);
    }
};

vtkStandardNewMacro(CellDistribute);

vtkSMPDistributePolyData::vtkSMPDistributePolyData()
  {
  }

vtkSMPDistributePolyData::~vtkSMPDistributePolyData()
  {
  }

void vtkSMPDistributePolyData::PrintSelf(ostream &os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

int vtkSMPDistributePolyData::RequestData(vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  input->BuildCells();

  vtkPoints *newPts;
  vtkPoints *inPts = input->GetPoints();
  vtkIdType numPts;
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  if ( !inPts )
    {
    vtkErrorMacro(<<"No input data");
    return 1;
    }

  numPts = inPts->GetNumberOfPoints();

  newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(numPts);

  outPD->CopyAllocate(pd);

  // Loop over all points, updating position
  //
  PointDistribute* distributePoints = PointDistribute::New();
  distributePoints->Setup( inPts, newPts, pd, outPD );
  vtkSMPStaticForEachOp( 0, numPts, distributePoints );
  distributePoints->Delete();

  // Update ourselves and release memory
  //
  output->SetPoints(newPts);
  newPts->Delete();


  // Same for cells
  //

  vtkCellArray* inVerts = input->GetVerts();
  vtkCellArray* inLines = input->GetLines();
  vtkCellArray* inPolys = input->GetPolys();
  vtkCellArray* inStrips = input->GetStrips();
  vtkCellArray *newVerts, *newLines, *newPolys, *newStrips;
  vtkCellData *cd = input->GetCellData(), *outCD = output->GetCellData();
  newVerts = vtkCellArray::New();
  newVerts->Allocate(inVerts->GetSize());
  newVerts->SetNumberOfCells(inVerts->GetNumberOfCells());
  newLines = vtkCellArray::New();
  newLines->Allocate(inLines->GetSize());
  newLines->SetNumberOfCells(inLines->GetNumberOfCells());
  newPolys = vtkCellArray::New();
  newPolys->Allocate(inPolys->GetSize());
  newPolys->SetNumberOfCells(inPolys->GetNumberOfCells());
  newStrips = vtkCellArray::New();
  newStrips->Allocate(inStrips->GetSize());
  newStrips->SetNumberOfCells(inStrips->GetNumberOfCells());

  outCD->CopyAllocate(cd);

  CellDistribute* distributeCells = CellDistribute::New();
  distributeCells->Setup( inVerts, inLines, inPolys, inStrips, newVerts, newLines, newPolys, newStrips, cd, outCD, input );
  vtkSMPStaticForEachOp( 0, input->GetNumberOfCells(), distributeCells );
  distributeCells->Delete();

  // Let’s bring MaxId to their real values
  //
  newVerts->GetData()->WritePointer(inVerts->GetNumberOfConnectivityEntries(),0);
  newLines->GetData()->WritePointer(inLines->GetNumberOfConnectivityEntries(),0);
  newPolys->GetData()->WritePointer(inPolys->GetNumberOfConnectivityEntries(),0);
  newStrips->GetData()->WritePointer(inStrips->GetNumberOfConnectivityEntries(),0);

  output->SetVerts(newVerts);
  output->SetLines(newLines);
  output->SetPolys(newPolys);
  output->SetStrips(newStrips);

  newVerts->Delete();
  newLines->Delete();
  newPolys->Delete();
  newStrips->Delete();

  output->BuildCells();
  output->BuildLinks();

  return 1;
  }
