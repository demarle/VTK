/*=========================================================================

Program:   Visualization Toolkit
Module:    TestParallelRandomStatisticsMPI.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
 * Copyright 2008 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
// .SECTION Thanks
// Thanks to Philippe Pebay and Ajith Mascarenhas from Sandia National Laboratories
// for implementing this test.

#include <mpi.h>

#include "vtkPCorrelativeStatistics.h"

#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include "vtkMPIController.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStdString.h"
#include "vtkTable.h"
#include "vtkTimerLog.h"
#include "vtkVariantArray.h"

#include "vtksys/CommandLineArguments.hxx"
#include "vtksys/SystemTools.hxx"

struct RealDataCorrelativeStatisticsArgs
{
  int nVals;
  int* retVal;
  int ioRank;
  vtkStdString fileName;
  int argc;
  char** argv;
};

// Calculate the processor id (integer triple), given its rank
void CalculateProcessorId( int *procDim, int rank, int *procId )
{
  int procXY = procDim[0] * procDim[1];

  procId[2] = rank / procXY;
  procId[1] = ( rank - procId[2] * procXY ) / procDim[0];
  procId[0] = ( rank - procId[2] * procXY ) % procDim[0];
}

// Calculate the processor rank given its id(integer triple)
int CalculateProcessorRank( int *procDim, int *procId )
{
  int rank = procId[2] * procDim[0] * procDim[1] +
    procId[1] * procDim[0] + procId[0];

  return rank;
}

// Read a block of data bounded by [low, high] from file into buffer.
// The entire data has dimensions dim
void ReadFloatDataBlockFromFile( ifstream& ifs,
                                 vtkIdType *dim,
                                 int *low,
                                 int *high,
                                 float *buffer )
{
  vtkIdType dimXY = dim[0] * dim[1];
  vtkIdType dimX  = dim[0];

  float *pbuffer = buffer;

  // Set bounds
  vtkIdType bounds[2][3];
  bounds[0][0] = ( low[0] < 0 ? 0 : low[0] );
  bounds[0][1] = ( low[1] < 0 ? 0 : low[1] );
  bounds[0][2] = ( low[2] < 0 ? 0 : low[2] );

  bounds[1][0] = ( high[0] >= dim[0] ? dim[0] - 1 : high[0] );
  bounds[1][1] = ( high[1] >= dim[1] ? dim[1] - 1 : high[1] );
  bounds[1][2] = ( high[2] >= dim[2] ? dim[2] - 1 : high[2] );

  vtkIdType rangeX = bounds[1][0] - bounds[0][0] + 1;
  vtkIdType sizeX = high[0] - low[0] + 1;
  vtkIdType sizeY = high[1] - low[1] + 1;
  vtkIdType sizeXY = sizeX * sizeY;

  // Next position to start writing
  pbuffer += (bounds[0][0] - low[0] );

  // Iterate over 'z'
  for (int z = low[2]; z <= high[2]; z++)
    {
    if (z >= bounds[0][2] && z <= bounds[1][2] )
      {
      vtkIdType offsetZ = z * dimXY;

      // Iterate over 'y'.
      for (int y = low[1]; y <= high[1]; y++)
        {
        if (y >= bounds[0][1] && y <= bounds[1][1] )
          {
          vtkIdType offsetY = y * dimX;
          long long offset = offsetZ + offsetY + bounds[0][0];

          // Seek to point
          ifs.seekg( offset * sizeof(*pbuffer), ios::beg );

          // Get a block of rangeX values
          ifs.read( reinterpret_cast<char *>( pbuffer ),
                    rangeX * sizeof(*pbuffer) );

          // Proceed to next write position
          pbuffer += sizeX;

          if ( ifs.fail() || ifs.eof() )
            {
            vtkGenericWarningMacro("Failed to read data or reached EOF.");
            exit( -1 );
            }
          }
        else
          {
          // Skip one line
          pbuffer += sizeX;
          }
        }
      }
    else
      {
      // Skip one plane
      pbuffer += sizeXY;
      }
    }
}

// Given the data dimensions dataDim, the process dimensions procDim, my
// process id myProcId, set the block bounding box myBlockBounds for my data.
// Also open the data file as filestream ifs.
void SetDataParameters( vtkIdType *dataDim,
                        int *procDim,
                        int *myProcId,
                        const char* fileName,
                        ifstream& ifs,
                        int myBlockBounds[2][3] )
{
  int myDim[3];
  myDim[0] = static_cast<int>( ceil( dataDim[0] / ( 1. * procDim[0] ) ) );
  myDim[1] = static_cast<int>( ceil( dataDim[1] / ( 1. * procDim[1] ) ) );
  myDim[2] = static_cast<int>( ceil( dataDim[2] / ( 1. * procDim[2] ) ) );

  // cout << "My data dim = " << myDim[0] << " x " << myDim[1] <<
  //   " x " << myDim[2] << endl;

  // Determine data bounds
  myBlockBounds[0][0] = myProcId[0] * myDim[0];
  myBlockBounds[0][1] = myProcId[1] * myDim[1];
  myBlockBounds[0][2] = myProcId[2] * myDim[2];

  int mybb0 = myBlockBounds[0][0] + myDim[0] - 1;
  int cast0 = static_cast<int>( dataDim[0] - 1 );
  myBlockBounds[1][0] = ( mybb0 < cast0 ? mybb0 : cast0 );

  int mybb1 = myBlockBounds[0][1] + myDim[1] - 1;
  int cast1 = static_cast<int>( dataDim[1] - 1 );
  myBlockBounds[1][1] = ( mybb1 < cast1 ? mybb1 : cast1 );

  int mybb2 = myBlockBounds[0][2] + myDim[2] - 1;
  int cast2 = static_cast<int>( dataDim[2] - 1 );
  myBlockBounds[1][2] = ( mybb2 < cast2 ? mybb2 : cast2 );

  // Open file
  ifs.open( fileName, ios::in | ios::binary );

  if( ifs.fail() )
    {
    vtkGenericWarningMacro("Error opening  file:"
                           << fileName
                           <<".");
    exit( -1 );
    }
}

// This will be called by all processes
void RealDataCorrelativeStatistics( vtkMultiProcessController* controller, void* arg )
{
  // Get test parameters
  RealDataCorrelativeStatisticsArgs* args = reinterpret_cast<RealDataCorrelativeStatisticsArgs*>( arg );
  *(args->retVal) = 0;

  // Get MPI communicator
  vtkMPICommunicator* com = vtkMPICommunicator::SafeDownCast( controller->GetCommunicator() );

  // Get local rank
  int myRank = com->GetLocalProcessId();
  int procDim[] = { 1, 1 ,1 };
  int myProcId[3];
  CalculateProcessorId( procDim, myRank, myProcId );

  // ************************** Read input data file ****************************

  vtkIdType dataDim[] = { 2025, 1600, 400 };
  ifstream ifs;
  int myBlockBounds[2][3];
  SetDataParameters( dataDim,
                     procDim,
                     myProcId,
                     args->fileName,
                     ifs,
                     myBlockBounds );

  vtkIdType myDataDim[3];
  myDataDim[0] = myBlockBounds[1][0] - myBlockBounds[0][0] + 1;
  myDataDim[1] = myBlockBounds[1][1] - myBlockBounds[0][1] + 1;
  myDataDim[2] = myBlockBounds[1][2] - myBlockBounds[0][2] + 1;
  vtkIdType myDataSize = myDataDim[0] * myDataDim[1] * myDataDim[2];
  float* buffer = new float[myDataSize];

  ReadFloatDataBlockFromFile( ifs,
                              dataDim,
                              myBlockBounds[0],
                              myBlockBounds[1],
                              buffer );

  vtkTable* inputData = vtkTable::New();

  delete [] buffer;

  // ************************** Correlative Statistics **************************

  // Synchronize and start clock
  com->Barrier();
  vtkTimerLog *timer=vtkTimerLog::New();
  timer->StartTimer();

  // Instantiate a parallel correlative statistics engine and set its input
  vtkPCorrelativeStatistics* pcs = vtkPCorrelativeStatistics::New();
  pcs->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, inputData );

  // Select column pairs
  // FIXME

  // Test (in parallel) with Learn, Derive, and Assess options turned on
  pcs->SetLearnOption( true );
  pcs->SetDeriveOption( true );
  pcs->SetAssessOption( true );
  pcs->Update();

  // Get output data and meta tables
  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( pcs->GetOutputDataObject( vtkStatisticsAlgorithm::OUTPUT_MODEL ) );
  vtkTable* outputPrimary = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 0 ) );
  vtkTable* outputDerived = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 1 ) );

  // Synchronize and stop clock
  com->Barrier();
  timer->StopTimer();

  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Completed parallel calculation of correlative statistics (with assessment):\n"
         << "   Total sample size: "
         << outputPrimary->GetValueByName( 0, "Cardinality" ).ToInt()
         << " \n"
         << "   Wall time: "
         << timer->GetElapsedTime()
         << " sec.\n";

    cout << "   Calculated the following primary statistics:\n";
    for ( vtkIdType r = 0; r < outputPrimary->GetNumberOfRows(); ++ r )
      {
      cout << "   ";
      for ( int i = 0; i < outputPrimary->GetNumberOfColumns(); ++ i )
        {
        cout << outputPrimary->GetColumnName( i )
             << "="
             << outputPrimary->GetValue( r, i ).ToString()
             << "  ";
        }
      cout << "\n";
      }

    cout << "   Calculated the following derived statistics:\n";
    for ( vtkIdType r = 0; r < outputDerived->GetNumberOfRows(); ++ r )
      {
      cout << "   ";
      for ( int i = 0; i < outputDerived->GetNumberOfColumns(); ++ i )
        {
        cout << outputDerived->GetColumnName( i )
             << "="
             << outputDerived->GetValue( r, i ).ToString()
             << "  ";
        }
      cout << "\n";
      }
    }

  // Clean up
  pcs->Delete();
  inputData->Delete();
  timer->Delete();

}

//----------------------------------------------------------------------------
int main( int argc, char** argv )
{
  // **************************** MPI Initialization ***************************
  vtkMPIController* controller = vtkMPIController::New();
  controller->Initialize( &argc, &argv );

  // If an MPI controller was not created, terminate in error.
  if ( ! controller->IsA( "vtkMPIController" ) )
    {
    vtkGenericWarningMacro("Failed to initialize a MPI controller.");
    controller->Delete();
    return 1;
    }

  vtkMPICommunicator* com = vtkMPICommunicator::SafeDownCast( controller->GetCommunicator() );

  // Get local rank
  int myRank = com->GetLocalProcessId();

  // ************************** Find an I/O node ********************************
  int* ioPtr;
  int ioRank;
  int flag;

  MPI_Attr_get( MPI_COMM_WORLD,
                MPI_IO,
                &ioPtr,
                &flag );

  if ( ( ! flag ) || ( *ioPtr == MPI_PROC_NULL ) )
    {
    // Getting MPI attributes did not return any I/O node found.
    ioRank = MPI_PROC_NULL;
    vtkGenericWarningMacro("No MPI I/O nodes found.");

    // As no I/O node was found, we need an unambiguous way to report the problem.
    // This is the only case when a testValue of -1 will be returned
    controller->Finalize();
    controller->Delete();

    return -1;
    }
  else
    {
    if ( *ioPtr == MPI_ANY_SOURCE )
      {
      // Anyone can do the I/O trick--just pick node 0.
      ioRank = 0;
      }
    else
      {
      // Only some nodes can do I/O. Make sure everyone agrees on the choice (min).
      com->AllReduce( ioPtr,
                      &ioRank,
                      1,
                      vtkCommunicator::MIN_OP );
      }
    }

  if ( myRank == ioRank )
    {
    cout << "\n# Process "
         << ioRank
         << " will be the I/O node.\n";
    }

  // **************************** Parse command line ***************************
  // If no arguments were provided, terminate in error.
  if ( argc < 2 )
    {
    vtkGenericWarningMacro("No input data arguments were provided.");
    controller->Delete();
    return 1;
    }

  // Set default argument values (some of which are invalid, for mandatory parameters)
  vtkStdString fileName= ""; // invalid
  int* dataDim; // invalid
  int procDim[] = { 1, 1 ,1 };

  // Initialize command line argument parser
  vtksys::CommandLineArguments clArgs;
  clArgs.Initialize( argc, argv );
  clArgs.StoreUnusedArguments( false );

  // Parse input data file name
  clArgs.AddArgument("--file-name",
                     vtksys::CommandLineArguments::SPACE_ARGUMENT,
                     &fileName, "Name of input data file");

  // Parse input data file name
  clArgs.AddArgument("--data-dim",
                     vtksys::CommandLineArguments::MULTI_ARGUMENT,
                     &dataDim, "Dimensions of the input data");

  // Parse process array dimensions
  clArgs.AddArgument("--proc-dim",
                     vtksys::CommandLineArguments::MULTI_ARGUMENT,
                     procDim, "Dimensions of the input data");

  // If incorrect arguments were provided, terminate in error.
  if ( ! clArgs.Parse() )
    {
    vtkGenericWarningMacro("Incorrect input data arguments were provided.");
    return 1;
    }

  // If no file name was provided, terminate in error.
  if ( ! strcmp( fileName.c_str(), "" ) )
    {
    if ( myRank == ioRank )
      {
      vtkGenericWarningMacro("No input data file name was provided.");
      }

    // Terminate cleanly
    controller->Finalize();
    controller->Delete();
    return 1;
    }
  else
    {
    if ( myRank == ioRank )
      {
      cout << "\n# Input data file name: "
           << fileName
           << "\n";
      }
    }

  for ( int ii = 0 ; ii < 3; ++ ii )
    cout << dataDim[ii]
         << " ";
  cout << "\n";

  // ************************** Initialize test *********************************
  // Check how many processes have been made available
  int numProcs = controller->GetNumberOfProcesses();
  if ( myRank == ioRank )
    {
    cout << "\n# Running test with "
         << numProcs
         << " processes...\n";
    }

  // Parameters for regression test.
  int testValue = 0;
  RealDataCorrelativeStatisticsArgs args;
  args.nVals = 100000;
  args.retVal = &testValue;
  args.ioRank = ioRank;
  args.fileName = fileName;
  args.argc = argc;
  args.argv = argv;

  // Execute the function named "process" on both processes
  controller->SetSingleMethod( RealDataCorrelativeStatistics, &args );
  controller->SingleMethodExecute();

  // Clean up and exit
  if ( myRank == ioRank )
    {
    cout << "\n# Test completed.\n\n";
    }

  controller->Finalize();
  controller->Delete();

  return testValue;
}
