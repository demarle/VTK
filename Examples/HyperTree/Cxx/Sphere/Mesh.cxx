/*=========================================================================

Copyright (c) Kitware Inc.
All rights reserved.

=========================================================================*/
// This class was written by Daniel Aguilera and Philippe Pebay
// This work was supported by Commissariat a l'Energie Atomique (CEA/DIF)

const static char * MESH_CPP_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";

#include "Mesh.h"
#include "Node.h"
#include "Cell.h"

#include <assert.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCellType.h>

#include <vector>
#include <map>
using namespace std;

Mesh * Mesh::_instance = 0;

/*-------------------------------------------------------------------------
  service     : Constructeur
  description : 

  parametres  : 
  xnode (int) :
  ynode (int) :
  znode (int) :
  n1 (Node*) :
  n2 (Node*) :
  n3 (Node*) :
  n4 (Node*) :
  n5 (Node*) :
  n6 (Node*) :
  n7 (Node*) :
  n8 (Node*) :
  -----------------------------------------------------------------------*/
Mesh::Mesh (int xnode, int ynode, int znode, Node* n1, Node* n2, Node* n3, Node* n4, Node* n5, Node* n6, Node* n7, Node* n8)
{
  assert (_instance == 0);
  _instance = this;

  _lastCellId = 0;
  _lastNodeId = 0;
  _dataSet = 0;
  _refineNumber = 0;

  cout << "creation du maillage de niveau 0" << endl;

  this->createCells (xnode, ynode, znode, n1, n2, n3, n4, n5, n6, n7, n8);
}

/*-------------------------------------------------------------------------
  service     : Destructeur
  description : 

  parametres  : Aucun
  -----------------------------------------------------------------------*/
Mesh::~Mesh()
{
  if (_dataSet) _dataSet->Delete();
}

/*-------------------------------------------------------------------------
  service     : addCell
  description : 

  retour      : aucun

  parametres  : 
  c (Cell *) :
  -----------------------------------------------------------------------*/
void Mesh::addCell (Cell * c)
{
  _cells.push_back (c);
}

/*-------------------------------------------------------------------------
  service     : addNode
  description : 

  retour      : aucun

  parametres  : 
  n (Node *) :
  -----------------------------------------------------------------------*/
void Mesh::addNode (Node * n)
{
  _nodes.push_back (n);
}


/*-------------------------------------------------------------------------
  service     : getDataSet
  description : 

  retour (vtkDataSet *) : 

  parametres  : aucun
  -----------------------------------------------------------------------*/
vtkDataSet * Mesh::getDataSet()
{
  cout << "Generation du dataset" << endl;

  // creation du dataset
  if (_dataSet) _dataSet->Delete();
  vtkUnstructuredGrid * ug = vtkUnstructuredGrid::New();
  _dataSet = ug;

  // creation des points
  vtkPoints * points = vtkPoints::New();
  points->SetNumberOfPoints (_nodes.size());
  for (vector<Node*>::iterator it = _nodes.begin(); it != _nodes.end(); it++)
    {
    int id = (*it)->getId();
    if (id != -1) points->SetPoint (id, (*it)->getX(),(*it)->getY(),(*it)->getZ());
    }

  // affectation des points
  ug->SetPoints(points);
  points->Delete();

  int count = 0;
  // ajout des mailles
  for (vector<Cell*>::iterator it = _cells.begin(); it != _cells.end(); it++)
    {
    if (!(*it)->isRefined())
      {
      vtkIdType * ids = (*it)->getNodeIds();
      ug->InsertNextCell (VTK_HEXAHEDRON, 8, ids);
      count++;
      }
    }

  cout << "Generation du dataset terminee" << endl;
  return _dataSet;
}

/*-------------------------------------------------------------------------
  service     : getNextNodeId
  description : 

  retour (int) : 

  parametres  : aucun
  -----------------------------------------------------------------------*/
int Mesh::getNextNodeId()
{
  return _lastNodeId++;
}

/*-------------------------------------------------------------------------
  service     : getNextCellId
  description : 

  retour (int) : 

  parametres  : aucun
  -----------------------------------------------------------------------*/
int Mesh::getNextCellId()
{
  return _lastCellId++;
}


/*-------------------------------------------------------------------------
  service     : createCells
  description : 

  retour (vector<Cell*> &) : 

  parametres  : 
  xnode (int) :
  ynode (int) :
  znode (int) :
  n1 (Node*) :
  n2 (Node*) :
  n3 (Node*) :
  n4 (Node*) :
  n5 (Node*) :
  n6 (Node*) :
  n7 (Node*) :
  n8 (Node*) :
  -----------------------------------------------------------------------*/
