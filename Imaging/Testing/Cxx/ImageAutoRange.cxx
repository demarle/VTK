/*=========================================================================

  Program:   Visualization Toolkit
  Module:    ImageAutoRange.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// Use vtkImageHistogramStatistics to auto compute the window/level
//
// The command line arguments are:
// -I        => run in interactive mode

#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkImageData.h"
#include "vtkImageSliceMapper.h"
#include "vtkImageProperty.h"
#include "vtkImageSlice.h"
#include "vtkPNGReader.h"
#include "vtkImageHistogramStatistics.h"

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

int ImageAutoRange(int argc, char *argv[])
{
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  vtkInteractorStyle *style = vtkInteractorStyleImage::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  iren->SetRenderWindow(renWin);
  iren->SetInteractorStyle(style);
  renWin->Delete();
  style->Delete();

  vtkPNGReader *reader = vtkPNGReader::New();

  char* fname = vtkTestUtilities::ExpandDataFileName(
    argc, argv, "Data/fullhead15.png");

  reader->SetFileName(fname);
  delete[] fname;

  vtkImageHistogramStatistics *statistics = vtkImageHistogramStatistics::New();
  statistics->SetInputConnection(reader->GetOutputPort());
  statistics->GenerateHistogramImageOff();
  statistics->Update();

  // Get a viewing range based on the full data range
  double range[2];
  range[0] = statistics->GetMinimum();
  range[1] = statistics->GetMaximum();

  // Use the autorange feature to get a better image range
  double autorange[2];
  statistics->GetAutoRange(autorange);

  for (int i = 0; i < 2; i++)
    {
    vtkRenderer *renderer = vtkRenderer::New();
    vtkCamera *camera = renderer->GetActiveCamera();
    renderer->SetBackground(0.0,0.0,0.0);
    renderer->SetViewport(0.5*(i&1), 0.0,
                          0.5 + 0.5*(i&1), 1.0);
    renWin->AddRenderer(renderer);
    renderer->Delete();

    vtkImageSliceMapper *imageMapper = vtkImageSliceMapper::New();
    imageMapper->SetInputConnection(reader->GetOutputPort());

    double *bounds = imageMapper->GetBounds();
    double point[3];
    point[0] = 0.5*(bounds[0] + bounds[1]);
    point[1] = 0.5*(bounds[2] + bounds[3]);
    point[2] = 0.5*(bounds[4] + bounds[5]);

    camera->SetFocalPoint(point);
    point[imageMapper->GetOrientation()] += 500.0;
    camera->SetPosition(point);
    camera->SetViewUp(0.0, 1.0, 0.0);
    camera->ParallelProjectionOn();
    camera->SetParallelScale(128);

    vtkImageSlice *image = vtkImageSlice::New();
    image->SetMapper(imageMapper);
    imageMapper->Delete();
    renderer->AddViewProp(image);

    if ((i & 1) == 0)
      {
      image->GetProperty()->SetColorWindow(range[1] - range[0]);
      image->GetProperty()->SetColorLevel(0.5*(range[0] + range[1]));
      }
    else
      {
      image->GetProperty()->SetColorWindow(autorange[1] - autorange[0]);
      image->GetProperty()->SetColorLevel(0.5*(autorange[0] + autorange[1]));
      }

    image->Delete();
    }

  renWin->SetSize(512,256);

  renWin->Render();
  int retVal = vtkRegressionTestImage( renWin );
  if ( retVal == vtkRegressionTester::DO_INTERACTOR )
    {
    iren->Start();
    }
  iren->Delete();

  statistics->Delete();
  reader->Delete();

  return !retVal;
}
