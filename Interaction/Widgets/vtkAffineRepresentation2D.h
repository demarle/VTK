/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAffineRepresentation2D.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAffineRepresentation2D - represent 2D affine transformations
// .SECTION Description
// This class is used to represent a vtkAffineWidget. This representation
// consists of three parts: a box, a circle, and a cross. The box is used for
// scaling and shearing, the circle for rotation, and the cross for
// translation. These parts are drawn in the overlay plane and maintain a
// constant size (width and height) specified in terms of normalized viewport
// coordinates. 
//
// The representation maintains an internal transformation matrix (see
// superclass' GetTransform() method). The transformations generated by this
// widget assume that the representation lies in the x-y plane. If this is
// not the case, the user is responsible for transforming this
// representation's matrix into the correct coordinate space (by judicious
// matrix multiplication). Note that the transformation matrix returned by
// GetTransform() is relative to the last PlaceWidget() invocation. (The
// PlaceWidget() sets the origin around which rotation and scaling occurs;
// the origin is the center point of the bounding box provided.)
//

// .SECTION See Also
// vtkAffineRepresentation vtkAffineWidget


#ifndef __vtkAffineRepresentation2D_h
#define __vtkAffineRepresentation2D_h

#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkAffineRepresentation.h"

class vtkProperty2D;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkTextProperty;
class vtkLeaderActor2D;
class vtkTextMapper;
class vtkActor2D;


