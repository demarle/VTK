/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProcessObject.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkProcessObject - abstract class specifies interface for visualization filters
//
// .SECTION Description
// vtkProcessObject is an abstract object that specifies behavior and
// interface of visualization network process objects (sources, filters,
// mappers). Source objects are creators of visualization data; filters
// input, process, and output visualization data; and mappers transform data
// into another form (like rendering primitives or write data to a file).
//
// vtkProcessObject provides a mechanism for invoking the methods
// StartMethod() and EndMethod() before and after object execution (via
// Execute()). These are convenience methods you can use for any purpose
// (e.g., debugging info, highlighting/notifying user interface, etc.) These
// methods accept a single void* pointer that can be used to send data to the
// methods. It is also possible to specify a function to delete the argument
// via StartMethodArgDelete and EndMethodArgDelete.
//
// Another method, ProgressMethod() can be specified. Some filters invoke this 
// method periodically during their execution. The use is similar to that of 
// StartMethod() and EndMethod(). Filters may also check their AbortExecute
// flag to determine whether to prematurely end their execution.
//
// An important feature of subclasses of vtkProcessObject is that it is
// possible to control the memory-management model (i.e., retain output
// versus delete output data). If enabled the ReleaseDataFlag enables the
// deletion of the output data once the downstream process object finishes
// processing the data (please see text).  
//
// .SECTION See Also
// vtkDataObject vtkSource vtkFilter vtkMapper vtkWriter

#ifndef __vtkProcessObject_h
#define __vtkProcessObject_h

#include "vtkDataObject.h"

class VTK_COMMON_EXPORT vtkProcessObject : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkProcessObject,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify function to be called before object executes.
  void SetStartMethod(void (*f)(void *), void *arg);

  // Description:
  // Specify function to be called to show progress of filter
  void SetProgressMethod(void (*f)(void *), void *arg);

  // Description:
  // Specify function to be called after object executes.
  void SetEndMethod(void (*f)(void *), void *arg);

  // Description:
  // Set the arg delete method. This is used to free user memory.
  void SetStartMethodArgDelete(void (*f)(void *));

  // Description:
  // Set the arg delete method. This is used to free user memory.
  void SetProgressMethodArgDelete(void (*f)(void *));

  // Description:
  // Set the arg delete method. This is used to free user memory.
  void SetEndMethodArgDelete(void (*f)(void *));

  // Description:
  // Set/Get the AbortExecute flag for the process object. Process objects
  // may handle premature termination of execution in different ways.
  vtkSetMacro(AbortExecute,int);
  vtkGetMacro(AbortExecute,int);
  vtkBooleanMacro(AbortExecute,int);

  // Description:
  // Set/Get the execution progress of a process object.
  vtkSetClampMacro(Progress,float,0.0,1.0);
  vtkGetMacro(Progress,float);

  // Description:
  // Update the progress of the process object. If a ProgressMethod exists,
  // executes it.  Then set the Progress ivar to amount. The parameter amount
  // should range between (0,1).
  void UpdateProgress(float amount);

  // Description:
  // Set the current text message associated with the progress state.
  // This may be used by a calling process/GUI.
  vtkSetStringMacro(ProgressText);
  vtkGetStringMacro(ProgressText);

  // left public for performance since it is used in inner loops
  int AbortExecute;

  // Description:
  // Return an array with all the inputs of this process object.
  // This is useful for tracing back in the pipeline to construct
  // graphs etc.
  vtkDataObject **GetInputs() {return this->Inputs;}
  vtkGetMacro(NumberOfInputs,int);

  // Description:
  // This method will rearrange the input array so that all NULL entries 
  // are removed.
  void SqueezeInputArray();
  
  // Description:
  // Remove all the input data.
  void RemoveAllInputs();

protected:
  vtkProcessObject();
  ~vtkProcessObject();

  // Progress/Update handling
  unsigned long StartTag;
  unsigned long ProgressTag;
  unsigned long EndTag;
  float Progress;
  char  *ProgressText;

  int NumberOfInputs;
  int NumberOfRequiredInputs;
  vtkDataObject **Inputs;     //An array of the inputs to the filter
  // Temporary arrays used internally.  
  // It is only valid after SortInputsByLocality is called.
  vtkDataObject **SortedInputs;   // Inputs sorted by locality
  // We need a second array for an effficeint search.  
  // This array is never valid.
  vtkDataObject **SortedInputs2;   
  void SortInputsByLocality();
  // A helper method for quicksort.
  void SortMerge(vtkDataObject **a1, int l1,
                 vtkDataObject **a2, int l2,
                 vtkDataObject **results);

  // Called to allocate the input array.  Copies old inputs.
  void SetNumberOfInputs(int num);

  // protected methods for setting inputs.
  virtual void SetNthInput(int num, vtkDataObject *input);
  virtual void AddInput(vtkDataObject *input);
  virtual void RemoveInput(vtkDataObject *input);
  
private:
  vtkProcessObject(const vtkProcessObject&);  // Not implemented.
  void operator=(const vtkProcessObject&);  // Not implemented.
};

#endif

