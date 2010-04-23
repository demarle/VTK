/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAnnotatedCubeActor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAnnotatedCubeActor - a 3D cube with face labels
// .SECTION Description
// vtkAnnotatedCubeActor is a hybrid 3D actor used to represent an anatomical
// orientation marker in a scene.  The class consists of a 3D unit cube centered
// on the origin with each face labelled in correspondance to a particular
// coordinate direction.  For example, with Cartesian directions, the user
// defined text labels could be: +X, -X, +Y, -Y, +Z, -Z, while for anatomical
// directions: A, P, L, R, S, I.  Text is automatically centered on each cube
// face and is not restriceted to single characters. In addition to or in
// replace of a solid text label representation, the outline edges of the labels
// can be displayed.  The individual properties of the cube, face labels
// and text outlines can be manipulated as can their visibility.

// .SECTION Caveats
// vtkAnnotatedCubeActor is primarily intended for use with
// vtkOrientationMarkerWidget. The cube face text is generated by vtkVectorText
// and therefore the font attributes are restricted.

// .SECTION See Also
// vtkAxesActor vtkOrientationMarkerWidget vtkVectorText

#ifndef __vtkAnnotatedCubeActor_h
#define __vtkAnnotatedCubeActor_h

#include "vtkProp3D.h"

class vtkActor;
class vtkAppendPolyData;
class vtkAssembly;
class vtkCubeSource;
class vtkFeatureEdges;
class vtkPropCollection;
class vtkProperty;
class vtkRenderer;
class vtkTransform;
class vtkTransformFilter;
class vtkVectorText;

class VTK_HYBRID_EXPORT vtkAnnotatedCubeActor : public vtkProp3D
{
public:
  static vtkAnnotatedCubeActor *New();
  vtkTypeMacro(vtkAnnotatedCubeActor,vtkProp3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // For some exporters and other other operations we must be
  // able to collect all the actors or volumes. These methods
  // are used in that process.
  virtual void GetActors(vtkPropCollection *);

  // Description:
  // Support the standard render methods.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport);

  // Description:
  // Does this prop have some translucent polygonal geometry?
  virtual int HasTranslucentPolygonalGeometry();
  
  // Description:
  // Shallow copy of an axes actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Get the bounds for this Actor as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax). (The
  // method GetBounds(double bounds[6]) is available from the superclass.)
  void GetBounds(double bounds[6]);
  double *GetBounds();

  // Description:
  // Get the actors mtime plus consider its properties and texture if set.
  unsigned long int GetMTime();

  // Description:
  // Set/Get the scale factor for the face text
  void SetFaceTextScale(double);
  vtkGetMacro(FaceTextScale, double);

  // Description:
  // Get the individual face text properties.
  vtkProperty *GetXPlusFaceProperty();
  vtkProperty *GetXMinusFaceProperty();
  vtkProperty *GetYPlusFaceProperty();
  vtkProperty *GetYMinusFaceProperty();
  vtkProperty *GetZPlusFaceProperty();
  vtkProperty *GetZMinusFaceProperty();

  // Description:
  // Get the cube properties.
  vtkProperty *GetCubeProperty();

  // Description:
  // Get the text edges properties.
  vtkProperty *GetTextEdgesProperty();

  // Description:
  // Set/get the face text.
  vtkSetStringMacro( XPlusFaceText );
  vtkGetStringMacro( XPlusFaceText );
  vtkSetStringMacro( XMinusFaceText );
  vtkGetStringMacro( XMinusFaceText );
  vtkSetStringMacro( YPlusFaceText );
  vtkGetStringMacro( YPlusFaceText );
  vtkSetStringMacro( YMinusFaceText );
  vtkGetStringMacro( YMinusFaceText );
  vtkSetStringMacro( ZPlusFaceText );
  vtkGetStringMacro( ZPlusFaceText );
  vtkSetStringMacro( ZMinusFaceText );
  vtkGetStringMacro( ZMinusFaceText );

  // Description:
  // Enable/disable drawing the vector text edges.
  void SetTextEdgesVisibility(int);
  int GetTextEdgesVisibility();

  // Description:
  // Enable/disable drawing the cube.
  void SetCubeVisibility(int);
  int GetCubeVisibility();

  // Description:
  // Enable/disable drawing the vector text.
  void SetFaceTextVisibility(int);
  int GetFaceTextVisibility();

  // Description:
  // Augment individual face text orientations.
  vtkSetMacro(XFaceTextRotation,double);
  vtkGetMacro(XFaceTextRotation,double);
  vtkSetMacro(YFaceTextRotation,double);
  vtkGetMacro(YFaceTextRotation,double);
  vtkSetMacro(ZFaceTextRotation,double);
  vtkGetMacro(ZFaceTextRotation,double);

  // Description:
  // Get the assembly so that user supplied transforms can be applied
  vtkAssembly *GetAssembly()
    { return this->Assembly; }

protected:
  vtkAnnotatedCubeActor();
  ~vtkAnnotatedCubeActor();

  vtkCubeSource      *CubeSource;
  vtkActor           *CubeActor;

  vtkAppendPolyData  *AppendTextEdges;
  vtkFeatureEdges    *ExtractTextEdges;
  vtkActor           *TextEdgesActor;

  void                UpdateProps();

  char               *XPlusFaceText;
  char               *XMinusFaceText;
  char               *YPlusFaceText;
  char               *YMinusFaceText;
  char               *ZPlusFaceText;
  char               *ZMinusFaceText;

  double              FaceTextScale;

  double              XFaceTextRotation;
  double              YFaceTextRotation;
  double              ZFaceTextRotation;

  vtkVectorText      *XPlusFaceVectorText;
  vtkVectorText      *XMinusFaceVectorText;
  vtkVectorText      *YPlusFaceVectorText;
  vtkVectorText      *YMinusFaceVectorText;
  vtkVectorText      *ZPlusFaceVectorText;
  vtkVectorText      *ZMinusFaceVectorText;

  vtkActor           *XPlusFaceActor;
  vtkActor           *XMinusFaceActor;
  vtkActor           *YPlusFaceActor;
  vtkActor           *YMinusFaceActor;
  vtkActor           *ZPlusFaceActor;
  vtkActor           *ZMinusFaceActor;

  vtkTransformFilter *TransformFilter;
  vtkTransform       *Transform;

  vtkAssembly        *Assembly;

private:
  vtkAnnotatedCubeActor(const vtkAnnotatedCubeActor&);  // Not implemented.
  void operator=(const vtkAnnotatedCubeActor&);  // Not implemented.
};

#endif

