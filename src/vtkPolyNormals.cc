/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPolyNormals.cc
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkPolyNormals.hh"
#include "vtkPolygon.hh"
#include "vtkMath.hh"
#include "vtkFloatNormals.hh"

// Description:
// Construct with feature angle=30, splitting and consistency turned on, 
// and flipNormals turned off.
vtkPolyNormals::vtkPolyNormals()
{
  this->FeatureAngle = 30.0;
  this->Splitting = 1;
  this->Consistency = 1;
  this->FlipNormals = 0;
  this->MaxRecursionDepth = 10000;
}

static  int NumFlips=0, NumExceededMaxDepth=0;
static  int *Visited;
static  vtkPolyData *OldMesh, *NewMesh;
static  int RecursionDepth;
static  int Mark;    
static  vtkFloatNormals *PolyNormals;
static	float	CosAngle;
static  vtkIdList *Seeds, *Map;

// Generate normals for polygon meshes

void vtkPolyNormals::Execute()
{
  int i, j;
  int *pts, npts;
  int numNewPts;
  float *polyNormal, *vertNormal, length;
  float flipDirection=1.0;
  int replacementPoint, numPolys;
  int cellId, numPts;
  vtkPoints *inPts;
  vtkCellArray *inPolys;
  vtkPolygon poly;
  vtkMath math;
  vtkFloatPoints *newPts;
  vtkFloatNormals *newNormals;
  vtkPointData *pd, *outPD;
  float n[3];
  vtkCellArray *newPolys;
  int ptId, oldId;
  vtkIdList cellIds(VTK_CELL_SIZE);
  vtkPolyData *input=(vtkPolyData *)this->Input;
  vtkPolyData *output=(vtkPolyData *)this->Output;

  vtkDebugMacro(<<"Generating surface normals");

  if ( (numPts=input->GetNumberOfPoints()) < 1 || 
  (numPolys=input->GetNumberOfPolys()) < 1 )
    {
    vtkErrorMacro(<<"No data to generate normals for!");
    return;
    }
//
// Load data into cell structure.  We need two copies: one is a 
// non-writable mesh used to perform topological queries.  The other 
// is used to write into and modify the connectivity of the mesh.
//
  inPts = input->GetPoints();
  inPolys = input->GetPolys();

  OldMesh = new vtkPolyData;
  OldMesh->SetPoints(inPts);
  OldMesh->SetPolys(inPolys);
  OldMesh->BuildLinks();
  
  pd = input->GetPointData();
  outPD = output->GetPointData();
    
  NewMesh = new vtkPolyData;
  NewMesh->SetPoints(inPts);
  // create a copy because we're modifying it
  newPolys = new vtkCellArray(*(inPolys));
  NewMesh->SetPolys(newPolys);
  NewMesh->BuildCells();

//
// The visited array keeps track of which polygons have been visited.
//
  if ( this->Consistency || this->Splitting ) 
    {
    Visited = new int[numPolys];
    for ( i=0; i < numPolys; i++) Visited[i] = 0;
    Mark = 1;
    }
  else
    {
    Visited = NULL;
    }
//
//  Traverse all elements insuring proper direction of ordering.  This
//  is a recursive neighbor search.  Note: have to truncate recursion
//  and keep track of seeds to start up again.
//
  if ( this->Consistency ) 
    {    
    NumFlips = 0;
    Seeds = new vtkIdList(1000,1000);

    for (cellId=0; cellId < numPolys; cellId++)
      {
      if ( ! Visited[cellId] ) 
        {
        if ( this->FlipNormals ) 
          {
          NumFlips++;
          NewMesh->ReverseCell(cellId);
          }
        RecursionDepth = 0;
        this->TraverseAndOrder(cellId);
        }

      for (i=0; i < Seeds->GetNumberOfIds(); i++) 
        {
        RecursionDepth = 0;
        this->TraverseAndOrder (Seeds->GetId(i));
        }

      Seeds->Reset();
      }
    vtkDebugMacro(<<"Reversed ordering of " << NumFlips << " polygons");
    vtkDebugMacro(<<"Exceeded recursion depth " << NumExceededMaxDepth
                 <<" times");

    Seeds->Delete();
    }
//
//  Compute polygon normals
//
  PolyNormals = new vtkFloatNormals(numPolys);

  for (cellId=0, newPolys->InitTraversal(); newPolys->GetNextCell(npts,pts); 
  cellId++ )
    {
    poly.ComputeNormal(inPts, npts, pts, n);
    PolyNormals->SetNormal(cellId,n);
    }
//
//  Traverse all nodes; evaluate loops and feature edges.  If feature
//  edges found, split mesh creating new nodes.  Update element connectivity.
//
  if ( this->Splitting ) 
    {
    CosAngle = cos ((double) math.DegreesToRadians() * this->FeatureAngle);
//
//  Splitting will create new points.  Have to create index array to map
//  new points into old points.
//
    Map = new vtkIdList(numPts,numPts);
    for (i=0; i < numPts; i++) Map->SetId(i,i);

    for (ptId=0; ptId < OldMesh->GetNumberOfPoints(); ptId++)
      {
      Mark++;
      replacementPoint = ptId;
      OldMesh->GetPointCells(ptId,cellIds);
      for (j=0; j < cellIds.GetNumberOfIds(); j++)
        {
        if ( Visited[cellIds.GetId(j)] != Mark )
          this->MarkAndReplace (cellIds.GetId(j), ptId, replacementPoint);

        replacementPoint = Map->GetNumberOfIds();
        }
      }

    Map->Squeeze();
    numNewPts = Map->GetNumberOfIds();

    vtkDebugMacro(<<"Created " << numNewPts-numPts << " new points");
//
//  Now need to map values of old points into new points.
//
    outPD->CopyNormalsOff();
    outPD->CopyAllocate(pd,numNewPts);

    newPts = new vtkFloatPoints(numNewPts);
    for (ptId=0; ptId < numNewPts; ptId++)
      {
      oldId = Map->GetId(ptId);
      newPts->SetPoint(ptId,inPts->GetPoint(oldId));
      outPD->CopyData(pd,oldId,ptId);
      }
    Map->Delete();
    } 
  else //no splitting
    {
    numNewPts = numPts;
    }
//
//  Finally, traverse all elements, computing polygon normals and
//  accumalating them at the vertices.
//
  if ( Visited ) delete [] Visited;

  if ( this->FlipNormals && ! this->Consistency ) flipDirection = -1.0;

  newNormals = new vtkFloatNormals(numNewPts);
  n[0] = n[1] = n[2] = 0.0;
  for (i=0; i < numNewPts; i++) newNormals->SetNormal(i,n);

  for (cellId=0, newPolys->InitTraversal(); newPolys->GetNextCell(npts,pts); 
  cellId++ )
    {
    polyNormal = PolyNormals->GetNormal(cellId);

    for (i=0; i < npts; i++) 
      {
      vertNormal = newNormals->GetNormal(pts[i]);
      for (j=0; j < 3; j++) n[j] = vertNormal[j] + polyNormal[j];
      newNormals->SetNormal(pts[i],n);
      }
    }

  for (i=0; i < numNewPts; i++) 
    {
    vertNormal = newNormals->GetNormal(i);
    length = math.Norm(vertNormal);
    if (length != 0.0) 
      {
      for (j=0; j < 3; j++) n[j] = vertNormal[j] / length * flipDirection;
      }
    newNormals->SetNormal(i,n);
    }
  PolyNormals->Delete();
//
//  Update ourselves.  If no new nodes have been created (i.e., no
//  splitting), can simply pass data through.
//
  if ( ! this->Splitting ) 
    {
    output->SetPoints(inPts);
    }
//
//  If there is splitting, then have to send down the new data.
//
  else
    {
    output->SetPoints(newPts);
    newPts->Delete();
    }

  outPD->SetNormals(newNormals);
  newNormals->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();

  OldMesh->Delete();
  NewMesh->Delete();
}

