/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPythonUtil.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtkPythonUtil_h
#define __vtkPythonUtil_h

#include "vtkPython.h"
#include "PyVTKClass.h"
#include "PyVTKObject.h"
#include "PyVTKSpecialObject.h"

class vtkPythonObjectMap;
class vtkPythonClassMap;
class vtkPythonSpecialTypeMap;

extern "C" void vtkPythonUtilDelete();

class VTK_PYTHON_EXPORT vtkPythonUtil
{
public:
  // Description:
  // This is a special version of ParseTuple that handles both bound
  // and unbound method calls for VTK objects, depending on whether
  // "self" is a PyVTKObject or a PyVTKClass.
  static vtkObjectBase *VTKParseTuple(PyObject *self, PyObject *args,
                                      char *format, ...);

  // Description:
  // Check python object against a format character and return a number
  // to indicate how well it matches (lower numbers are better).
  static int CheckArg(PyObject *arg, const char *format,
                      const char *classname, int level=0);

  // Description:
  // Call the method that is the best match for the for the provided
  // arguments.  The docstrings in the PyMethodDef must provide info
  // about the argument types for each method.
  static PyObject *CallOverloadedMethod(PyMethodDef *methods,
                                        PyObject *self, PyObject *args);

  // Description:
  // Find a method that takes the single arg provided, this is used
  // to locate the correct constructor signature for a conversion.
  // The docstrings in the PyMethodDef must provide info about the
  // argument types for each method.
  static PyMethodDef *FindConversionMethod(PyMethodDef *methods,
                                           PyObject *arg);

  // Description:
  // Add a PyVTKClass to the type lookup table, this allows us to later
  // create object given only the class name.
  static void AddClassToMap(PyObject *obj, const char *classname);

  // Description:
  // Get information about a special VTK type, given the type name.
  static PyObject *FindClass(const char *classname);

  // Description:
  // For an VTK object whose class is not in the ClassMap, search
  // the whole ClassMap to find out which class is the closest base
  // class of the object.  Returns a PyVTKClass.
  static PyObject *FindNearestBaseClass(vtkObjectBase *ptr);

  // Description:
  // Extract the vtkObjectBase from a PyVTKObject.  If the PyObject is
  // not a PyVTKObject, or is not a PyVTKObject of the specified type,
  // the python error indicator will be set.
  // Special behavior: Py_None is converted to NULL without no error.
  static vtkObjectBase *GetPointerFromObject(PyObject *obj,
                                             const char *classname);

  // Description:
  // Convert a vtkObjectBase to a PyVTKObject.  This will first check to
  // see if the PyVTKObject already exists, and create a new PyVTKObject
  // if necessary.  This function also passes ownership of the reference
  // to the PyObject.
  // Special behaviour: NULL is converted to Py_None.
  static PyObject *GetObjectFromPointer(vtkObjectBase *ptr);

  // Description:
  // Try to convert some PyObject into a PyVTKObject, currently conversion
  // is supported for SWIG-style mangled pointer strings.
  static PyObject *GetObjectFromObject(PyObject *arg, const char *type);

  // Description:
  // Add PyVTKObject/vtkObjectBase pairs to the internal mapping.
  // This methods do not change the reference counts of either the
  // vtkObjectBase or the PyVTKObject.
  static void AddObjectToMap(PyObject *obj, vtkObjectBase *anInstance);

  // Description:
  // Remove a PyVTKObject from the internal mapping.  No reference
  // counts are changed.
  static void RemoveObjectFromMap(PyObject *obj);

  // Description:
  // Add a special VTK type to the type lookup table, this allows us to
  // later create object given only the class name.
  static PyVTKSpecialType *AddSpecialTypeToMap(
    char *classname, char *docstring[], PyMethodDef *methods,
    PyMethodDef *constructors, PyVTKSpecialCopyFunc copyfunc,
    PyVTKSpecialDeleteFunc deletefunc, PyVTKSpecialPrintFunc printfunc);

  // Description:
  // Get information about a special VTK type, given the type name.
  static PyVTKSpecialType *FindSpecialType(const char *classname);

