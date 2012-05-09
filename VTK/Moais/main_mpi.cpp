/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: ParallelIso.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This example demonstrates the use of data parallelism in VTK. The
// pipeline ( vtkImageReader -> vtkContourFilter -> vtkElevationFilter )
// is created in parallel and each process is assigned 1 piece to process.
// All satellite processes send the result to the first process which
// collects and renders them.

#include "vtkActor.h"
#include "vtkAppendPolyData.h"
#include "vtkCamera.h"
#include "vtkConeSource.h"
#include "vtkContourFilter.h"
#include "vtkDataSet.h"
#include "vtkElevationFilter.h"
#include "vtkImageReader.h"
#include "vtkMath.h"
#include "vtkMPIController.h"
#include "vtkParallelFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkWindowToImageFilter.h"
#include "vtkImageData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkEnSightGoldBinaryReader.h"
#include "vtkTransformFilter.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkTransform.h"

#include "vtkDebugLeaks.h"

#include "vtkUnstructuredGrid.h"
#include "vtkAppendFilter.h"
#include "vtkPointSet.h"
#include "vtkBenchTimer.h"
#include "mpi.h"
#include <cstdlib>

static const float ISO_START=4250.0;
static const float ISO_STEP=-1250.0;
static const int ISO_NUM=3;
// Just pick a tag which is available
static const int ISO_VALUE_RMI_TAG=300;
static const int ISO_OUTPUT_TAG=301;
static const int PT_OUTPUT_TAG=302;
static const int T_OUTPUT_TAG=303;


// This will be called by all processes
void MyMain( vtkMultiProcessController *controller, void *arg )
{
  vtkEnSightGoldBinaryReader* reader = reinterpret_cast<vtkEnSightGoldBinaryReader*>(arg);
  int myid = controller->GetLocalProcessId();
  int numproc = controller->GetNumberOfProcesses();
  cout << "process " << myid << " started" << endl;

  vtkTransformFilter* pre_transform = vtkTransformFilter::New();
  pre_transform->SetInputConnection( reader->GetOutput()->GetBlock(myid)->GetProducerPort() );

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( pre_transform->GetOutputPort() );

  vtkTransform* t = vtkTransform::New();
  t->Scale( -1., -1., -1. );
  pre_transform->SetTransform( t );
  transform->SetTransform( t );
  t->Delete();

  vtkContourFilter* isosurface = vtkContourFilter::New();
  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->GenerateValues( 11, 0.0, 1.0 );
  isosurface->UseScalarTreeOff();

  vtkBenchTimer* timer = vtkBenchTimer::New();
  int runs = 0;
  while (runs < 50)
    {
    if (myid != 0)
      {
      pre_transform->Modified();
      pre_transform->Update();
      controller->Send(pre_transform->GetOutput(), 0, PT_OUTPUT_TAG);
      }
    else
      {
      if (!runs) cout << "Transform1" << endl;
      timer->start_bench_timer();
      pre_transform->Modified();
      pre_transform->Update();
      vtkAppendFilter* app = vtkAppendFilter::New();
      for (int i = 1; i < numproc; ++i)
        {
        vtkUnstructuredGrid* ps = vtkUnstructuredGrid::New();
        controller->Receive(ps, i, PT_OUTPUT_TAG);
        app->AddInput(ps);
        ps->Delete();
        }
      vtkUnstructuredGrid* outcp = vtkUnstructuredGrid::New();
      outcp->ShallowCopy(pre_transform->GetOutput());
      app->AddInput(outcp);
      outcp->Delete();
      app->Update();
      app->Delete();
      timer->end_bench_timer();
      cout << endl;
      }
    ++runs;
    }

  runs = 0;
  while (runs < 50)
    {
    if (myid != 0)
      {
      transform->Modified();
      transform->Update();
      controller->Send(pre_transform->GetOutput(), 0, T_OUTPUT_TAG);
      }
    else
      {
      if (!runs) cout << "Transform2" << endl;
      timer->start_bench_timer();
      transform->Modified();
      transform->Update();
      vtkAppendFilter* app = vtkAppendFilter::New();
      for (int i = 1; i < numproc; ++i)
        {
        vtkUnstructuredGrid* ps = vtkUnstructuredGrid::New();
        controller->Receive(ps, i, T_OUTPUT_TAG);
        app->AddInput(ps);
        ps->Delete();
        }
      vtkUnstructuredGrid* outcp = vtkUnstructuredGrid::New();
      outcp->ShallowCopy(transform->GetOutput());
      app->AddInput(outcp);
      outcp->Delete();
      app->Update();
      app->Delete();
      timer->end_bench_timer();
      cout << endl;
      }
    ++runs;
    }

  runs = 0;
  while (runs < 50)
    {
    if (myid != 0)
      {
      isosurface->Modified();
      isosurface->Update();
      controller->Send(pre_transform->GetOutput(), 0, ISO_OUTPUT_TAG);
      }
    else
      {
      if (!runs) cout << "Isosurface" << endl;
      timer->start_bench_timer();
      isosurface->Modified();
      isosurface->Update();
      vtkAppendPolyData *app = vtkAppendPolyData::New();
      for (int i = 1; i < numproc; ++i)
        {
        vtkPolyData* pd = vtkPolyData::New();
        controller->Receive(pd, i, ISO_OUTPUT_TAG);
        app->AddInput(pd);
        pd->Delete();
        }
      vtkPolyData* outcp = vtkPolyData::New();
      outcp->ShallowCopy(isosurface->GetOutput());
      app->AddInput(outcp);
      outcp->Delete();
      app->Update();
      app->Delete();
      timer->end_bench_timer();
      cout << endl;
      }
    ++runs;
    }

  // clean up objects in all processes.
  pre_transform->Delete();
  transform->Delete();
  isosurface->Delete();
}


int main( int argc, char* argv[] )
{
  // This is here to avoid false leak messages from vtkDebugLeaks when
  // using mpich. It appears that the root process which spawns all the
  // main processes waits in MPI_Init() and calls exit() when
  // the others are done, causing apparent memory leaks for any objects
  // created before MPI_Init().

  int provided;
//  MPI_Init(&argc,&argv);
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  // Note that this will create a vtkMPIController if MPI
  // is configured, vtkThreadedController otherwise.
  vtkMPIController* controller = vtkMPIController::New();
  controller->Initialize(&argc, &argv, 1);
  controller->SetNumberOfProcesses(atoi(argv[2]));

  vtkParallelFactory* pf = vtkParallelFactory::New();
  vtkObjectFactory::RegisterFactory(pf);
  pf->Delete();

  vtkEnSightGoldBinaryReader* reader = vtkEnSightGoldBinaryReader::New();
  reader->SetFilePath(argv[1]);
  reader->SetCaseFileName("lucy_bin.0.case");
  reader->Update();

  controller->SetSingleMethod(MyMain, reader);
  controller->SingleMethodExecute();

  controller->Finalize();
  controller->Delete();

  reader->Delete();

  return 0;
}





