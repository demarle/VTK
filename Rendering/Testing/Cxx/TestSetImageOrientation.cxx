/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestSetImageOrientation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This tests the Nearest, Linear, and Cubic interpolation.
//
// The command line arguments are:
// -I        => run in interactive mode

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkImageData.h"
#include "vtkImageResliceMapper.h"
#include "vtkImageProperty.h"
#include "vtkImageSlice.h"
#include "vtkImageReader2.h"
#include "vtkLookupTable.h"
#include "vtkPlane.h"

int TestSetImageOrientation(int argc, char* argv[])
{
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  vtkInteractorStyleImage *style = vtkInteractorStyleImage::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->SetSize(512,512);
  iren->SetRenderWindow(renWin);
  iren->SetInteractorStyle(style);
  renWin->Delete();
  style->Delete();

  // x is right, y is up
  style->SetZViewRightVector(1, 0, 0);
  style->SetZViewUpVector(0, 1, 0);

  // x is right, z is up
  style->SetYViewRightVector(1, 0, 0);
  style->SetYViewUpVector(0, 0, -1);

  // z is left, y is up
  style->SetXViewRightVector(0, -1, 0);
  style->SetXViewUpVector(0, 0, -1);

  // oblique view
  double obliqueViewRightVector[3] = { 1, 0, 0};
  double obliqueViewUpVector[3] = { 0, 0.8660254, 0.5 };

  vtkImageReader2 *reader = vtkImageReader2::New();
  reader->SetDataByteOrderToLittleEndian();
  reader->SetDataExtent(0, 63, 0, 63, 1, 93);
  reader->SetDataSpacing(3.2, 3.2, 1.5);
  reader->SetDataOrigin(-100.8, -100.8, -69.0);
  char* fname = vtkTestUtilities::ExpandDataFileName(
    argc, argv, "Data/headsq/quarter");
  reader->SetFilePrefix(fname);
  reader->Update();
  delete[] fname;

  for (int i = 0; i < 4; i++)
    {
    vtkRenderer *renderer = vtkRenderer::New();
    vtkCamera *camera = renderer->GetActiveCamera();
    renderer->SetBackground(0.0, 0.0, 0.0);
    renderer->SetViewport(0.5*(i&1), 0.25*(i&2),
                          0.5 + 0.5*(i&1), 0.5 + 0.25*(i&2));
    renWin->AddRenderer(renderer);
    renderer->Delete();

    vtkImageResliceMapper *imageMapper = vtkImageResliceMapper::New();
    imageMapper->SetInputConnection(reader->GetOutputPort());
    imageMapper->SliceFacesCameraOn();

    vtkImageSlice *image = vtkImageSlice::New();
    image->SetMapper(imageMapper);
    imageMapper->Delete();
    renderer->AddViewProp(image);

    image->GetProperty()->SetColorWindow(2000);
    image->GetProperty()->SetColorLevel(1000);

    image->Delete();
    camera->ParallelProjectionOn();
    renderer->ResetCamera();
    camera->SetParallelScale(80.0);

    style->SetCurrentRenderer(renderer);
    if (i == 0)
      {
      style->SetImageOrientation(
        style->GetXViewRightVector(),
        style->GetXViewUpVector());
      }
    else if (i == 1)
      {
      style->SetImageOrientation(
        obliqueViewRightVector,
        obliqueViewUpVector);
      }
    else if (i == 2)
      {
      style->SetImageOrientation(
        style->GetYViewRightVector(),
        style->GetYViewUpVector());
      }
    else // if (i == 3)
      {
      style->SetImageOrientation(
        style->GetZViewRightVector(),
        style->GetZViewUpVector());
      }
    }

  renWin->Render();
  int retVal = vtkRegressionTestImage( renWin );
  if ( retVal == vtkRegressionTester::DO_INTERACTOR )
    {
    iren->Start();
    }
  iren->Delete();

  reader->Delete();

  return !retVal;
}
