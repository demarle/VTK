#include "vtkSMPTransform.h"
#include "vtkDataArray.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkPoints.h"
#include "vtkObjectFactory.h"

#include "vtkSMP.h"

#include <stdlib.h>

vtkStandardNewMacro(vtkSMPTransform);

//----------------------------------------------------------------------------
vtkSMPTransform::vtkSMPTransform()
{
  this->Input = NULL;

  // most of the functionality is provided by the concatenation
  this->Concatenation = vtkTransformConcatenation::New();

  // the stack will be allocated the first time Push is called
  this->Stack = NULL;

  // initialize the legacy 'Point' info
  this->Point[0] = this->Point[1] = this->Point[2] = this->Point[3] = 0.0;
  this->DoublePoint[0] =
    this->DoublePoint[1] = this->DoublePoint[2] = this->DoublePoint[3] = 0.0;

  // save the original matrix MTime as part of a hack to support legacy code
  this->MatrixUpdateMTime = this->Matrix->GetMTime();
}

//----------------------------------------------------------------------------
vtkSMPTransform::~vtkSMPTransform()
{
  this->SetInput(NULL);

  if (this->Concatenation)
    {
    this->Concatenation->Delete();
    }
  if (this->Stack)
    {
    this->Stack->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkSMPTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Update();

  this->Superclass::PrintSelf(os, indent);
  os << indent << "Input: (" << this->Input << ")\n";
  os << indent << "InverseFlag: " << this->GetInverseFlag() << "\n";
  os << indent << "NumberOfConcatenatedTransforms: " <<
    this->GetNumberOfConcatenatedTransforms() << "\n";
  if (this->GetNumberOfConcatenatedTransforms() != 0)
    {
    int n = this->GetNumberOfConcatenatedTransforms();
    for (int i = 0; i < n; i++)
      {
      vtkLinearTransform *t = this->GetConcatenatedTransform(i);
      os << indent << "    " << i << ": " << t->GetClassName() << " at " <<
         t << "\n";
      }
    }

  os << indent << "DoublePoint: " << "( " <<
     this->DoublePoint[0] << ", " << this->DoublePoint[1] << ", " <<
     this->DoublePoint[2] << ", " << this->DoublePoint[3] << ")\n";

  os << indent << "Point: " << "( " <<
     this->Point[0] << ", " << this->Point[1] << ", " <<
     this->Point[2] << ", " << this->Point[3] << ")\n";
}

//----------------------------------------------------------------------------
void vtkSMPTransform::Identity()
{
  this->Concatenation->Identity();
  this->Modified();

  // support for the legacy hack in InternalUpdate
  if (this->Matrix->GetMTime() > this->MatrixUpdateMTime)
    {
    this->Matrix->Identity();
    }
}

//----------------------------------------------------------------------------
void vtkSMPTransform::Inverse()
{
  this->Concatenation->Inverse();
  this->Modified();

  // for the legacy hack in InternalUpdate
  if (this->Matrix->GetMTime() > this->MatrixUpdateMTime)
    {
    this->Matrix->Invert();
    }
}

//----------------------------------------------------------------------------
void vtkSMPTransform::InternalDeepCopy(vtkAbstractTransform *gtrans)
{
  vtkSMPTransform *transform = static_cast<vtkSMPTransform *>(gtrans);

  // copy the input
  this->SetInput(transform->Input);

  // copy the concatenation
  this->Concatenation->DeepCopy(transform->Concatenation);

  // copy the stack
  if (transform->Stack)
    {
    if (this->Stack == NULL)
      {
      this->Stack = vtkTransformConcatenationStack::New();
      }
    this->Stack->DeepCopy(transform->Stack);
    }
  else
    {
    if (this->Stack)
      {
      this->Stack->Delete();
      this->Stack = NULL;
      }
    }

  // legacy stuff: copy Point and DoublePoint
  for (int j = 0; j < 3; j++)
    {
    this->Point[j] = transform->Point[j];
    this->DoublePoint[j] = transform->DoublePoint[j];
    }

  // to support the legacy hack in InternalUpdate
  this->Matrix->DeepCopy(transform->Matrix);
  this->MatrixUpdateMTime = this->Matrix->GetMTime();
#ifndef VTK_LEGACY_REMOVE
  if (transform->Matrix->GetMTime() > transform->MatrixUpdateMTime)
    { // this copies the legacy hack flag to the transform
    vtkWarningMacro(<<"InternalDeepCopy: Legacy Hack deprecated in VTK 4.2.  May be removed in a future version.");
    this->MatrixUpdateMTime--;
    }
#endif
}

//----------------------------------------------------------------------------
void vtkSMPTransform::InternalUpdate()
{
  int i;
  int nTransforms = this->Concatenation->GetNumberOfTransforms();
  int nPreTransforms = this->Concatenation->GetNumberOfPreTransforms();

  // check to see whether someone has been fooling around with our matrix
  int doTheLegacyHack = 0;
  if (this->Matrix->GetMTime() > this->MatrixUpdateMTime)
    {
    vtkDebugMacro(<<"InternalUpdate: this->Matrix was modified by something other than 'this'");

    // check to see if we have any inputs or concatenated transforms
    int isPipelined = (this->Input != 0);
    for (i = 0; i < nTransforms && !isPipelined; i++)
      { // the vtkSimpleTransform is just a matrix placeholder,
        // it is not a real transform
      isPipelined =
        !this->Concatenation->GetTransform(i)->IsA("vtkSimpleTransform");
      }
    // do the legacy hack only if we have no input transforms
    doTheLegacyHack = !isPipelined;
    }

  // copy matrix from input
  if (this->Input)
    {
    this->Matrix->DeepCopy(this->Input->GetMatrix());
    // if inverse flag is set, invert the matrix
    if (this->Concatenation->GetInverseFlag())
      {
      this->Matrix->Invert();
      }
    }
  else if (doTheLegacyHack)
    {
    vtkWarningMacro("InternalUpdate: doing hack to support legacy code.  "
                    "This is deprecated in VTK 4.2.  May be removed in a "
                    "future version.");
    // this heuristic works perfectly if GetMatrix() or GetMatrixPointer()
    // was called immediately prior to the matrix modifications
    // (fortunately, this is almost always the case)
    if (this->Matrix->GetMTime() > this->Concatenation->GetMaxMTime())
      { // don't apply operations that occurred after matrix modification
      nPreTransforms = nTransforms = 0;
      }
    }
  else
    {  // otherwise, we start with the identity transform as our base
    this->Matrix->Identity();
    }

  // concatenate PreTransforms
  for (i = nPreTransforms-1; i >= 0; i--)
    {
    vtkHomogeneousTransform *transform =
      static_cast<vtkHomogeneousTransform *>(this->Concatenation->GetTransform(i));
    vtkMatrix4x4::Multiply4x4(this->Matrix,transform->GetMatrix(),
                              this->Matrix);
    }

  // concatenate PostTransforms
  for (i = nPreTransforms; i < nTransforms; i++)
    {
    vtkHomogeneousTransform *transform =
      static_cast<vtkHomogeneousTransform *>(this->Concatenation->GetTransform(i));
    vtkMatrix4x4::Multiply4x4(transform->GetMatrix(),this->Matrix,
                              this->Matrix);
    }

  if (doTheLegacyHack)
    { // the transform operations have been incorporated into the matrix,
      // so delete them
    this->Concatenation->Identity();
    }
  else
    { // having this in the 'else' forces the legacy flag to be sticky
    this->MatrixUpdateMTime = this->Matrix->GetMTime();
    }
}

//----------------------------------------------------------------------------
void vtkSMPTransform::Concatenate(vtkLinearTransform *transform)
{
  if (transform->CircuitCheck(this))
    {
    vtkErrorMacro("Concatenate: this would create a circular reference.");
    return;
    }
  this->Concatenation->Concatenate(transform);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSMPTransform::SetInput(vtkLinearTransform *input)
{
  if (this->Input == input)
    {
    return;
    }
  if (input && input->CircuitCheck(this))
    {
    vtkErrorMacro("SetInput: this would create a circular reference.");
    return;
    }
  if (this->Input)
    {
    this->Input->Delete();
    }
  this->Input = input;
  if (this->Input)
    {
    this->Input->Register(this);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkSMPTransform::CircuitCheck(vtkAbstractTransform *transform)
{
  if (this->vtkLinearTransform::CircuitCheck(transform) ||
      (this->Input && this->Input->CircuitCheck(transform)))
    {
    return 1;
    }

  int n = this->Concatenation->GetNumberOfTransforms();
  for (int i = 0; i < n; i++)
    {
    if (this->Concatenation->GetTransform(i)->CircuitCheck(transform))
      {
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkAbstractTransform *vtkSMPTransform::MakeTransform()
{
  return vtkSMPTransform::New();
}

//----------------------------------------------------------------------------
unsigned long vtkSMPTransform::GetMTime()
{
  unsigned long mtime = this->vtkLinearTransform::GetMTime();
  unsigned long mtime2;

  // checking the matrix MTime is part of the legacy hack in InternalUpdate
  if ((mtime2 = this->Matrix->GetMTime()) > this->MatrixUpdateMTime)
    {
    if (mtime2 > mtime)
      {
      mtime = mtime2;
      }
    }

  if (this->Input)
    {
    mtime2 = this->Input->GetMTime();
    if (mtime2 > mtime)
      {
      mtime = mtime2;
      }
    }
  mtime2 = this->Concatenation->GetMaxMTime();
  if (mtime2 > mtime)
    {
    return mtime2;
    }
  return mtime;
}

//----------------------------------------------------------------------------
// Get the x, y, z orientation angles from the transformation matrix as an
// array of three floating point values.
void vtkSMPTransform::GetOrientation(double orientation[3],
                                  vtkMatrix4x4 *amatrix)
{
#define VTK_AXIS_EPSILON 0.001
#define VTK_ORTHO_EPSILON 4e-16
  int i;

  // convenient access to matrix
  double (*matrix)[4] = amatrix->Element;
  double ortho[3][3];

  for (i = 0; i < 3; i++)
    {
    ortho[0][i] = matrix[0][i];
    ortho[1][i] = matrix[1][i];
    ortho[2][i] = matrix[2][i];
    }
  if (vtkMath::Determinant3x3(ortho) < 0)
    {
    ortho[0][2] = -ortho[0][2];
    ortho[1][2] = -ortho[1][2];
    ortho[2][2] = -ortho[2][2];
    }

  // Check whether matrix is orthogonal
  double r1 = vtkMath::Dot(ortho[0],ortho[1]);
  double r2 = vtkMath::Dot(ortho[0],ortho[2]);
  double r3 = vtkMath::Dot(ortho[1],ortho[2]);

  // Orthogonalize the matrix if it isn't already orthogonal
  if ((r1*r1) + (r2*r2) + (r3*r3) > (VTK_ORTHO_EPSILON*VTK_ORTHO_EPSILON))
    {
    vtkMath::Orthogonalize3x3(ortho, ortho);
    }

  // first rotate about y axis
  double x2 = ortho[2][0];
  double y2 = ortho[2][1];
  double z2 = ortho[2][2];

  double x3 = ortho[1][0];
  double y3 = ortho[1][1];
  double z3 = ortho[1][2];

  double d1 = sqrt(x2*x2 + z2*z2);

  double cosTheta, sinTheta;
  if (d1 < VTK_AXIS_EPSILON)
    {
    cosTheta = 1.0;
    sinTheta = 0.0;
    }
  else
    {
    cosTheta = z2/d1;
    sinTheta = x2/d1;
    }

  double theta = atan2(sinTheta, cosTheta);
  orientation[1] = - vtkMath::DegreesFromRadians( theta );

  // now rotate about x axis
  double d = sqrt(x2*x2 + y2*y2 + z2*z2);

  double sinPhi, cosPhi;
  if (d < VTK_AXIS_EPSILON)
    {
    sinPhi = 0.0;
    cosPhi = 1.0;
    }
  else if (d1 < VTK_AXIS_EPSILON)
    {
    sinPhi = y2/d;
    cosPhi = z2/d;
    }
  else
    {
    sinPhi = y2/d;
    cosPhi = (x2*x2 + z2*z2)/(d1*d);
    }

  double phi = atan2(sinPhi, cosPhi);
  orientation[0] = vtkMath::DegreesFromRadians( phi );

  // finally, rotate about z
  double x3p = x3*cosTheta - z3*sinTheta;
  double y3p = - sinPhi*sinTheta*x3 + cosPhi*y3 - sinPhi*cosTheta*z3;
  double d2 = sqrt(x3p*x3p + y3p*y3p);

  double cosAlpha, sinAlpha;
  if (d2 < VTK_AXIS_EPSILON)
    {
    cosAlpha = 1.0;
    sinAlpha = 0.0;
    }
  else
    {
    cosAlpha = y3p/d2;
    sinAlpha = x3p/d2;
    }

  double alpha = atan2(sinAlpha, cosAlpha);
  orientation[2] = vtkMath::DegreesFromRadians( alpha );
}

//----------------------------------------------------------------------------
// Get the x, y, z orientation angles from the transformation matrix as an
// array of three floating point values.
void vtkSMPTransform::GetOrientation(double orientation[3])
{
  this->Update();
  this->GetOrientation(orientation, this->Matrix);
}

//----------------------------------------------------------------------------
// vtkSMPTransform::GetOrientationWXYZ
void vtkSMPTransform::GetOrientationWXYZ(double wxyz[4])
{
  int i;

  this->Update();
  // convenient access to matrix
  double (*matrix)[4] = this->Matrix->Element;
  double ortho[3][3];

  for (i = 0; i < 3; i++)
    {
    ortho[0][i] = matrix[0][i];
    ortho[1][i] = matrix[1][i];
    ortho[2][i] = matrix[2][i];
    }
  if (vtkMath::Determinant3x3(ortho) < 0)
    {
    ortho[0][2] = -ortho[0][2];
    ortho[1][2] = -ortho[1][2];
    ortho[2][2] = -ortho[2][2];
    }

  vtkMath::Matrix3x3ToQuaternion(ortho, wxyz);

  // calc the return value wxyz
 double mag = sqrt( wxyz[1] * wxyz[1] + wxyz[2] * wxyz[2] + wxyz[3] * wxyz[3] );

  if ( mag )
    {
    wxyz[0] = 2. * vtkMath::DegreesFromRadians( acos( wxyz[0] ) );
    wxyz[1] /= mag;
    wxyz[2] /= mag;
    wxyz[3] /= mag;
    }
  else
    {
    wxyz[0] = 0.0;
    wxyz[1] = 0.0;
    wxyz[2] = 0.0;
    wxyz[3] = 1.0;
    }
}


//----------------------------------------------------------------------------
// Return the position from the current transformation matrix as an array
// of three floating point numbers. This is simply returning the translation
// component of the 4x4 matrix.
void vtkSMPTransform::GetPosition(double position[3])
{
  this->Update();

  position[0] = this->Matrix->Element[0][3];
  position[1] = this->Matrix->Element[1][3];
  position[2] = this->Matrix->Element[2][3];
}

//----------------------------------------------------------------------------
// Return the x, y, z scale factors of the current transformation matrix as
// an array of three float numbers.
void vtkSMPTransform::GetScale(double scale[3])
{
  this->Update();

  // convenient access to matrix
  double (*matrix)[4] = this->Matrix->Element;
  double U[3][3], VT[3][3];

  for (int i = 0; i < 3; i++)
    {
    U[0][i] = matrix[0][i];
    U[1][i] = matrix[1][i];
    U[2][i] = matrix[2][i];
    }

  vtkMath::SingularValueDecomposition3x3(U, U, scale, VT);
}

//----------------------------------------------------------------------------
// Return the inverse of the current transformation matrix.
void vtkSMPTransform::GetInverse(vtkMatrix4x4 *inverse)
{
  vtkMatrix4x4::Invert(this->GetMatrix(),inverse);
}

//----------------------------------------------------------------------------
// Obtain the transpose of the current transformation matrix.
void vtkSMPTransform::GetTranspose(vtkMatrix4x4 *transpose)
{
  vtkMatrix4x4::Transpose(this->GetMatrix(),transpose);
}

/* ************************************************************************* *
 *                                                                           *
 *                Kaapi transformations of vtkLinearTransform                *
 *                                                                           *
 * ************************************************************************* */
//------------------------------------------------------------------------
template <class T1, class T2, class T3>
inline void vtkSMPTransformPoint(T1 matrix[4][4],
                                    T2 in[3], T3 out[3])
{
  T3 x = static_cast<T3>(
    matrix[0][0]*in[0]+matrix[0][1]*in[1]+matrix[0][2]*in[2]+matrix[0][3]);
  T3 y = static_cast<T3>(
    matrix[1][0]*in[0]+matrix[1][1]*in[1]+matrix[1][2]*in[2]+matrix[1][3]);
  T3 z = static_cast<T3>(
    matrix[2][0]*in[0]+matrix[2][1]*in[1]+matrix[2][2]*in[2]+matrix[2][3]);

  out[0] = x;
  out[1] = y;
  out[2] = z;
}

//------------------------------------------------------------------------
template <class T1, class T2, class T3, class T4>
inline void vtkSMPTransformDerivative(T1 matrix[4][4],
                                         T2 in[3], T3 out[3],
                                         T4 derivative[3][3])
{
  vtkSMPTransformPoint(matrix,in,out);

  for (int i = 0; i < 3; i++)
    {
    derivative[0][i] = static_cast<T4>(matrix[0][i]);
    derivative[1][i] = static_cast<T4>(matrix[1][i]);
    derivative[2][i] = static_cast<T4>(matrix[2][i]);
    }
}

//------------------------------------------------------------------------
template <class T1, class T2, class T3>
inline void vtkSMPTransformVector(T1 matrix[4][4],
                                     T2 in[3], T3 out[3])
{
  T3 x = static_cast<T3>(
    matrix[0][0]*in[0] + matrix[0][1]*in[1] + matrix[0][2]*in[2]);
  T3 y = static_cast<T3>(
    matrix[1][0]*in[0] + matrix[1][1]*in[1] + matrix[1][2]*in[2]);
  T3 z = static_cast<T3>(
    matrix[2][0]*in[0] + matrix[2][1]*in[1] + matrix[2][2]*in[2]);

  out[0] = x;
  out[1] = y;
  out[2] = z;
}

//------------------------------------------------------------------------
template <class T1, class T2, class T3>
inline void vtkSMPTransformNormal(T1 mat[4][4],
                                            T2 in[3], T3 out[3])
{
  // to transform the normal, multiply by the transposed inverse matrix
  T1 matrix[4][4];
  memcpy(*matrix,*mat,16*sizeof(T1));
  vtkMatrix4x4::Invert(*matrix,*matrix);
  vtkMatrix4x4::Transpose(*matrix,*matrix);

  vtkSMPTransformVector(matrix,in,out);

  vtkMath::Normalize(out);
}

//------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformPoint(const float in[3],
                                                float out[3])
{
  vtkSMPTransformPoint(this->Matrix->Element,in,out);
}

//------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformPoint(const double in[3],
                                                double out[3])
{
  vtkSMPTransformPoint(this->Matrix->Element,in,out);
}

//------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformNormal(const float in[3],
                                                 float out[3])
{
  vtkSMPTransformNormal(this->Matrix->Element,in,out);
}

//------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformNormal(const double in[3],
                                                 double out[3])
{
  vtkSMPTransformNormal(this->Matrix->Element,in,out);
}

//------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformVector(const float in[3],
                                                 float out[3])
{
  vtkSMPTransformVector(this->Matrix->Element,in,out);
}

//------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformVector(const double in[3],
                                                 double out[3])
{
  vtkSMPTransformVector(this->Matrix->Element,in,out);
}

//----------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformDerivative(const float in[3],
                                                     float out[3],
                                                     float derivative[3][3])
{
  vtkSMPTransformDerivative(this->Matrix->Element,in,out,derivative);
}

