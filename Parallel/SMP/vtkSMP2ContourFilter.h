/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMP2ContourFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMP2ContourFilter - !!!!
// .SECTION Description
// vtkSMP2ContourFilter !!!!

// .SECTION See Also
// vtkContourFilter

#ifndef __vtkSMP2ContourFilter_h
#define __vtkSMP2ContourFilter_h

#include "vtkParallelSMPModule.h" // For export macro
#include "vtkSMPPolyDataAlgorithm.h"

#include "vtkContourValues.h" // Needed for inline methods

class vtkIncrementalPointLocator;
class vtkScalarTree;
class vtkSynchronizedTemplates2D;
class vtkSynchronizedTemplates3D;
class vtkGridSynchronizedTemplates3D;
class vtkRectilinearSynchronizedTemplates;

template <class T> class vtkThreadLocal;

class VTKPARALLELSMP_EXPORT vtkSMP2ContourFilter : public vtkSMPPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkSMP2ContourFilter,vtkSMPPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with initial range (0,1) and single contour value
  // of 0.0.
  static vtkSMP2ContourFilter *New();

  // Description:
  // Methods to set / get contour values.
  void SetValue(int i, double value);
  double GetValue(int i);
  double *GetValues();
  void GetValues(double *contourValues);
  void SetNumberOfContours(int number);
  int GetNumberOfContours();
  void GenerateValues(int numContours, double range[2]);
  void GenerateValues(int numContours, double rangeStart, double rangeEnd);

  // Description:
  // Modified GetMTime Because we delegate to vtkContourValues
  unsigned long GetMTime();

  // Description:
  // Set/Get the computation of normals. Normal computation is fairly
  // expensive in both time and storage. If the output data will be
  // processed by filters that modify topology or geometry, it may be
  // wise to turn Normals and Gradients off.
  vtkSetMacro(ComputeNormals,int);
  vtkGetMacro(ComputeNormals,int);
  vtkBooleanMacro(ComputeNormals,int);

  // Description:
  // Set/Get the computation of gradients. Gradient computation is
  // fairly expensive in both time and storage. Note that if
  // ComputeNormals is on, gradients will have to be calculated, but
  // will not be stored in the output dataset.  If the output data
  // will be processed by filters that modify topology or geometry, it
  // may be wise to turn Normals and Gradients off.
  vtkSetMacro(ComputeGradients,int);
  vtkGetMacro(ComputeGradients,int);
  vtkBooleanMacro(ComputeGradients,int);

  // Description:
  // Set/Get the computation of scalars.
  vtkSetMacro(ComputeScalars,int);
  vtkGetMacro(ComputeScalars,int);
  vtkBooleanMacro(ComputeScalars,int);

  // Description:
  // Enable the use of a scalar tree to accelerate contour extraction.
  vtkSetMacro(UseScalarTree,int);
  vtkGetMacro(UseScalarTree,int);
  vtkBooleanMacro(UseScalarTree,int);

  // Description:
  // Enable the use of a scalar tree to accelerate contour extraction.
  virtual void SetScalarTree(vtkScalarTree*);
  vtkGetObjectMacro(ScalarTree,vtkScalarTree);

  // Description:
  // Set / get a spatial locator for merging points. By default,
  // an instance of vtkMergePoints is used.
  void SetLocator(vtkIncrementalPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkIncrementalPointLocator);

  // Description:
  // Create default locator. Used to create one when none is
  // specified. The locator is used to merge coincident points.
  void CreateDefaultLocator();

  // Description:
  // Set/get which component of the scalar array to contour on; defaults to 0.
  // Currently this feature only works if the input is a vtkImageData.
  void SetArrayComponent( int );
  int  GetArrayComponent();

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

protected:
  vtkSMP2ContourFilter();
  ~vtkSMP2ContourFilter();

  virtual void ReportReferences(vtkGarbageCollector*);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector,
                          vtkThreadLocal<vtkDataObject>** output);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  vtkContourValues *ContourValues;
  int ComputeNormals;
  int ComputeGradients;
  int ComputeScalars;
  vtkIncrementalPointLocator *Locator;
  int UseScalarTree;
  vtkScalarTree *ScalarTree;

  vtkSynchronizedTemplates2D *SynchronizedTemplates2D;
  vtkSynchronizedTemplates3D *SynchronizedTemplates3D;
  vtkGridSynchronizedTemplates3D *GridSynchronizedTemplates;
  vtkRectilinearSynchronizedTemplates *RectilinearSynchronizedTemplates;

private:
  vtkSMP2ContourFilter(const vtkSMP2ContourFilter&);  // Not implemented.
  void operator=(const vtkSMP2ContourFilter&);  // Not implemented.
};

// Description:
// Set a particular contour value at contour number i. The index i ranges
// between 0<=i<NumberOfContours.
inline void vtkSMP2ContourFilter::SetValue(int i, double value)
{this->ContourValues->SetValue(i,value);}

// Description:
// Get the ith contour value.
inline double vtkSMP2ContourFilter::GetValue(int i)
{return this->ContourValues->GetValue(i);}

// Description:
// Get a pointer to an array of contour values. There will be
// GetNumberOfContours() values in the list.
inline double *vtkSMP2ContourFilter::GetValues()
{return this->ContourValues->GetValues();}

// Description:
// Fill a supplied list with contour values. There will be
// GetNumberOfContours() values in the list. Make sure you allocate
// enough memory to hold the list.
inline void vtkSMP2ContourFilter::GetValues(double *contourValues)
{this->ContourValues->GetValues(contourValues);}

// Description:
// Set the number of contours to place into the list. You only really
// need to use this method to reduce list size. The method SetValue()
// will automatically increase list size as needed.
inline void vtkSMP2ContourFilter::SetNumberOfContours(int number)
{this->ContourValues->SetNumberOfContours(number);}

// Description:
// Get the number of contours in the list of contour values.
inline int vtkSMP2ContourFilter::GetNumberOfContours()
{return this->ContourValues->GetNumberOfContours();}

// Description:
// Generate numContours equally spaced contour values between specified
// range. Contour values will include min/max range values.
inline void vtkSMP2ContourFilter::GenerateValues(int numContours, double range[2])
{this->ContourValues->GenerateValues(numContours, range);}

// Description:
// Generate numContours equally spaced contour values between specified
// range. Contour values will include min/max range values.
inline void vtkSMP2ContourFilter::GenerateValues(int numContours, double
                                             rangeStart, double rangeEnd)
{this->ContourValues->GenerateValues(numContours, rangeStart, rangeEnd);}


#endif


