#include "vtkActor.h"
#include "vtkAssignAttribute.h"
#include "vtkContourFilter.h"
#include "vtkDataSetMapper.h"
#include "vtkDataSetReader.h"
#include "vtkElevationFilter.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRTAnalyticSource.h"
#include "vtkSMPContourFilter.h"
#include "vtkSMPMergePoints.h"
#include "vtkSMPMinMaxTree.h"
#include "vtkSMPTransform.h"
#include "vtkTestUtilities.h"
#include "vtkThreshold.h"
#include "vtkTimerLog.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkUnstructuredGrid.h"

#include <cstdlib>

#define REPS 5

void setupTest(vtkDataSetReader* reader, vtkContourFilter* isosurface, bool sequential = true)
{
  vtkTimerLog *timer = vtkTimerLog::New();
  double t, t0,t1;

  cerr << "/************* ";
  if (sequential)
   cerr << "Sequential";
  else
   cerr << "SMP";
  cerr << " *************/" << endl;

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( reader->GetOutputPort() );
  if (sequential)
    {
    vtkTransform* tr = vtkTransform::New();
    tr->Identity();
    transform->SetTransform(tr);
    tr->Delete();
    }
  else
    {
    vtkSMPTransform* tr = vtkSMPTransform::New();
    tr->Identity();
    transform->SetTransform(tr);
    tr->Delete();
    }

  isosurface->SetInputConnection( transform->GetOutputPort() );
  transform->Delete();
  isosurface->GenerateValues(11,0.0,1.0);
  isosurface->UseScalarTreeOn();
  cerr << "First " << transform->GetClassName() << " execution" << endl;
  t0 = timer->GetUniversalTime();
  transform->Update();
  t1 = timer->GetUniversalTime();
  cerr << t1-t0 << endl;
  cerr << "First " << isosurface->GetClassName() << " execution" << endl;
  t0 = timer->GetUniversalTime();
  isosurface->Update();
  t1 = timer->GetUniversalTime();
  cerr << t1-t0 << endl;

  cerr << "Average time for " << REPS << " other executions" << endl;
  t = 0.0;
  for (int i = 0; i < REPS; ++i)
    {
    transform->Modified();
    t0 = timer->GetUniversalTime();
    transform->Update();
    t1 = timer->GetUniversalTime();
    t += t1-t0;
    }
  cerr << "Transform: " << (t)/REPS << endl;
  t = 0.0;
  for (int i = 0; i < REPS; ++i)
    {
    isosurface->Modified();
    t0 = timer->GetUniversalTime();
    isosurface->Update();
    t1 = timer->GetUniversalTime();
    t += t1-t0;
    }
  cerr << "Isosurface: " << (t)/REPS << endl;
  timer->Delete();
}

int TestSMPPD2( int argc, char * argv [] )
{
  double t0, t1;
  vtkTimerLog* timer;
  if (argc < 1)
    {
    cerr << "You should specify a filename" << endl;
    }
  
  vtkDataSetReader* aa = vtkDataSetReader::New();
  aa->SetFileName(argv[1]);

  cerr << "Reading " << aa->GetFileName() << endl;
  timer = vtkTimerLog::New();
  t0 = timer->GetUniversalTime();
  aa->Update();
  t1 = timer->GetUniversalTime();
  cerr << t1-t0 << endl;
  timer->Delete();


  /* === Testing contour filter === */

  vtkContourFilter* isosurface1 = vtkContourFilter::New();

  vtkContourFilter* isosurface2 = vtkSMPContourFilter::New();
  vtkSMPMergePoints* locator = vtkSMPMergePoints::New();
  isosurface2->SetLocator( locator );
  locator->Delete();
  vtkSMPMinMaxTree* tree = vtkSMPMinMaxTree::New();
  isosurface2->SetScalarTree(tree);
  tree->Delete();

  setupTest(aa,isosurface1);
  setupTest(aa,isosurface2,false);
  aa->Delete();

  /* === Watching outputs === */

  double b[6];
  isosurface1->GetOutput()->GetBounds(b);

  vtkDataSetMapper* map1 = vtkDataSetMapper::New();
  map1->SetInputConnection( isosurface1->GetOutputPort() );
  vtkActor* actor1 = vtkActor::New();
  actor1->SetMapper( map1 );
  map1->Delete();

  vtkDataSetMapper* map2 = vtkDataSetMapper::New();
  map2->SetInputConnection( isosurface2->GetOutputPort() );
  vtkActor* actor2 = vtkActor::New();
  actor2->SetMapper( map2 );
  map2->Delete();
  actor2->AddPosition( (b[1]-b[0])*1.1, 0., 0. );

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );
  viewport->AddActor( actor1 );
  viewport->AddActor( actor2 );
  actor1->Delete();
  actor2->Delete();

  vtkRenderWindow* window = vtkRenderWindow::New();
  window->AddRenderer( viewport );
  viewport->Delete();

  vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow( window );
  window->Delete();

  iren->Initialize();

  iren->Start();

  iren->Delete();

  isosurface1->Delete();
  isosurface2->Delete();

  return 0;
}