//
//  Mark current polygon as visited, make sure that all neighboring
//  polygons are ordered consistent with this one.
//
void vtkPolyNormals::TraverseAndOrder (int cellId)
{
  int p1, p2;
  int j, k, l;
  int npts, *pts;
  vtkIdList cellIds(VTK_CELL_SIZE);
  int numNeiPts, *neiPts, neighbor;

  Visited[cellId] = Mark; //means that it's been ordered properly

  if ( RecursionDepth++ > this->MaxRecursionDepth ) 
    {
    Seeds->InsertNextId(cellId);
    NumExceededMaxDepth++;
    return;
    }

  NewMesh->GetCellPoints(cellId, npts, pts);

  for (j=0; j < npts; j++) 
    {
    p1 = pts[j];
    p2 = pts[(j+1)%npts];

    OldMesh->GetCellEdgeNeighbors(cellId, p1, p2, cellIds);
//
//  Check the direction of the neighbor ordering.  Should be
//  consistent with us (i.e., if we are n1->n2, neighbor should be n2->n1).
//
    for (k=0; k < cellIds.GetNumberOfIds(); k++) 
      {
      if ( ! Visited[cellIds.GetId(k)] ) 
        {
        neighbor = cellIds.GetId(k);
        NewMesh->GetCellPoints(neighbor,numNeiPts,neiPts);
        for (l=0; l < numNeiPts; l++)
          if (neiPts[l] == p2)
             break;
//
//  Have to reverse ordering if neighbor not consistent
//
         if ( neiPts[(l+1)%numNeiPts] != p1 ) 
           {
           NumFlips++;
           NewMesh->ReverseCell(neighbor);
           }
         this->TraverseAndOrder (neighbor);
        }
      } // for each edge neighbor
    } // for all edges of this polygon

  RecursionDepth--;
  return;
}

