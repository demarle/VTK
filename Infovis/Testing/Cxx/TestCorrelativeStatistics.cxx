/*
 * Copyright 2007 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */

#include "vtkDoubleArray.h"
#include "vtkTable.h"
#include "vtkCorrelativeStatistics.h"

//=============================================================================
int TestCorrelativeStatistics( int, char *[] )
{
  int testIntValue = 0;

  double mingledData[] = 
    {
    46,
    45,
    47,
    49,
    46,
    47,
    46,
    46,
    47,
    46,
    47,
    49,
    49,
    49,
    47,
    45,
    50,
    50,
    46,
    46,
    51,
    50,
    48,
    48,
    52,
    54,
    48,
    47,
    52,
    52,
    49,
    49,
    53,
    54,
    50,
    50,
    53,
    54,
    50,
    52,
    53,
    53,
    50,
    51,
    54,
    54,
    49,
    49,
    52,
    52,
    50,
    51,
    52,
    52,
    49,
    47,
    48,
    48,
    48,
    50,
    46,
    48,
    47,
    47,
    };
  int nVals = 32;
;

  vtkDoubleArray* dataset1Arr = vtkDoubleArray::New();
  dataset1Arr->SetNumberOfComponents( 1 );
  dataset1Arr->SetName( "Metric 1" );

  vtkDoubleArray* dataset2Arr = vtkDoubleArray::New();
  dataset2Arr->SetNumberOfComponents( 1 );
  dataset2Arr->SetName( "Metric 2" );

  for ( int i = 0; i < nVals; ++ i )
    {
    int ti = i << 1;
    dataset1Arr->InsertNextValue( mingledData[ti] );
    dataset2Arr->InsertNextValue( mingledData[ti + 1] );
    }

  vtkTable* datasetTable = vtkTable::New();
  datasetTable->AddColumn( dataset1Arr );
  dataset1Arr->Delete();
  datasetTable->AddColumn( dataset2Arr );
  dataset2Arr->Delete();

  double nomValue[] = { 49.2188, 49.5 };
  double allowedDev[] = { 5.98286, 7.54839, 6.14516 };
  double threshold = .2;

  vtkDoubleArray* paramsArr = vtkDoubleArray::New();
  paramsArr->SetNumberOfComponents( 1 );
  paramsArr->SetName( "Params" );
  paramsArr->InsertNextValue( nomValue[0] );
  paramsArr->InsertNextValue( nomValue[1] );
  paramsArr->InsertNextValue( allowedDev[0] );
  paramsArr->InsertNextValue( allowedDev[1] );
  paramsArr->InsertNextValue( allowedDev[2] );
  paramsArr->InsertNextValue( threshold );

  vtkTable* paramsTable = vtkTable::New();
  paramsTable->AddColumn( paramsArr );
  paramsArr->Delete();

  vtkCorrelativeStatistics* haruspex = vtkCorrelativeStatistics::New();
  haruspex->SetInput( 0, datasetTable );
  haruspex->SetInput( 1, paramsTable );
  vtkTable* outputTable = haruspex->GetOutput();

  datasetTable->Delete();
  paramsTable->Delete();

// -- Select Column Pair of Interest -- 
  haruspex->AddColumnPair( 0, 1 );

// -- Test Learn Mode -- 
  haruspex->SetExecutionMode( vtkStatisticsAlgorithm::LearnMode );
  haruspex->Update();
  vtkIdType n = haruspex->GetSampleSize();

  cout << "## Calculated the following statistics ( "
       << n
       << " entries per column ):\n";
  for ( vtkIdType r = 0; r < outputTable->GetNumberOfRows(); ++ r )
    {
    cout << "   X: "
         << datasetTable->GetColumnName( outputTable->GetValue( r, 0 ).ToInt() )
         << ", Y: "
         << datasetTable->GetColumnName( outputTable->GetValue( r, 1 ).ToInt() )
         << ":";

    for ( int i = 2; i < 7; ++ i )
      {
      cout << " "
           << outputTable->GetColumnName( i )
           << ": "
           << outputTable->GetValue( r, i ).ToDouble();
      }

    cout << "\n   Y = "
       << outputTable->GetValue( r,  7 ).ToDouble()
       << " * X + "
       << outputTable->GetValue( r,  8 ).ToDouble()
       << ", X = "
       << outputTable->GetValue( r,  9 ).ToDouble()
       << " * Y + "
       << outputTable->GetValue( r, 10 ).ToDouble()
       << ", corr. coeff.: "
       << outputTable->GetValue( r, 11 ).ToDouble()
       << "\n";
    }

// -- Test Evince Mode -- 
  cout << "## Searching for outliers with relative PDF < "
       << threshold
       << "\n"
       << "   PDF: Bivariate Gaussian with mean ( "
       << nomValue[0]
       << "  "
       << nomValue[1]
       << " ) and covariance [ "
       << allowedDev[0]
       << "  "
       << allowedDev[2]
       << " ; "
       << allowedDev[2]
       << "  "
       << allowedDev[1]
       << " ].\n";

  haruspex->SetExecutionMode( vtkStatisticsAlgorithm::EvinceMode );
  haruspex->Update();

  testIntValue = outputTable->GetNumberOfRows();
  if ( testIntValue != 7 )
    {
    cerr << "Reported an incorrect number of outliers: "
         << testIntValue
         << " != 10.\n";
    return 1;
    }

  cout << "Found "
       << testIntValue
       << " outliers:\n";

  for ( vtkIdType r = 0; r < outputTable->GetNumberOfRows(); ++ r )
    {
    int i = outputTable->GetValue( r, 0 ).ToInt();
    cout << "   "
         << i
         << "-th double ( "
         << datasetTable->GetValue( i, 0 ).ToDouble()
         << " , "
         << datasetTable->GetValue( i, 1 ).ToDouble()
         << " ) has a relative PDF of "
         << outputTable->GetValue( r, 1 ).ToDouble()
         << "\n";
    }

  haruspex->Delete();

  return 0;
}
