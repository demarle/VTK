/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDelimitedTextReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#ifndef __vtkDelimitedTextReader_h
#define __vtkDelimitedTextReader_h

#include "vtkTableAlgorithm.h"
#include "vtkUnicodeString.h" // Needed for vtkUnicodeString
#include "vtkStdString.h" // Needed for vtkStdString

// .NAME vtkDelimitedTextReader - reads in delimited ascii or unicode text files
// and outputs a vtkTable data structure.
//
// .SECTION Description
// vtkDelimitedTextReader is an interface for pulling in data from a
// flat, delimited ascii or unicode text file (delimiter can be any character).
//
// The behavior of the reader with respect to ascii or unicode input
// is controlled by the SetUnicodeCharacterSet() method.  By default
// (without calling SetUnicodeCharacterSet()), the reader will expect
// to read ascii text and will output vtkStdString columns.  Use the
// Set and Get methods to set delimiters that do not contain UTF8 in
// the name when operating the reader in default ascii mode.  If the
// SetUnicodeCharacterSet() method is called, the reader will output
// vtkUnicodeString columns in the output table.  In addition, it is
// necessary to use the Set and Get methods that contain UTF8 in the
// name to specify delimiters when operating in unicode mode.
//
// There is also a special character set US-ASCII-WITH-FALLBACK that
// will treat the input text as ASCII no matter what.  If and when it
// encounters a character with its 8th bit set it will replace that
// character with the code point ReplacementCharacter.  You may use
// this if you have text that belongs to a code page like LATIN9 or
// ISO-8859-1 or friends: mostly ASCII but not entirely.  Eventually
// this class will acquire the ability to read gracefully text from
// any code page, making this option obsolete.
//
// This class emits ProgressEvent for every 100 lines it reads.
//
// .SECTION Thanks
// Thanks to Andy Wilson, Brian Wylie, Tim Shead, and Thomas Otahal
// from Sandia National Laboratories for implementing this class.
//
// .SECTION Caveats
//
// This reader assumes that the first line in the file (whether that's
// headers or the first document) contains at least as many fields as
// any other line in the file.

