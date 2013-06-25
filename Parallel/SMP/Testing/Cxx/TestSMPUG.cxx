#include "vtkActor.h"
#include "vtkAssignAttribute.h"
#include "vtkDataSetMapper.h"
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


int TestSMPUG( int argc, char * argv [] )
{
  vtkTimerLog *timer = vtkTimerLog::New();
  double t0, t1;

  int threads = 0;
  if (argc > 1)
    {
    threads = atoi(argv[1]);
    }
  bool sequential = (threads==0);

  vtkRTAnalyticSource* wavelet = vtkRTAnalyticSource::New();
#define SMP_R 15
  wavelet->SetWholeExtent(-SMP_R,SMP_R,-SMP_R,SMP_R,-SMP_R,SMP_R);
  wavelet->SetCenter(0,0,0);
  wavelet->SetMaximum(255);
  wavelet->SetXFreq(60);
  wavelet->SetYFreq(30);
  wavelet->SetZFreq(40);
  wavelet->SetXMag(10);
  wavelet->SetYMag(18);
  wavelet->SetZMag(5);
  wavelet->SetStandardDeviation(0.5);
  wavelet->SetSubsampleRate(1);

  //convert to unstructured grid
  vtkThreshold* threshold = vtkThreshold::New();
  threshold->SetInputConnection( wavelet->GetOutputPort() );
  threshold->ThresholdBetween(0,3000);
  wavelet->Delete();

  vtkAssignAttribute* aa = vtkAssignAttribute::New();
  aa->SetInputConnection(threshold->GetOutputPort());
  aa->Assign("RTData", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( aa->GetOutputPort() );
  aa->Delete();

  if ( sequential )
    {
    vtkTransform* t = vtkTransform::New();
    t->Scale( 1, 2, 3 );
    transform->SetTransform( t );
    cerr << "update " << t->GetClassName() << endl;
    t->Delete();
    }
  else
    {
    vtkSMPTransform* t = vtkSMPTransform::New();
    t->Scale( 1, 2, 3 );
    transform->SetTransform( t );
    cerr << "update " << t->GetClassName() << endl;
    t->Delete();
    }

#define REPS 25
  t0 = timer->GetCPUTime();
  for (int i = 0; i < REPS; ++i)
    {
    transform->Modified();
    transform->Update();
    }
  t1 = timer->GetCPUTime();
  cerr << (t1-t0)/REPS << endl;

  /* === Testing contour filter === */

  vtkContourFilter* isosurface;
  if ( sequential )
    {
    isosurface = vtkContourFilter::New();
    }
  else
    {
    isosurface = vtkSMPContourFilter::New();

    vtkSMPMergePoints* locator = vtkSMPMergePoints::New();
    isosurface->SetLocator( locator );
    locator->Delete();
    vtkSMPMinMaxTree* tree = vtkSMPMinMaxTree::New();
    isosurface->SetScalarTree(tree);
    tree->Delete();
    }

  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->SetInputArrayToProcess(0,0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "RTData");
  isosurface->GenerateValues( 11, 30.0, 300.0 );
  isosurface->UseScalarTreeOn();


  cerr << "update " << isosurface->GetClassName() << endl;
  double t = 0;

  for (int i = 0; i < REPS; ++i)
    {
    isosurface->Modified();
    t0 = timer->GetCPUTime();
    isosurface->Update();
    t1 = timer->GetCPUTime();
    t += t1-t0;
    }
  cerr << (t)/REPS << endl;

  double b[6];
  threshold->GetOutput()->GetBounds(b);

  vtkDataSetMapper* map1 = vtkDataSetMapper::New();
  map1->SetInputConnection( threshold->GetOutputPort() );
  threshold->Delete();

  vtkActor* actor1 = vtkActor::New();
  actor1->SetMapper( map1 );
  map1->Delete();

  vtkDataSetMapper* map2 = vtkDataSetMapper::New();
  map2->SetInputConnection( transform->GetOutputPort() );
  transform->Delete();

  vtkActor* actor2 = vtkActor::New();
  actor2->SetMapper( map2 );
  map2->Delete();
  actor2->AddPosition( (b[1]-b[0])*1.1, 0., 0. );

  vtkDataSetMapper* map3 = vtkDataSetMapper::New();
  map3->SetInputConnection( isosurface->GetOutputPort() );
  isosurface->Delete();

  vtkActor* actor3 = vtkActor::New();
  actor3->SetMapper( map3 );
  map3->Delete();
  actor3->AddPosition( (b[1]-b[0])*2.1, 0., 0. );

  vtkRenderer* viewport = vtkRenderer::New();
  viewport->SetBackground( .5, .5, .5 );
  viewport->AddActor( actor1 );
  viewport->AddActor( actor2 );
  viewport->AddActor( actor3 );
  actor1->Delete();
  actor2->Delete();
  actor3->Delete();

  vtkRenderWindow* window = vtkRenderWindow::New();
  window->AddRenderer( viewport );
  viewport->Delete();

  vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow( window );
  window->Delete();

  iren->Initialize();

  iren->Start();

  iren->Delete();

  timer->Delete();
  return 0;
}