//----------------------------------------------------------------------------
void vtkSMPTransform::InternalTransformDerivative(const double in[3],
                                                     double out[3],
                                                     double derivative[3][3])
{
  vtkSMPTransformDerivative(this->Matrix->Element,in,out,derivative);
}

//----------------------------------------------------------------------------
// Transform the normals and vectors using the derivative of the
// transformation.  Either inNms or inVrs can be set to NULL.
// Normals are multiplied by the inverse transpose of the transform
// derivative, while vectors are simply multiplied by the derivative.
// Note that the derivative of the inverse transform is simply the
// inverse of the derivative of the forward transform.
struct AllModificator : public vtkFunctor {
  vtkPoints* inPts;
  vtkPoints* outPts;
  vtkDataArray* inNms;
  vtkDataArray* outNms;
  vtkDataArray* inVcs;
  vtkDataArray* outVcs;
  double (*matrix)[4];
  double (*matrixInvTr)[4];
  void operator()(vtkIdType id) const
  {
    double point[3];
    inPts->GetPoint(id, point);
    vtkSMPTransformPoint(matrix, point, point);
    outPts->SetPoint(id, point);
    if (inVcs)
    {
      inVcs->GetTuple(id, point);
      vtkSMPTransformVector(matrix, point, point);
      outVcs->SetTuple(id, point);
    }
    if (inNms)
    {
      inNms->GetTuple(id, point);
      vtkSMPTransformVector(matrixInvTr, point, point);
      vtkMath::Normalize( point );
      outNms->SetTuple(id, point);
    }
  }
};

