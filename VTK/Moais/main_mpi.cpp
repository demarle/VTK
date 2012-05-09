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

#include <mpi.h>

static const float ISO_START=4250.0;
static const float ISO_STEP=-1250.0;
static const int ISO_NUM=3;
// Just pick a tag which is available
static const int ISO_VALUE_RMI_TAG=300;
static const int ISO_OUTPUT_TAG=301;


// This will be called by all processes
void MyMain( vtkMultiProcessController *controller, void *arg )
{
  vtkEnSightGoldBinaryReader* reader = reinterpret_cast<vtkEnSightGoldBinaryReader*>(arg);
  int myid = controller->GetLocalProcessId();

  vtkTransformFilter* pre_transform = vtkTransformFilter::New();
  pre_transform->SetInputConnection( reader->GetOutput()->GetBlock(myid)->GetProducerPort() );

  vtkTransformFilter* transform = vtkTransformFilter::New();
  transform->SetInputConnection( pre_transform->GetOutputPort() );
  pre_transform->Delete();

  vtkTransform* t = vtkTransform::New();
  t->Scale( -1., -1., -1. );
  pre_transform->SetTransform( t );
  transform->SetTransform( t );
  t->Delete();

  vtkContourFilter* isosurface = vtkContourFilter::New();
  isosurface->SetInputConnection( transform->GetOutputPort() );
  isosurface->GenerateValues( 11, 0.0, 1.0 );
  isosurface->UseScalarTreeOff();
  transform->Delete();

  isosurface->Update();

  if (myid != 0)
    {
    controller->Send(isosurface->GetOutput(), 0, ISO_OUTPUT_TAG);
    }
  else
    {
    // Create the rendering part of the pipeline
    vtkAppendPolyData *app = vtkAppendPolyData::New();
    vtkRenderer *ren = vtkRenderer::New();
    vtkRenderWindow *renWindow = vtkRenderWindow::New();
    vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
    vtkActor *actor = vtkActor::New();
    renWindow->AddRenderer(ren);
    ren->SetBackground( .5, .5, .5 );
    iren->SetRenderWindow(renWindow);
    mapper->SetInputConnection(app->GetOutputPort());
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    for (int i = 1; i < controller->GetNumberOfProcesses(); ++i)
      {
      vtkPolyData* pd = vtkPolyData::New();
      controller->Receive(pd, i, ISO_OUTPUT_TAG);
      app->AddInput(pd);
      pd->Delete();

//      controller->TriggerRMI(i, vtkMultiProcessController::BREAK_RMI_TAG);
      }

    vtkPolyData* outputCopy = vtkPolyData::New();
    outputCopy->ShallowCopy(isosurface->GetOutput());
    app->AddInput(outputCopy);
    outputCopy->Delete();
    app->Update();
    renWindow->Render();

    iren->Start();

    // Clean up
    app->Delete();
    ren->Delete();
    renWindow->Delete();
    iren->Delete();
    mapper->Delete();
    actor->Delete();
    }

  // clean up objects in all processes.
  isosurface->Delete();
}


int main( int argc, char* argv[] )
{
  // This is here to avoid false leak messages from vtkDebugLeaks when
  // using mpich. It appears that the root process which spawns all the
  // main processes waits in MPI_Init() and calls exit() when
  // the others are done, causing apparent memory leaks for any objects
  // created before MPI_Init().

  vtkEnSightGoldBinaryReader* reader = vtkEnSightGoldBinaryReader::New();
  reader->SetFilePath(argv[1]);
  reader->SetCaseFileName("bunny.0.case");
  reader->Update();

  MPI_Init(&argc,&argv);
//  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  // Note that this will create a vtkMPIController if MPI
  // is configured, vtkThreadedController otherwise.
  vtkMPIController* controller = vtkMPIController::New();
  controller->Initialize(&argc, &argv, 1);

  vtkParallelFactory* pf = vtkParallelFactory::New();
  vtkObjectFactory::RegisterFactory(pf);
  pf->Delete();

  controller->SetSingleMethod(MyMain, reader);
  controller->SingleMethodExecute();

  controller->Finalize();
  controller->Delete();

  reader->Delete();

  return 0;
}





