#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAxesActor.h"

#include "vtkDataObject.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"
#ifdef VTK_CAN_USE_SMP
  #include "vtkSMPContourFilter2.h"
  #include "vtkSMPPipeline.h"
#else
  #include "vtkContourFilter.h"
  #include "vtkCompositeDataPipeline.h"
#endif

int main( int argc, char** argv )
  {
  if (argc < 2)
    {
    cout << "You must provide at least one file name." << endl;
    return 1;
    }
#ifdef VTK_CAN_USE_SMP
  vtkSMPPipeline* p = vtkSMPPipeline::New();
#else
  vtkCompositeDataPipeline* p = vtkCompositeDataPipeline::New();
#endif
  vtkAlgorithm::SetDefaultExecutivePrototype(p);
  p->Delete();

  vtkPolyDataReader* reader = vtkPolyDataReader::New();
  reader->SetFileName(argv[1]);

#ifdef VTK_CAN_USE_SMP
  vtkSMPContourFilter2* filter = vtkSMPContourFilter2::New();
#else
  vtkContourFilter* filter = vtkContourFilter::New();
#endif
  filter->SetInputConnection( reader->GetOutputPort() );
  filter->GenerateValues(11, 0., 1.);
  filter->UseScalarTreeOff();
  reader->Delete();

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( filter->GetOutputPort() );
  filter->Delete();

  vtkTransform* t = vtkTransform::New();
  t->RotateWXYZ(45.,1.,1.,1.);
  transform->SetTransform(t);
  t->Delete();

  /* === Pipeline pull === */
#ifdef HIDE_VTK_WINDOW
  transform->Update();
  transform->GetOutputDataObject(0)->Print( cout );
  transform->Delete();
#else
  vtkCompositePolyDataMapper* mapper = vtkCompositePolyDataMapper::New();
  mapper->SetInputConnection( transform->GetOutputPort() );
  transform->Delete();

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(mapper);
  mapper->Delete();

  vtkRenderer* ren = vtkRenderer::New();
  ren->AddActor(actor);
  actor->Delete();

  vtkRenderWindow* win = vtkRenderWindow::New();
  win->AddRenderer(ren);
  ren->Delete();

  vtkRenderWindowInteractor* inter = vtkRenderWindowInteractor::New();
  inter->SetRenderWindow(win);
  win->Delete();

  inter->Initialize();
  inter->Start();

  inter->Delete();
#endif

  return 0;
  }