void vtkSMPTransform::TransformPointsNormalsVectors(vtkPoints *inPts,
                                                       vtkPoints *outPts,
                                                       vtkDataArray *inNms,
                                                       vtkDataArray *outNms,
                                                       vtkDataArray *inVrs,
                                                       vtkDataArray *outVrs)
{
  vtkIdType n = inPts->GetNumberOfPoints();
  double matrix[4][4];
  this->Update();

  AllModificator mymodificator;
  mymodificator.inPts = inPts;
  mymodificator.outPts = outPts;
  mymodificator.inNms = inNms;
  mymodificator.outNms = outNms;
  mymodificator.inVcs = inVrs;
  mymodificator.outVcs = outVrs;
  mymodificator.matrix = this->Matrix->Element;
  if (inNms)
    {
    vtkMatrix4x4::DeepCopy(*matrix,this->Matrix);
    vtkMatrix4x4::Invert(*matrix,*matrix);
    vtkMatrix4x4::Transpose(*matrix,*matrix);
    mymodificator.matrixInvTr = matrix;
    }
  
  vtkSMP::ForEach( 0, n, mymodificator );
}

//----------------------------------------------------------------------------
struct PtsModificator : public vtkFunctor {
  vtkPoints* inPts;
  vtkPoints* outPts;
  double (*matrix)[4];
  void operator () ( vtkIdType id ) const
  {
    double point[3];
    inPts->GetPoint( id, point );
    vtkSMPTransformPoint( matrix, point, point );
    outPts->SetPoint( id, point );
  }
};

