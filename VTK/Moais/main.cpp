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

#include "vtkPNGReader.h"
#include "vtkJPEGReader.h"
#include "vtkImageDotProduct.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPImageDotProduct.h"
#endif

int main( int argc, char** argv )
  {
  vtkJPEGReader* image1 = vtkJPEGReader::New();
  image1->SetFileName("/home/mathias/dataset.jpeg");
  vtkJPEGReader* image2 = vtkJPEGReader::New();
  image2->SetFileName("/home/mathias/dataset.jpeg");

#ifdef VTK_CAN_USE_SMP
  vtkSMPImageDotProduct* filter = vtkSMPImageDotProduct::New();
#else
  vtkImageDotProduct* filter = vtkImageDotProduct::New();
#endif
  filter->SetInputConnection( image1->GetOutputPort() );
  filter->SetInputConnection( 1, image2->GetOutputPort() );
  image1->Delete();
  image2->Delete();

  /* === Pipeline pull === */
#ifndef HIDE_VTK_WINDOW
  vtkImageViewer* viewer = vtkImageViewer::New();
  viewer->SetInputConnection( filter->GetOutputPort() );
  viewer->SetColorWindow(18);
  viewer->SetColorLevel(9);
  viewer->SetSize(1024,1024);
  filter->Delete();

  vtkRenderWindowInteractor* it = vtkRenderWindowInteractor::New();
  viewer->SetupInteractor(it);

  it->Initialize();
  it->Start();
  it->Delete();
  viewer->Delete();
#else
  filter->Update();
  filter->Delete();
#endif

  return 0;
  }
