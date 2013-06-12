#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPClipDataSet.h"
#else
  #include "vtkClipDataSet.h"
#endif

int main( int argc, char** argv )
  {
  if (argc < 2)
    {
    cout << "You must provide at least one file name." << endl;
    return 1;
    }
  vtkstd::string filename(argv[1]);
  cout << "Using file " << filename.substr(filename.find_last_of('/')).substr(0,filename.find_last_of('.'));

  vtkPolyDataReader* reader = vtkPolyDataReader::New();
  reader->SetFileName(filename.c_str());

#ifdef VTK_CAN_USE_SMP
  vtkSMPClipDataSet* filter = vtkSMPClipDataSet::New();
#else
  vtkClipDataSet* filter = vtkClipDataSet::New();
#endif
  filter->SetInputConnection( reader->GetOutputPort() );
  filter->SetValue(0.0);
  filter->UseValueAsOffsetOff();
  reader->Delete();

  /* === Pipeline pull === */
#ifdef HIDE_VTK_WINDOW
  filter->Update();
  filter->Delete();
#else
  vtkDataSetSurfaceFilter* pdf = vtkDataSetSurfaceFilter::New();
  pdf->SetInputConnection( filter->GetOutputPort() );
  filter->Delete();

  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection( pdf->GetOutputPort() );
  pdf->Delete();

  vtkActor* obj = vtkActor::New();
  obj->SetMapper(mapper);
  mapper->Delete();

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->AddActor(obj);
  viewport->SetBackground(.3,.3,.3);
  obj->Delete();

  vtkRenderWindow* window = vtkRenderWindow::New();
  window->AddRenderer(viewport);
  viewport->Delete();

  vtkRenderWindowInteractor* it = vtkRenderWindowInteractor::New();
  it->SetRenderWindow(window);
  window->Delete();

  it->Initialize();
  it->Start();

  it->Delete();
#endif

  return 0;
  }