class VTKINTERACTIONWIDGETS_EXPORT vtkAffineRepresentation2D : public vtkAffineRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static vtkAffineRepresentation2D *New();

  // Description:
  // Standard methods for instances of this class.
  vtkTypeMacro(vtkAffineRepresentation2D,vtkAffineRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the width of the various parts of the representation (in
  // pixels).  The three parts are of the representation are the translation
  // axes, the rotation circle, and the scale/shear box. Note that since the
  // widget resizes itself so that the width and height are always the
  // same, only the width needs to be specified.
  vtkSetClampMacro(BoxWidth,int,10,VTK_LARGE_INTEGER);
  vtkGetMacro(BoxWidth,int);
  vtkSetClampMacro(CircleWidth,int,10,VTK_LARGE_INTEGER);
  vtkGetMacro(CircleWidth,int);
  vtkSetClampMacro(AxesWidth,int,10,VTK_LARGE_INTEGER);
  vtkGetMacro(AxesWidth,int);

  // Description:
  // Specify the origin of the widget (in world coordinates). The origin
  // is the point where the widget places itself. Note that rotations and
  // scaling occurs around the origin.
  void SetOrigin(double o[3]) {this->SetOrigin(o[0],o[1],o[2]);}
  void SetOrigin(double ox, double oy, double oz);
  vtkGetVector3Macro(Origin,double);

  // Description:
  // Retrieve a linear transform characterizing the affine transformation
  // generated by this widget. This method copies its internal transform into
  // the transform provided. Note that the PlaceWidget() method initializes
  // the internal matrix to identity. All subsequent widget operations (i.e.,
  // scale, translate, rotate, shear) are concatenated with the internal
  // transform.
  virtual void GetTransform(vtkTransform *t);

  // Description:
  // Set/Get the properties when unselected and selected.
  void SetProperty(vtkProperty2D*);
  void SetSelectedProperty(vtkProperty2D*);
  void SetTextProperty(vtkTextProperty*);
  vtkGetObjectMacro(Property,vtkProperty2D);
  vtkGetObjectMacro(SelectedProperty,vtkProperty2D);
  vtkGetObjectMacro(TextProperty,vtkTextProperty);
  
  // Description:
  // Enable the display of text with numeric values characterizing the 
  // transformation. Rotation and shear are expressed in degrees; translation
  // the distance in world coordinates; and scale normalized (sx,sy) values.
  vtkSetMacro(DisplayText,int);
  vtkGetMacro(DisplayText,int);
  vtkBooleanMacro(DisplayText,int);

  // Description:
  // Subclasses of vtkAffineRepresentation2D must implement these methods. These
  // are the methods that the widget and its representation use to
  // communicate with each other. Note: PlaceWidget() reinitializes the 
  // transformation matrix (i.e., sets it to identity). It also sets the
  // origin for scaling and rotation.
  virtual void PlaceWidget(double bounds[6]);
  virtual void StartWidgetInteraction(double eventPos[2]);
  virtual void WidgetInteraction(double eventPos[2]);
  virtual void EndWidgetInteraction(double eventPos[2]);
  virtual int ComputeInteractionState(int X, int Y, int modify=0);
  virtual void BuildRepresentation();

  // Description:
  // Methods to make this class behave as a vtkProp.
  virtual void ShallowCopy(vtkProp *prop);
  virtual void GetActors2D(vtkPropCollection *);
  virtual void ReleaseGraphicsResources(vtkWindow *);
  virtual int RenderOverlay(vtkViewport *viewport);
  
protected:
  vtkAffineRepresentation2D();
  ~vtkAffineRepresentation2D();

  // Methods to manipulate the cursor
  void Translate(double eventPos[2]);
  void Scale(double eventPos[2]);
  void Rotate(double eventPos[2]);
  void Shear(double eventPos[2]);
  void Highlight(int highlight);
  void UpdateText(const char *text, double eventPos[2]);

  // The width of the widget in normalized viewport coordinates.
  int BoxWidth;
  int CircleWidth;
  int AxesWidth;

  // Display text
  int DisplayText;
  
  // Internal variables for bookkeeping (in display coordinates unless noted)
  double CurrentWidth;
  double CurrentRadius;
  double CurrentAxesWidth;

  // The internal transformation matrix
  vtkTransform *CurrentTransform;
  vtkTransform *TotalTransform;
  vtkTransform *TempTransform;
  double Origin[4]; //the current origin in world coordinates
  double DisplayOrigin[3]; //the current origin in display coordinates
  double CurrentTranslation[3]; //translation this movement
  double StartWorldPosition[4]; //Start event position converted to world
  double StartAngle; //The starting angle (always positive)
  double CurrentAngle;
  double CurrentScale[2];
  double CurrentShear[2];
  void   ApplyShear(); //helper method to apply shear to matrix
  
  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty2D   *Property;
  vtkProperty2D   *SelectedProperty;
  vtkTextProperty *TextProperty;
  void             CreateDefaultProperties();
  double           Opacity;
  double           SelectedOpacity;
  
  // Support picking
  double LastEventPosition[2];
  
  // These are the classes that form the geometric representation -----------
  // The label 
  vtkTextMapper *TextMapper;
  vtkActor2D    *TextActor;
  
  // The outer box
  vtkPoints           *BoxPoints;
  vtkCellArray        *BoxCellArray;
  vtkPolyData         *Box;
  vtkPolyDataMapper2D *BoxMapper;
  vtkActor2D          *BoxActor;

  vtkPoints           *HBoxPoints;
  vtkCellArray        *HBoxCellArray;
  vtkPolyData         *HBox;
  vtkPolyDataMapper2D *HBoxMapper;
  vtkActor2D          *HBoxActor;

  // The circle
  vtkPoints           *CirclePoints;
  vtkCellArray        *CircleCellArray;
  vtkPolyData         *Circle;
  vtkPolyDataMapper2D *CircleMapper;
  vtkActor2D          *CircleActor;

  vtkPoints           *HCirclePoints;
  vtkCellArray        *HCircleCellArray;
  vtkPolyData         *HCircle;
  vtkPolyDataMapper2D *HCircleMapper;
  vtkActor2D          *HCircleActor;
  
  // The translation axes
  vtkLeaderActor2D    *XAxis;
  vtkLeaderActor2D    *YAxis;
  vtkLeaderActor2D    *HXAxis;
  vtkLeaderActor2D    *HYAxis;

private:
  vtkAffineRepresentation2D(const vtkAffineRepresentation2D&);  //Not implemented
  void operator=(const vtkAffineRepresentation2D&);  //Not implemented
};

#endif
