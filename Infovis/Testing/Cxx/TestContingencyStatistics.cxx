/*
 * Copyright 2008 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
// .SECTION Thanks
// Thanks to Philippe Pebay from Sandia National Laboratories 
// for implementing this test.

#include "vtkMultiBlockDataSet.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"
#include "vtkContingencyStatistics.h"

//=============================================================================
int TestContingencyStatistics( int, char *[] )
{
  int testStatus = 0;

  vtkVariant mingledData[] = 
    {
      123,456,80,"HTTP",
      123,789,80,"HTTP",
      123,789,80,"HTTP",
      123,456,80,"HTTP",
      456,123,80,"HTTP",
      456,123,80,"HTTP",
      456,123,8080,"HTTP",
      789,123,1122,"HTTP",
      456,789,80,"HTTP",
      456,789,25,"SMTP",
      456,789,25,"SMTP",
      456,789,25,"SMTP",
      456,789,25,"SMTP",
      123,789,25,"SMTP",
      789,123,80,"SMTP",
      123,456,20,"FTP",
      789,456,20,"FTP",
      789,123,20,"FTP",
      789,123,122,"FTP",
      789,456,20,"FTP",
      789,456,20,"FTP",
    };
  int nVals = 21;

  vtkVariantArray* dataset0Arr = vtkVariantArray::New();
  dataset0Arr->SetNumberOfComponents( 1 );
  dataset0Arr->SetName( "Source" );

  vtkVariantArray* dataset1Arr = vtkVariantArray::New();
  dataset1Arr->SetNumberOfComponents( 1 );
  dataset1Arr->SetName( "Destination" );

  vtkVariantArray* dataset2Arr = vtkVariantArray::New();
  dataset2Arr->SetNumberOfComponents( 1 );
  dataset2Arr->SetName( "Port" );

  vtkVariantArray* dataset3Arr = vtkVariantArray::New();
  dataset3Arr->SetNumberOfComponents( 1 );
  dataset3Arr->SetName( "Protocol" );

  for ( int i = 0; i < nVals; ++ i )
    {
    int ti = i << 2;
    dataset0Arr->InsertNextValue( mingledData[ti] );
    dataset1Arr->InsertNextValue( mingledData[ti + 1] );
    dataset2Arr->InsertNextValue( mingledData[ti + 2] );
    dataset3Arr->InsertNextValue( mingledData[ti + 3] );
    }

  vtkTable* datasetTable = vtkTable::New();
  datasetTable->AddColumn( dataset0Arr );
  dataset0Arr->Delete();
  datasetTable->AddColumn( dataset1Arr );
  dataset1Arr->Delete();
  datasetTable->AddColumn( dataset2Arr );
  dataset2Arr->Delete();
  datasetTable->AddColumn( dataset3Arr );
  dataset3Arr->Delete();

  int nMetricPairs = 3;

  vtkContingencyStatistics* haruspex = vtkContingencyStatistics::New();
  haruspex->SetInput( 0, datasetTable );
  vtkTable* outputData = haruspex->GetOutput( 0 );

  datasetTable->Delete();

// -- Select Column Pair of Interest ( Learn Mode ) -- 
  haruspex->AddColumnPair( "Port", "Protocol" ); // A valid pair
  haruspex->AddColumnPair( "Protocol", "Port" ); // The same valid pair, just reversed
  haruspex->AddColumnPair( "Source", "Port" ); // Another valid pair
  haruspex->AddColumnPair( "Source", "Dummy" ); // An invalid pair

// -- Test Learn Mode -- 
  haruspex->SetLearn( true );
  haruspex->SetAssess( true );
  haruspex->Update();

  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( haruspex->GetOutputDataObject( 1 ) );
  vtkTable* outputSummary = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 0 ) );
  vtkTable* outputContingency = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 1 ) );

  int testIntValue = 0;

  vtkIdType n = outputContingency->GetValueByName( 0, "Cardinality" ).ToInt();
  cout << "## Calculated the following information entropies( grand total: "
       << n
       << " ):\n";

  for ( vtkIdType r = 0; r < outputSummary->GetNumberOfRows(); ++ r )
    {
    cout << "   ("
         << outputSummary->GetValue( r, 0 ).ToString()
         << ", "
         << outputSummary->GetValue( r, 1 ).ToString()
         << "):";

    for ( vtkIdType c = 2; c < outputSummary->GetNumberOfColumns(); ++ c )
      {
      cout << ", "
           << outputSummary->GetColumnName( c )
           << "="
           << outputSummary->GetValue( r, c ).ToDouble();
      }
    cout << "\n";
    }

  cout << "## Calculated the following probabilities:\n";

  // Skip first row which contains data set cardinality
  for ( vtkIdType r = 1; r < outputContingency->GetNumberOfRows(); ++ r )
    {
    cout << "   ("
         << outputSummary->GetValue( outputContingency->GetValue( r, 0 ).ToInt(), 0 ).ToString()
         << ", "
         << outputSummary->GetValue( outputContingency->GetValue( r, 0 ).ToInt(), 1 ).ToString()
         << ") = ("
         << outputContingency->GetValue( r, 1 ).ToString()
         << ", "
         << outputContingency->GetValue( r, 2 ).ToString()
         << ")";

    for ( vtkIdType c = 3; c < outputContingency->GetNumberOfColumns(); ++ c )
      {
      cout << ", "
           << outputContingency->GetColumnName( c )
           << "="
           << outputContingency->GetValue( r, c ).ToDouble();
      }
    
    cout << "\n";

    // Update total cardinality
    testIntValue += outputContingency->GetValueByName( r, "Cardinality" ).ToInt();
    }

  if ( testIntValue != nVals * nMetricPairs )
    {
    vtkGenericWarningMacro("Reported an incorrect total cardinality: " 
                           << testIntValue 
                           << " != " 
                           << nVals * nMetricPairs
                           << ".");
    testStatus = 1;
    }

  outputData->Dump();

  haruspex->Delete();

  return testStatus;
}