  // Description:
  // Given a PyObject, convert it into a "result_type" object, where
  // "result_type" must have been wrapped.  The C object is returned
  // as a void *, while the python object is returned in "newobj" unless
  // the original object was already of the correct type, in which case
  // newobj is set to NULL.  If a python exception was raised, NULL will be
  // returned.
  static void *GetPointerFromSpecialObject(
    PyObject *obj, const char *result_type, PyObject **newobj);

  // Description:
  // Convert a pointer to an object of special wrapped type "class_type"
  // into a PyObject of that type.  If the given pointer is NULL, then
  // Py_None will be returned with no error.  If the given pointer is
  // of the wrong type, expect fireworks.
  static PyObject *GetSpecialObjectFromPointer(void *ptr,
    const char *class_type);

  // Description:
  // Utility function to build a docstring by concatenating a series
  // of strings until a null string is found.
  static PyObject *BuildDocString(char *docstring[]);

  // Description:
  // Utility function for creating SWIG-style mangled pointer string.
  static char *ManglePointer(void *ptr, const char *type);

  // Description:
  // Utility function decoding a SWIG-style mangled pointer string.
  static void *UnmanglePointer(char *ptrText, int *len, const char *type);

  // Description:
  // check array arguments sent through the wrappers to see if the
  // underlying C++ method changed the values, and attempt to modify
  // the original python sequence (list or tuple) if so.
  static int CheckArray(PyObject *args, int i, char *a, int n);
  static int CheckArray(PyObject *args, int i, signed char *a, int n);
  static int CheckArray(PyObject *args, int i, unsigned char *a, int n);
  static int CheckArray(PyObject *args, int i, short *a, int n);
  static int CheckArray(PyObject *args, int i, unsigned short *a, int n);
  static int CheckArray(PyObject *args, int i, int *a, int n);
  static int CheckArray(PyObject *args, int i, unsigned int *a, int n);
  static int CheckArray(PyObject *args, int i, long *a, int n);
  static int CheckArray(PyObject *args, int i, unsigned long *a, int n);
  static int CheckArray(PyObject *args, int i, float *a, int n);
  static int CheckArray(PyObject *args, int i, double *a, int n);
#if defined(VTK_TYPE_USE_LONG_LONG)
  static int CheckArray(PyObject *args, int i, long long *a, int n);
  static int CheckArray(PyObject *args, int i, unsigned long long *a, int n);
#endif
#if defined(VTK_TYPE_USE___INT64)
  static int CheckArray(PyObject *args, int i, __int64 *a, int n);
  static int CheckArray(PyObject *args, int i, unsigned __int64 *a, int n);
#endif

private:
  vtkPythonUtil();
  ~vtkPythonUtil();
  vtkPythonUtil(const vtkPythonUtil&);  // Not implemented.
  void operator=(const vtkPythonUtil&);  // Not implemented.

  vtkPythonObjectMap *ObjectMap;
  vtkPythonClassMap *ClassMap;
  vtkPythonSpecialTypeMap *SpecialTypeMap;

  friend void vtkPythonUtilDelete();
};

// For use by SetXXMethod() , SetXXMethodArgDelete()
extern VTK_PYTHON_EXPORT void vtkPythonVoidFunc(void *);
extern VTK_PYTHON_EXPORT void vtkPythonVoidFuncArgDelete(void *);

// The following macro is used to supress missing initializer
// warnings.  Python documentation says these should not be necessary.
// We define it as a macro in case the length needs to change across
// python versions.
#if   PY_VERSION_HEX >= 0x02060000 // for tp_version_tag
#define VTK_PYTHON_UTIL_SUPRESS_UNINITIALIZED \
  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0, 0,
#elif   PY_VERSION_HEX >= 0x02030000
#define VTK_PYTHON_UTIL_SUPRESS_UNINITIALIZED \
  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
#elif PY_VERSION_HEX >= 0x02020000
#define VTK_PYTHON_UTIL_SUPRESS_UNINITIALIZED \
  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
#else
#define VTK_PYTHON_UTIL_SUPRESS_UNINITIALIZED
#endif

#if PY_VERSION_HEX < 0x02050000
  typedef int Py_ssize_t;
#endif

#endif
