/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestMySQLDatabase.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
  Copyright (c) Sandia Corporation
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
  ----------------------------------------------------------------------------*/
// .SECTION Thanks
// Thanks to Andrew Wilson and Philippe Pebay from Sandia National Laboratories 
// for implementing this test.

#include "vtkMySQLDatabase.h"
#include "vtkSQLQuery.h"
#include "vtkRowQueryToTable.h"
#include "vtkSQLDatabaseSchema.h"
#include "vtkStdString.h"
#include "vtkTable.h"
#include "vtkVariant.h"
#include "vtkVariantArray.h"
#include "vtkToolkits.h"

#include <vtkstd/vector>

int TestMySQLDatabase( int, char ** const )
{
  vtkMySQLDatabase* db = vtkMySQLDatabase::SafeDownCast( vtkSQLDatabase::CreateFromURL( VTK_MYSQL_TEST_URL ) );
  bool status = db->Open();

  if ( ! status )
    {
    cerr << "Couldn't open database.\n";
    return 1;
    }

  vtkSQLQuery* query = db->GetQueryInstance();
  vtkStdString createQuery( "CREATE TABLE IF NOT EXISTS people (name TEXT, age INTEGER, weight FLOAT)" );
  cout << createQuery << endl;
  query->SetQuery( createQuery.c_str() );
  if ( !query->Execute() )
    {
    cerr << "Create query failed" << endl;
    return 1;
    }

  for ( int i = 0; i < 40; ++ i )
    {
    char insertQuery[200];
    sprintf( insertQuery, "INSERT INTO people VALUES('John Doe %d', %d, %d)",
            i, i, 10*i );
    cout << insertQuery << endl;
    query->SetQuery( insertQuery );
    if ( !query->Execute() )
      {
      cerr << "Insert query " << i << " failed" << endl;
      return 1;
      }
    }

  const char* queryText = "SELECT name, age, weight FROM people WHERE age <= 20";
  query->SetQuery( queryText );
  cerr << endl << "Running query: " << query->GetQuery() << endl;

  cerr << endl << "Using vtkSQLQuery directly to execute query:" << endl;
  if ( !query->Execute() )
    {
    cerr << "Query failed" << endl;
    return 1;
    }

  for ( int col = 0; col < query->GetNumberOfFields(); ++ col )
    {
    if ( col > 0 )
      {
      cerr << ", ";
      }
    cerr << query->GetFieldName( col );
    }
  cerr << endl;
  while ( query->NextRow() )
    {
    for ( int field = 0; field < query->GetNumberOfFields(); ++ field )
      {
      if ( field > 0 )
        {
        cerr << ", ";
        }
      cerr << query->DataValue( field ).ToString().c_str();
      }
    cerr << endl;
    }
  
  cerr << endl << "Using vtkSQLQuery to execute query and retrieve by row:" << endl;
  if ( !query->Execute() )
    {
    cerr << "Query failed" << endl;
    return 1;
    }
  for ( int col = 0; col < query->GetNumberOfFields(); ++ col )
    {
    if ( col > 0 )
      {
      cerr << ", ";
      }
    cerr << query->GetFieldName( col );
    }
  cerr << endl;
  vtkVariantArray* va = vtkVariantArray::New();
  while ( query->NextRow( va ) )
    {
    for ( int field = 0; field < va->GetNumberOfValues(); ++ field )
      {
      if ( field > 0 )
        {
        cerr << ", ";
        }
      cerr << va->GetValue( field ).ToString().c_str();
      }
    cerr << endl;
    }
  va->Delete();

  cerr << endl << "Using vtkRowQueryToTable to execute query:" << endl;
  vtkRowQueryToTable* reader = vtkRowQueryToTable::New();
  reader->SetQuery( query );
  reader->Update();
  vtkTable* table = reader->GetOutput();
  for ( vtkIdType col = 0; col < table->GetNumberOfColumns(); ++ col )
    {
    table->GetColumn( col )->Print( cerr );
    }
  cerr << endl;
  for ( vtkIdType row = 0; row < table->GetNumberOfRows(); ++ row )
    {
    for ( vtkIdType col = 0; col < table->GetNumberOfColumns(); ++ col )
      {
      vtkVariant v = table->GetValue( row, col );
      cerr << "row " << row << ", col " << col << " - "
           << v.ToString() << " ( " << vtkImageScalarTypeNameMacro( v.GetType()) << " )" << endl;
      }
    }

  query->SetQuery( "DROP TABLE people" );
  if ( ! query->Execute() )
    {
    cerr << "DROP TABLE people query failed" << endl;
    return 1;
    }
  
  reader->Delete();
  query->Delete();
  db->Delete();

// ----------------------------------------------------------------------
// Testing transformation of a schema into a MySQL database

  // 1. Create the schema
#include "DatabaseSchemaWith2Tables.cxx"

  // 2. Convert the schema into a MySQL database
  cerr << "@@ Converting the schema into a MySQL database...";

  db = vtkMySQLDatabase::SafeDownCast( vtkSQLDatabase::CreateFromURL( VTK_MYSQL_TEST_URL ) );
  status = db->Open();

  if ( ! status )
    {
    cerr << "Couldn't open database.\n";
    return 1;
    }

  status = db->EffectSchema( schema ); 
  if ( ! status )
    {
    cerr << "Could not effect test schema.\n";
    return 1;
    }
  cerr << " done." << endl;

  // 3. Count tables of the newly created database
  cerr << "@@ Counting tables of the newly created database... ";

  query = db->GetQueryInstance();
  query->SetQuery( "SHOW TABLES" );
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    return 1;
    }

  vtkstd::vector<vtkStdString> tables;
  while ( query->NextRow() )
    {
    tables.push_back( query->DataValue( 0 ).ToString() );
    }

  int numTbl = tables.size();

  if ( numTbl != schema->GetNumberOfTables() )
    {
    cerr << "Found an incorrect number of tables: " 
         << numTbl 
         << " != " 
         << schema->GetNumberOfTables()
         << endl;
    return 1;
    }
  
  cerr << numTbl 
       << " found.\n";

  // 4. Inspect these tables
  cerr << "@@ Inspecting these tables..." << "\n";

  vtkStdString queryStr;
  for ( tblHandle = 0; tblHandle < numTbl; ++ tblHandle )
    {
    vtkStdString tblName( schema->GetTableNameFromHandle( tblHandle ) );
    cerr << "   Table: " 
         << tblName
         << "\n";

    if ( tblName != tables[tblHandle] )
      {
      cerr << "Fetched an incorrect name: " 
           << tables[tblHandle]
           << " != " 
           << tblName
           << endl;
      return 1;
      }
                       
    // 4.1 Check columns
    queryStr = "DESCRIBE ";
    queryStr += tblName;
    query->SetQuery( queryStr );
    if ( ! query->Execute() )
      {
      cerr << "Query failed" << endl;
      return 1;
      }
    
    int numFields = query->GetNumberOfFields();
    int colHandle = 0;
    for ( ; query->NextRow(); ++ colHandle )
      {
      for ( int field = 0; field < numFields; ++ field )
        {
        if ( field )
          {
          cerr << ", ";
          }
        else // if ( field )
          {
          vtkStdString colName ( schema->GetColumnNameFromHandle( tblHandle, colHandle ) );
          if ( colName != query->DataValue( field ).ToString() )
            {
            cerr << "Found an incorrect column name: " 
                 << query->DataValue( field ).ToString()
                 << " != " 
                 << colName
                 << endl;
            return 1;
            }
          cerr << "     Column: ";
          }
        cerr << query->DataValue( field ).ToString().c_str();
        }
      cerr << endl;
      }
    
    if ( colHandle != schema->GetNumberOfColumnsInTable( tblHandle ) )
      {
      cerr << "Found an incorrect number of columns: " 
           << colHandle
           << " != " 
           << schema->GetNumberOfColumnsInTable( tblHandle )
           << endl;
      return 1;
      }

    // 4.2 Check indices
    queryStr = "SHOW INDEX FROM ";
    queryStr += tblName;
    query->SetQuery( queryStr );
    if ( ! query->Execute() )
      {
      cerr << "Query failed" << endl;
      return 1;
      }

    numFields = query->GetNumberOfFields();
    int idxHandle = -1;
    while ( query->NextRow() )
      {
      int cnmHandle = atoi( query->DataValue( 3 ).ToString() ) - 1;
      if ( ! cnmHandle )
        {
        ++ idxHandle;
        }

      vtkStdString colName ( schema->GetIndexColumnNameFromHandle( tblHandle, idxHandle, cnmHandle ) );
      for ( int field = 0; field < numFields; ++ field )
        {
        if ( field )
          {
          cerr << ", ";
          }
        else // if ( field )
          {
          cerr << "     Index: ";
          }
        cerr << query->DataValue( field ).ToString().c_str();
        }
      cerr << endl;

      if ( colName != query->DataValue( 4 ).ToString() )
        {
        cerr << "Fetched an incorrect column name: " 
             << query->DataValue( 4 ).ToString()
             << " != " 
             << colName
             << endl;
        return 1;
        }
      }
    
    if ( idxHandle + 1 != schema->GetNumberOfIndicesInTable( tblHandle ) )
      {
      cerr << "Found an incorrect number of indices: " 
           << idxHandle + 1
           << " != " 
           << schema->GetNumberOfIndicesInTable( tblHandle )
           << endl;
      return 1;
      }
    }

  // 5. Populate these tables using the trigger mechanism
  cerr << "@@ Populating table atable...";

  queryStr = "INSERT INTO atable (somename,somenmbr) VALUES ( 'Bas-Rhin', 67 )";
  query->SetQuery( queryStr );
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    schema->Delete();
    query->Delete();
    db->Delete();
    return 1;
    }

  queryStr = "INSERT INTO atable (somename,somenmbr) VALUES ( 'Hautes-Pyrenees', 65 )";
  query->SetQuery( queryStr );
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    schema->Delete();
    query->Delete();
    db->Delete();
    return 1;
    }

  queryStr = "INSERT INTO atable (somename,somenmbr) VALUES ( 'Vosges', 88 )";
  query->SetQuery( queryStr );
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    schema->Delete();
    query->Delete();
    db->Delete();
    return 1;
    }

  cerr << " done." << endl;

  // 6. Check that the trigger-dependent table has indeed been populated
  cerr << "@@ Checking trigger-dependent table btable...\n";

  queryStr = "SELECT somevalue FROM btable ORDER BY somevalue DESC";
  query->SetQuery( queryStr );
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    schema->Delete();
    query->Delete();
    db->Delete();
    return 1;
    }

  cerr << "   Entries in column somevalue of table btable, in descending order:\n";
  static const char *dpts[] = { "88", "67", "65" };
  int numDpt = 0;
  for ( ; query->NextRow(); ++ numDpt )
    {
    if ( query->DataValue( 0 ).ToString() != dpts[numDpt] )
      {
      cerr << "Found an incorrect value: " 
           << query->DataValue( 0 ).ToString()
           << " != " 
           << dpts[numDpt]
           << endl;
      schema->Delete();
      query->Delete();
      db->Delete();
      return 1;
      }
    cerr << "     " 
         << query->DataValue( 0 ).ToString()
         << "\n";
    }
    
  if ( numDpt != 3 )
    {
    cerr << "Found an incorrect number of entries: " 
         << numDpt
         << " != " 
         << 3
         << endl;
    schema->Delete();
    query->Delete();
    db->Delete();
    return 1;
    }

  cerr << " done." << endl;

  // 7. Drop tables
  cerr << "@@ Dropping these tables...";

  for ( vtkstd::vector<vtkStdString>::iterator it = tables.begin();
        it != tables.end(); ++ it )
    {
    queryStr = "DROP TABLE ";
    queryStr += *it;
    query->SetQuery( queryStr );

    if ( ! query->Execute() )
      {
      cerr << "Query failed" << endl;
      return 1;
      }
    }

  cerr << " done." << endl;

  // Clean up
  db->Delete();
  schema->Delete();
  query->Delete();

  return 0;
}
