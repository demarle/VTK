/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkUnivariateStatisticsAlgorithmPrivate.h

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
// .NAME vtkDescriptiveStatistics - Private implementation for univariate
// statistics algorithms.
//
// .SECTION Description
// The main purpose of this class is to avoid exposure of STL container
// through the APIs of the vtkStatistics classes APIs
//
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkUnivariateStatisticsAlgorithmPrivate_h
#define __vtkUnivariateStatisticsAlgorithmPrivate_h

#include "vtkType.h"

#include <vtkstd/set> // used to iterate over internal organs

class vtkUnivariateStatisticsAlgorithmPrivate
{
public:
  vtkUnivariateStatisticsAlgorithmPrivate()
    {
    }
  ~vtkUnivariateStatisticsAlgorithmPrivate()
    {
    }
  
  vtkstd::set<vtkIdType> Columns;
};

#endif // __vtkUnivariateStatisticsAlgorithmPrivate_h
