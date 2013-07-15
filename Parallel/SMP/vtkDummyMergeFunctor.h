/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDummyMergeFunctor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDummyMergeFunctor - ?
// .SECTION Description
// Used to initialize locator especially in the case of skip_threads

#ifndef _vtkDummyMergeFunctor_h_
#define _vtkDummyMergeFunctor_h_

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkFunctor.h"
#include "vtkThreadLocal.h"//TODO: Can this be removed somehow?

class vtkCellData;
class vtkCellArray;
class vtkIdList;
class vtkOffsetManager;
class vtkPointData;
class vtkPoints;
class vtkSMPMergePoints;

class VTKPARALLELSMP_EXPORT vtkDummyMergeFunctor : public vtkFunctor
{
public:
  vtkTypeMacro(vtkDummyMergeFunctor,vtkFunctor);
  static vtkDummyMergeFunctor* New();
  void PrintSelf(ostream &os, vtkIndent indent);

  //Description:
  //?
  void operator ()( vtkIdType pointId ) const;

  //TODO: Convention is that Get must have matching Set, so should
  //probably rename to Query
  //Description:
  //?
  vtkIdType GetNumberOfCells() const { return NumberOfCells; }
  vtkIdType GetNumberOfPoints() const { return NumberOfPoints; }

  //Description:
  //?
  void InitializeNeeds( vtkThreadLocal<vtkSMPMergePoints>* _locator,
                        vtkThreadLocal<vtkPoints>* _points,
                        vtkSMPMergePoints* _outlocator,
                        vtkThreadLocal<vtkCellArray>* _inverts,
                        vtkCellArray* _outverts,
                        vtkThreadLocal<vtkCellArray>* _inlines,
                        vtkCellArray* _outlines,
                        vtkThreadLocal<vtkCellArray>* _inpolys,
                        vtkCellArray* _outpolys,
                        vtkThreadLocal<vtkCellArray>* _instrips,
                        vtkCellArray* _outstrips,
                        vtkThreadLocal<vtkPointData>* _inpd,
                        vtkPointData* _outpd,
                        vtkThreadLocal<vtkCellData>* _incd,
                        vtkCellData* _outcd );

  //TODO: make private
  vtkDummyMergeFunctor ( const vtkDummyMergeFunctor& ); //Not implemented
  void operator =( const vtkDummyMergeFunctor& ); //Not implemented

  //TODO: Can be made private?
  vtkThreadLocal<vtkSMPMergePoints>* Locators;
  vtkThreadLocal<vtkPoints>* InPoints;

  vtkThreadLocal<vtkPointData>* InPd;
  vtkThreadLocal<vtkCellData>* InCd;

  vtkThreadLocal<vtkIdList>* Maps;

  vtkThreadLocal<vtkCellArray>* InVerts;
  vtkThreadLocal<vtkCellArray>* InLines;
  vtkThreadLocal<vtkCellArray>* InPolys;
  vtkThreadLocal<vtkCellArray>* InStrips;

  vtkSMPMergePoints* outputLocator;
  vtkCellArray* outputVerts;
  vtkCellArray* outputLines;
  vtkCellArray* outputPolys;
  vtkCellArray* outputStrips;
  vtkCellData* outputCd;
  vtkPointData* outputPd;

  vtkOffsetManager* vertOffset;
  vtkOffsetManager* lineOffset;
  vtkOffsetManager* polyOffset;
  vtkOffsetManager* stripOffset;

protected:
  vtkDummyMergeFunctor ();
  ~vtkDummyMergeFunctor ();

  vtkIdType NumberOfCells;
  vtkIdType NumberOfPoints;
};

#endif //_vtkDummyMergeFunctor_h_
