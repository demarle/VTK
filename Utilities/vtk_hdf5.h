/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtk_zlib.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtk_hdf5_h
#define __vtk_hdf5_h

/* Use the zlib library configured for VTK.  */
#include "vtkToolkits.h"
#ifdef VTK_USE_HDF5_ZLIB
# include <hdf5.h>
#else
# include <vtkhdf5/hdf5.h>
#endif

#endif
