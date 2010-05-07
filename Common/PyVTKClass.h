/*=========================================================================

  Program:   Visualization Toolkit
  Module:    PyVTKClass.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __PyVTKClass_h
#define __PyVTKClass_h

#include "vtkPython.h"

// Prototype for static constructor
class vtkObjectBase;
typedef vtkObjectBase *(*vtknewfunc)();

// This is the VTK/Python 'class,' it contains the method list and a pointer
// to the superclass

struct PyVTKClass {
  PyObject_HEAD
  // the first six are common to PyClassObject
  PyObject *vtk_bases;
  PyObject *vtk_dict;
  PyObject *vtk_name;
  PyObject *vtk_getattr;
  PyObject *vtk_setattr;
  PyObject *vtk_delattr;
  // these are unique to the PyVTKClass
  PyObject *vtk_module;
  PyObject *vtk_doc;
  PyMethodDef *vtk_methods;
  vtknewfunc vtk_new;
};

extern "C"
{
VTK_PYTHON_EXPORT
int PyVTKClass_Check(PyObject *obj);

VTK_PYTHON_EXPORT
PyObject *PyVTKClass_New(vtknewfunc constructor, PyMethodDef *methods,
                         char *classname, char *modulename, char *docstring[],
                         PyObject *base);
}

#endif
