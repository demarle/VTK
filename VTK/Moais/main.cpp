#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAxesActor.h"

#include "vtkPNGReader.h"
#include "vtkJPEGReader.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageViewer.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPImageGaussianSmooth.h"
#endif

int main( int argc, char** argv )
  {
  if (argc < 2)
    {
    cout << "You must provide at least one file name." << endl;
    return 1;
    }

  vtkJPEGReader* reader = vtkJPEGReader::New();
  reader->SetFileName(argv[1]);

#ifdef VTK_CAN_USE_SMP
  vtkSMPImageGaussianSmooth* filter = vtkSMPImageGaussianSmooth::New();
#else
  vtkImageGaussianSmooth* filter = vtkImageGaussianSmooth::New();
#endif
  filter->SetInputConnection( reader->GetOutputPort() );
  filter->SetDimensionality(3);
  filter->SetStandardDeviation(.3,.3,.3);
  reader->Delete();

  /* === Pipeline pull === */
#ifdef HIDE_VTK_WINDOW
  filter->Update();
  filter->Delete();
#else
  vtkImageViewer* viewer = vtkImageViewer::New();
  viewer->SetInputConnection( filter->GetOutputPort() );
  viewer->SetColorWindow(18);
  viewer->SetColorLevel(9);
  viewer->SetSize(1546,1161);
  filter->Delete();

  vtkRenderWindowInteractor* it = vtkRenderWindowInteractor::New();
  viewer->SetupInteractor(it);

  it->Initialize();
  it->Start();

  it->Delete();
  viewer->Delete();
#endif

  return 0;
  }