//
//  Mark polygons around vertex.  Create new vertex (if necessary) and
//  replace (i.e., split mesh).
//
void vtkPolyNormals::MarkAndReplace (int cellId, int n, int replacementPoint)
{
  int i, spot;
  int neiNode[2];
  float *thisNormal, *neiNormal;
  int numOldPts, *oldPts;
  int numNewPts, *newPts;
  vtkIdList cellIds(VTK_CELL_SIZE);
  vtkMath math;

  Visited[cellId] = Mark;
  OldMesh->GetCellPoints(cellId,numOldPts,oldPts);
//
//  Replace the node if necessary
//
  if ( n != replacementPoint ) 
    {
    Map->InsertId(replacementPoint, n);

    NewMesh->GetCellPoints(cellId,numNewPts,newPts);
    for (i=0; i < numNewPts; i++) 
      {
      if ( newPts[i] == n ) 
        {
        newPts[i] = replacementPoint; // this is very nasty! direct write!
        break;
        }
      }
    }
//
//  Look at neighbors who share central point and see whether a
//  feature edge separates us.  If not, can recursively call this
//  routine. 
//
  for (spot=0; spot < numOldPts; spot++)
    if ( oldPts[spot] == n )
      break;

  if ( spot == 0 ) 
    {
    neiNode[0] = oldPts[spot+1];
    neiNode[1] = oldPts[numOldPts-1];
    } 
  else if ( spot == (numOldPts-1) ) 
    {
    neiNode[0] = oldPts[spot-1];
    neiNode[1] = oldPts[0];
    } 
  else 
    {
    neiNode[0] = oldPts[spot+1];
    neiNode[1] = oldPts[spot-1];
    }

  for (i=0; i<2; i++) 
    {
    OldMesh->GetCellEdgeNeighbors(cellId, n, neiNode[i], cellIds);
    if ( cellIds.GetNumberOfIds() == 1 && Visited[cellIds.GetId(0)] != Mark ) 
      {
      thisNormal = PolyNormals->GetNormal(cellId);
      neiNormal =  PolyNormals->GetNormal(cellIds.GetId(0));

      if ( math.Dot(thisNormal,neiNormal) > CosAngle )
        this->MarkAndReplace (cellIds.GetId(0), n, replacementPoint);
      }
    }

  return;
}

void vtkPolyNormals::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyToPolyFilter::PrintSelf(os,indent);

  os << indent << "Feature Angle: " << this->FeatureAngle << "\n";
  os << indent << "Splitting: " << (this->Splitting ? "On\n" : "Off\n");
  os << indent << "Consistency: " << (this->Consistency ? "On\n" : "Off\n"); 
  os << indent << "Flip Normals: " << (this->FlipNormals ? "On\n" : "Off\n");
  os << indent << "Maximum Recursion Depth: " << this->MaxRecursionDepth << "\n";
}