void vtkSMPTransform::TransformPoints(vtkPoints *inPts,
                                         vtkPoints *outPts)
{
  vtkIdType n = inPts->GetNumberOfPoints();
  this->Update();

  PtsModificator mypointsmodificator;
  mypointsmodificator.inPts = inPts;
  mypointsmodificator.outPts = outPts;
  mypointsmodificator.matrix = this->Matrix->Element;

  vtkSMP::ForEach( 0, n, mypointsmodificator );
}

//----------------------------------------------------------------------------
struct NmsModificator : public vtkFunctor {
  vtkDataArray* inNms;
  vtkDataArray* outNms;
  double (*matrix)[4];
  void operator () ( vtkIdType id ) const
  {
    double norm[3];
    inNms->GetTuple( id, norm );
    vtkSMPTransformVector( matrix, norm, norm );
    vtkMath::Normalize( norm );
    outNms->SetTuple( id, norm );
  }
};

void vtkSMPTransform::TransformNormals(vtkDataArray *inNms,
                                          vtkDataArray *outNms)
{
  vtkIdType n = inNms->GetNumberOfTuples();
  double matrix[4][4];

  this->Update();

  // to transform the normal, multiply by the transposed inverse matrix
  vtkMatrix4x4::DeepCopy(*matrix,this->Matrix);
  vtkMatrix4x4::Invert(*matrix,*matrix);
  vtkMatrix4x4::Transpose(*matrix,*matrix);

  NmsModificator mynormalsmodificator;
  mynormalsmodificator.inNms = inNms;
  mynormalsmodificator.outNms = outNms;
  mynormalsmodificator.matrix = matrix;

  vtkSMP::ForEach( 0, n, mynormalsmodificator );
}

//----------------------------------------------------------------------------
struct VcsModificator : public vtkFunctor {
  vtkDataArray* inVcs;
  vtkDataArray* outVcs;
  double (*matrix)[4];
  void operator () ( vtkIdType id ) const
  {
    double vec[3];
    inVcs->GetTuple( id, vec );
    vtkSMPTransformVector( matrix, vec, vec );
    outVcs->SetTuple( id, vec );
  }
};

void vtkSMPTransform::TransformVectors(vtkDataArray *inNms,
                                          vtkDataArray *outNms)
{
  vtkIdType n = inNms->GetNumberOfTuples();
  this->Update();

  VcsModificator myvectorsmodificator;
  myvectorsmodificator.inVcs = inNms;
  myvectorsmodificator.outVcs = outNms;
  myvectorsmodificator.matrix = this->Matrix->Element;

  vtkSMP::ForEach( 0, n, myvectorsmodificator );
}
