/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestSQLiteTableReadWrite.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME Test of vtkTableToSQLiteWriter and vtkSQLiteToTableReader
// .SECTION Description
//

#include "vtkSmartPointer.h"
#include "vtkSQLiteDatabase.h"
#include "vtkSQLQuery.h"
#include "vtkTable.h"
#include "vtkTableReader.h"
#include "vtkTableWriter.h"

#include "vtkTableToSQLiteWriter.h"
#include "vtkSQLiteToTableReader.h"

#include "vtksys/SystemTools.hxx"
#include <sys/stat.h>

void PrintFile(const char* name, std::ostream& os);

int TestSQLiteTableReadWrite(int argc, char *argv[])
{
  if ( argc <= 1 )
    {
    std::cerr << "Usage: " << argv[0] << " <.vtk table file>" << std::endl;
    return 1;
    }
  std::cerr << "reading a vtkTable from file" << std::endl;
  vtkSmartPointer<vtkTableReader> tableFileReader =
    vtkSmartPointer<vtkTableReader>::New();
  tableFileReader->SetFileName(argv[1]);
  vtkTable *table = tableFileReader->GetOutput();
  tableFileReader->Update();

  std::cerr << "opening an SQLite database connection" << std::endl;
  vtkSQLiteDatabase* db = vtkSQLiteDatabase::SafeDownCast(
    vtkSQLDatabase::CreateFromURL( "sqlite://local.db" ) );
  bool status = db->Open("", vtkSQLiteDatabase::CREATE_OR_CLEAR);
  if ( ! status )
    {
    std::cerr << "Couldn't open database using CREATE_OR_CLEAR.\n";
    return 1;
    }

  std::cerr << "creating an SQLite table from a vtkTable" << std::endl;
  vtkSmartPointer<vtkTableToSQLiteWriter> writerToTest =
    vtkSmartPointer<vtkTableToSQLiteWriter>::New();

  writerToTest->SetInputData(table);
  writerToTest->SetDatabase(db);
  writerToTest->SetTableName("tableTest");
  writerToTest->Update();

  std::cerr << "converting it back to a vtkTable" << std::endl;
  vtkSmartPointer<vtkSQLiteToTableReader> readerToTest =
    vtkSmartPointer<vtkSQLiteToTableReader>::New();

  readerToTest->SetDatabase(db);
  readerToTest->SetTableName("tableTest");
  readerToTest->Update();

  std::cerr << "writing the table out to disk" << std::endl;
  vtkSmartPointer<vtkTableWriter> tableFileWriter =
    vtkSmartPointer<vtkTableWriter>::New();
  tableFileWriter->SetFileName("TestSQLiteTableReadWrite.vtk");
  tableFileWriter->SetInputConnection(readerToTest->GetOutputPort());
  tableFileWriter->Update();

  std::cerr << "verifying that it's the same as what we started with...";
  int result = 0;
  if(vtksys::SystemTools::FilesDiffer(argv[1], "TestSQLiteTableReadWrite.vtk"))
    {
    std::cerr << argv[1] << " differs from TestSQLiteTableReadWrite.vtk" << std::endl;
    PrintFile(argv[1], std::cerr);
    PrintFile("TestSQLiteTableReadWrite.vtk", std::cerr);
    result = 1;
    }
  else
    {
    std::cerr << "it is!" << std::endl;
    }

  //drop the table we created
  vtkSQLQuery* query = db->GetQueryInstance();
  query->SetQuery("DROP TABLE tableTest");
  query->Execute();

  //clean up memory
  db->Delete();
  query->Delete();

  return result;
}

void PrintFile(const char* name, std::ostream& os)
{
  const char* div = "=======================================================================";
  // Preserve valuable output regardless of the limits set in
  // CMake/CTestCustom.cmake
  os << "CTEST_FULL_OUTPUT\n";
  os << "File \"" << name << "\"";
  struct stat fs;
  if(stat(name, &fs) != 0)
    {
    os << " does not exist.\n";
    return;
    }
  else
    {
    os << " has " << fs.st_size << " bytes";
    }

  std::ifstream fin(name);
  if(fin)
    {
    os << ":\n" << div << "\n";
    os << fin.rdbuf();
    os << div << "\n";
    os.flush();
    }
  else
    {
    os << " but cannot be opened for read.\n";
    }
}
