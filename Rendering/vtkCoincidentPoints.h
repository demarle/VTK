/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCoincidentPoints.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkCoincidentPoints - contains an octree of labels
//
// .SECTION Description
// This class provides a collection of points that is organized such that 
// each coordinate is stored with a set of point id's of points that are 
// all coincident.

#ifndef __vtkCoincidentPoints_h
#define __vtkCoincidentPoints_h

#include "vtkObject.h"

class vtkIdList;
class vtkPoints;

class VTK_RENDERING_EXPORT vtkCoincidentPoints : public vtkObject
{
public:
  static vtkCoincidentPoints* New();
  vtkTypeRevisionMacro(vtkCoincidentPoints,vtkObject);
  virtual void PrintSelf( ostream& os, vtkIndent indent );

  // Description:
  // Accumulates a set of Ids in a map where the point coordinate
  // is the key. All Ids in a given map entry are thus coincident.
  // @param Id - a unique Id for the given \a point that will be stored in an vtkIdList.
  // @param[in] point - the point coordinate that we will store in the map to test if any other points are
  // coincident with it.
  void AddPoint(vtkIdType Id, const double point[3]);

  // Description:
  // Retrieve the list of point Ids that are coincident with the given \a point.
  // @param[in] point - the coordinate of coincident points we want to retrieve.
  vtkIdList * GetCoincidentPointIds(const double point[3]);

  // Description:
  // Used to iterate the sets of coincident points within the map.
  // InitTraversal must be called first or NULL will always be returned.
  vtkIdList * GetNextCoincidentPointIds();

  // Description
  // Initialize iteration to the begining of the coincident point map.
  void InitTraversal();

  // Description
  // Iterate through all added points and remove any entries that have
  // no coincident points (only a single point Id).
  void RemoveNonCoincidentPoints();

  //BTX
  class implementation;

  implementation * GetImplementation() { return this->Implementation; }
  //ETX

protected:
  vtkCoincidentPoints();
  virtual ~vtkCoincidentPoints();

private:
  vtkCoincidentPoints( const vtkCoincidentPoints& ); // Not implemented.
  void operator = ( const vtkCoincidentPoints& ); // Not implemented.

  //BTX
  implementation* Implementation;

  friend class implementation;
  //ETX
};

#endif // __vtkCoincidentPoints_h