class VTK_INFOVIS_EXPORT vtkDelimitedTextReader : public vtkTableAlgorithm
{
public:
  static vtkDelimitedTextReader* New();
  vtkTypeMacro(vtkDelimitedTextReader, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specifies the delimited text file to be loaded.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  // Description:
  // Specifies the character set used in the input file.  Valid character set
  // names will be drawn from the list maintained by the Internet Assigned Name
  // Authority at
  //
  //   http://www.iana.org/assignments/character-sets
  //
  // Where multiple aliases are provided for a character set, the preferred MIME name
  // will be used.  vtkUnicodeDelimitedTextReader currently supports "US-ASCII", "UTF-8",
  // "UTF-16", "UTF-16BE", and "UTF-16LE" character sets.
  vtkGetStringMacro(UnicodeCharacterSet);
  vtkSetStringMacro(UnicodeCharacterSet);

  // Description:
  // Specify the character(s) that will be used to separate records.
  // The order of characters in the string does not matter.  Defaults
  // to "\r\n".
  void SetUTF8RecordDelimiters(const char* delimiters);
  const char* GetUTF8RecordDelimiters();
  void SetUnicodeRecordDelimiters(const vtkUnicodeString& delimiters);
  vtkUnicodeString GetUnicodeRecordDelimiters();

  // Description:
  // Specify the character(s) that will be used to separate fields.  For
  // example, set this to "," for a comma-separated value file.  Set
  // it to ".:;" for a file where columns can be separated by a
  // period, colon or semicolon.  The order of the characters in the
  // string does not matter.  Defaults to a comma.
  vtkSetStringMacro(FieldDelimiterCharacters);
  vtkGetStringMacro(FieldDelimiterCharacters);

  void SetUTF8FieldDelimiters(const char* delimiters);
  const char* GetUTF8FieldDelimiters();
  void SetUnicodeFieldDelimiters(const vtkUnicodeString& delimiters);
  vtkUnicodeString GetUnicodeFieldDelimiters();

  // Description:
  // Get/set the character that will begin and end strings.  Microsoft
  // Excel, for example, will export the following format:
  //
  // "First Field","Second Field","Field, With, Commas","Fourth Field"
  //
  // The third field has a comma in it.  By using a string delimiter,
  // this will be correctly read.  The delimiter defaults to '"'.
  vtkGetMacro(StringDelimiter, char);
  vtkSetMacro(StringDelimiter, char);

  void SetUTF8StringDelimiters(const char* delimiters);
  const char* GetUTF8StringDelimiters();
  void SetUnicodeStringDelimiters(const vtkUnicodeString& delimiters);
  vtkUnicodeString GetUnicodeStringDelimiters();

  // Description:
  // Set/get whether to use the string delimiter.  Defaults to on.
  vtkSetMacro(UseStringDelimiter, bool);
  vtkGetMacro(UseStringDelimiter, bool);
  vtkBooleanMacro(UseStringDelimiter, bool);

  // Description:
  // Set/get whether to treat the first line of the file as headers.
  vtkGetMacro(HaveHeaders, bool);
  vtkSetMacro(HaveHeaders, bool);

  // Description:
  // Set/get whether to merge successive delimiters.  Use this if (for
  // example) your fields are separated by spaces but you don't know
  // exactly how many.
  vtkSetMacro(MergeConsecutiveDelimiters, bool);
  vtkGetMacro(MergeConsecutiveDelimiters, bool);
  vtkBooleanMacro(MergeConsecutiveDelimiters, bool);

  // Description:
  // Specifies the maximum number of records to read from the file.  Limiting the
  // number of records to read is useful for previewing the contents of a file.
  vtkGetMacro(MaxRecords, vtkIdType);
  vtkSetMacro(MaxRecords, vtkIdType);

  // Description:
  // When set to true, the reader will detect numeric columns and create
  // vtkDoubleArray or vtkIntArray for those instead of vtkStringArray. Default
  // is off.
  vtkSetMacro(DetectNumericColumns, bool);
  vtkGetMacro(DetectNumericColumns, bool);
  vtkBooleanMacro(DetectNumericColumns, bool);

  // Description:
  // The name of the array for generating or assigning pedigree ids
  // (default "id").
  vtkSetStringMacro(PedigreeIdArrayName);
  vtkGetStringMacro(PedigreeIdArrayName);

  // Description:
  // If on (default), generates pedigree ids automatically.
  // If off, assign one of the arrays to be the pedigree id.
  vtkSetMacro(GeneratePedigreeIds, bool);
  vtkGetMacro(GeneratePedigreeIds, bool);
  vtkBooleanMacro(GeneratePedigreeIds, bool);

  // Description:
  // If on, assigns pedigree ids to output. Defaults to off.
  vtkSetMacro(OutputPedigreeIds, bool);
  vtkGetMacro(OutputPedigreeIds, bool);
  vtkBooleanMacro(OutputPedigreeIds, bool);

  // Description:
  // Returns a human-readable description of the most recent error, if any.
  // Otherwise, returns an empty string.  Note that the result is only valid
  // after calling Update().
  vtkStdString GetLastError();

  // Description:
  // Fallback character for use in the US-ASCII-WITH-FALLBACK
  // character set.  Any characters that have their 8th bit set will
  // be replaced with this code point.  Defaults to 'x'.
  vtkSetMacro(ReplacementCharacter, vtkTypeUInt32);
  vtkGetMacro(ReplacementCharacter, vtkTypeUInt32);

//BTX
protected:
  vtkDelimitedTextReader();
  ~vtkDelimitedTextReader();

  int RequestData(
    vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*);

  char* FileName;
  char* UnicodeCharacterSet;
  vtkIdType MaxRecords;
  vtkUnicodeString UnicodeRecordDelimiters;
  vtkUnicodeString UnicodeFieldDelimiters;
  vtkUnicodeString UnicodeStringDelimiters;
  vtkUnicodeString UnicodeWhitespace;
  vtkUnicodeString UnicodeEscapeCharacter;
  bool DetectNumericColumns;
  char* FieldDelimiterCharacters;
  char StringDelimiter;
  bool UseStringDelimiter;
  bool HaveHeaders;
  bool UnicodeOutputArrays;
  bool MergeConsecutiveDelimiters;
  char* PedigreeIdArrayName;
  bool GeneratePedigreeIds;
  bool OutputPedigreeIds;
  vtkStdString LastError;
  vtkTypeUInt32 ReplacementCharacter;

private:
  vtkDelimitedTextReader(const vtkDelimitedTextReader&); // Not implemented
  void operator=(const vtkDelimitedTextReader&);   // Not implemented
//ETX
};

#endif

