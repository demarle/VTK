#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAxesActor.h"

#include "vtkImageCanvasSource2D.h"
#include "vtkImageConvolve.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPImageConvolve.h"
#endif

int main( int argc, char** argv )
  {
  vtkImageCanvasSource2D* image = vtkImageCanvasSource2D::New();
  image->SetScalarTypeToFloat();
  image->SetExtent(0,1023,0,1023,0,0);
  image->SetDrawColor(0.0);
  image->FillBox(0,1023,0,1023);
  image->SetDrawColor(1.0);
  image->FillBox(300,700,300,700);

  double kernel[25] = {1,1,1,1,1,5,4,3,2,1,5,4,3,2,1,5,4,3,2,1,1,1,1,1,1};

#ifdef VTK_CAN_USE_SMP
  vtkSMPImageConvolve* convolve = vtkSMPImageConvolve::New();
#else
  vtkImageConvolve* convolve = vtkImageConvolve::New();
#endif
  convolve->SetInputConnection( image->GetOutputPort() );
  convolve->SetKernel5x5(kernel);
  image->Delete();

  /* === Pipeline pull === */
#ifndef HIDE_VTK_WINDOW
  vtkImageViewer* viewer = vtkImageViewer::New();
  viewer->SetInputConnection( convolve->GetOutputPort() );
  viewer->SetColorWindow(18);
  viewer->SetColorLevel(9);
  viewer->SetSize(1024,1024);
  convolve->Delete();

  vtkRenderWindowInteractor* it = vtkRenderWindowInteractor::New();
  viewer->SetupInteractor(it);

  it->Initialize();
  it->Start();
  it->Delete();
  viewer->Delete();
#else
  convolve->Update();
  convolve->Delete();
#endif

  return 0;
  }