vector<Cell*> & Mesh::createCells (int xnode, int ynode, int znode, 
                                   Node* n1, Node* n2, Node* n3, Node* n4, Node* n5, Node* n6, Node* n7, Node* n8, 
                                   Cell * fromCell)
{
  _lastCreatedCells.clear();

  int xm = xnode - 1;
  int ym = ynode - 1;
  int zm = znode - 1;

  vector<Node*> tempNodes;
   
#define chekNode(N) {if (N->getId()==-1) {this->addNode (N); N->setId(_lastNodeId++);} tempNodes.push_back(N);}

  // creation des noeuds
  for (int i = 0; i <= xm; i++)
    {
    for (int j = 0; j <= ym; j++)
      {
      for (int k = 0; k <= zm; k++)
        {
        if      (i == 0  && j == 0  && k == 0)  chekNode(n1)
        else if (i == xm && j == 0  && k == 0)  chekNode(n2)
        else if (i == xm && j == 0  && k == zm) chekNode(n3)
        else if (i == 0  && j == 0  && k == zm) chekNode(n4)
        else if (i == 0  && j == ym && k == 0)  chekNode(n5)
        else if (i == xm && j == ym && k == 0)  chekNode(n6)
        else if (i == xm && j == ym && k == zm) chekNode(n7)
        else if (i == 0  && j == ym && k == zm) chekNode(n8)
        else
          {
          // on calcule les coordonnees : pour l'instant on suppose qu'on a des parallelepipedes
          double x, y, z;
          x = n1->getX() + (double) i / (double) xm * (n2->getX() - n1->getX());
          y = n1->getY() + (double) j / (double) ym * (n5->getY() - n1->getY());
          z = n1->getZ() + (double) k / (double) zm * (n4->getZ() - n1->getZ());

          Node * n = new Node(_lastNodeId++, x, y, z);
          tempNodes.push_back(n);
          this->addNode (n);
          }
        }
      }
    }

  // creation des mailles
  for (int i = 0; i < xnode-1; i++)
    {
    for (int j = 0; j < ynode-1; j++)
      {
      for (int k = 0; k < znode-1; k++)
        {
        int id = i*ynode*znode + j*znode + k;
        vector<Node*> nodes;
        nodes.push_back (tempNodes[id]);
        nodes.push_back (tempNodes[id+1]);
        nodes.push_back (tempNodes[id+ynode*znode+1]);
        nodes.push_back (tempNodes[id+ynode*znode]);

        nodes.push_back (tempNodes[id+znode]);
        nodes.push_back (tempNodes[id+znode+1]);
        nodes.push_back (tempNodes[id+ynode*znode+znode+1]);
        nodes.push_back (tempNodes[id+ynode*znode+znode]);

        int idm = _lastCellId++;
        Cell * c = new Cell(idm, nodes);
        // ordre des voisines : xmin, xmax, ymin, ymax, zmin, zmax
        int idx1 = (i == 0 ? -1 : (i-1)*(ynode-1)*(znode-1) + j*(znode-1) + k);
        int idx2 = (i == xnode-1 ? -1 : (i+1)*(ynode-1)*(znode-1) + j*(znode-1) + k);
        int idy1 = (j == 0 ? -1 : i*(ynode-1)*(znode-1) + (j-1)*(znode-1) + k);
        int idy2 = (j == ynode-1 ? -1 : i*(ynode-1)*(znode-1) + (j+1)*(znode-1) + k);
        int idz1 = (k == 0 ? -1 : i*(ynode-1)*(znode-1) + j*(znode-1) + k-1);
        int idz2 = (k == znode-1 ? -1 : i*(ynode-1)*(znode-1) + j*(znode-1) + k+1);
            
        c->setNeighbours(idx1, idx2, idy1, idy2, idz1, idz2);

        _lastCreatedCells.push_back(c);
        this->addCell (c);

        }
      }
    }
   
  return _lastCreatedCells;
}

/*-------------------------------------------------------------------------
  service     : setRefine
  description : 

  retour      : aucun

  parametres  : 
  refine (int) :
  -----------------------------------------------------------------------*/
void Mesh::setRefine (int refine)
{
  _refineNumber = refine;
  Cell::setRefine (_refineNumber);
}
/*-------------------------------------------------------------------------
  service     : refine
  description : 

  retour      : aucun

  parametres  : aucun
  -----------------------------------------------------------------------*/
void Mesh::refine()
{
  assert (_refineNumber != 0);

  cout << "Raffinement d'un niveau" << endl;

  vector<Cell*> tempCells = _cells;

  // on itere sur toutes les mailles
  for (vector<Cell*>::iterator it = tempCells.begin(); it != tempCells.end(); it++)
    {
    (*it)->refineIfNeeded();
    }
}

/*-------------------------------------------------------------------------
  service     : mergePoints
  description : 

  retour      : aucun

  parametres  : aucun
  -----------------------------------------------------------------------*/
void Mesh::mergePoints ()
{
  cout << "Suppression des points multiples" << endl;

  map<double, map<double , map<double, Node * > > > nodesMap;

  for (vector<Node*>::iterator it = _nodes.begin(); it != _nodes.end(); it++)
    {
    Node * n = (*it);
    double x = n->getX();
    double y = n->getY();
    double z = n->getZ();

    bool nodeExist = false;

    map<double, map<double , map<double, Node *> > >::iterator itx = nodesMap.find (x);
    if (itx != nodesMap.end())
      {
      map<double , map<double, Node *> > xmap = itx->second;
      map<double , map<double, Node *> >::iterator ity = xmap.find (y);
 
      if (ity != xmap.end())
        {
        map<double, Node *> ymap = ity->second;
        map<double, Node *>::iterator itz = ymap.find (z);

        if (itz != ymap.end())
          {
          // on a trouve un meme noeud
          Node * nodeInMap = itz->second;
               
          if (nodeInMap != n)
            {
            // remplacement de n par nodeInMap
            n->replaceBy (nodeInMap);
            n->setId (-1);
            nodeExist = true;
            }
          }
        }
      }

    if (!nodeExist)
      {
      nodesMap[x][y][z] = n;
      }
    }
}
