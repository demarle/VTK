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

bool isPNG(const std::string& n)
  {
  size_t index = n.find_last_of('.');
  std::string s = n.substr(index+1);
  return s == "png";
  }

int main( int argc, char** argv )
  {
  if (argc < 2)
    {
    cout << "You must provide at least one file name." << endl;
    return 1;
    }

  vtkImageReader2* image1 = 0;
  vtkImageReader2* image2 = 0;

  std::string name1(argv[1]);
  std::string name2(name1);
  if (argc > 2)
    {
    name2 = argv[2];
    }

  if(isPNG(name1))
    {
    image1 = vtkPNGReader::New();
    }
  else
    {
    image1 = vtkJPEGReader::New();
    }

  if(isPNG(name2))
    {
    image2 = vtkPNGReader::New();
    }
  else
    {
    image2 = vtkJPEGReader::New();
    }

  image1->SetFileName(name1.c_str());
  image2->SetFileName(name2.c_str());

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
