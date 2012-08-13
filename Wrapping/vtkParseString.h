/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParseString.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*
  String allocation routines used by vtkParse.

  The parser uses "const char *" as its string object type, and expects
  all string objects to persist and be constant for the entire lifetime
  of the data generated by the parse (usually this means until the parser
  executable has exited).  All strings that are stored in the parser's
  data objects should either be statically allocated, or allocated with
  the vtkParse_NewString() or vtkParse_CacheString() methods declared here.
*/

#ifndef VTK_PARSE_STRING_H
#define VTK_PARSE_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * StringCache provides a simple way of allocating strings centrally.
 * It eliminates the need to allocate and free each individual strings,
 * which makes the code simpler and more efficient.
 */
typedef struct _StringCache
{
  unsigned long  NumberOfChunks;
  char         **Chunks;
  size_t         ChunkSize;
  size_t         Position;
} StringCache;

/**
 * Initialize the string cache.
 */
void vtkParse_InitStringCache(StringCache *cache);

/**
 * Alocate a new string from the cache.
 * A total of n+1 bytes will be allocated, to leave room for null.
 */
char *vtkParse_NewString(StringCache *cache, size_t n);

/**
 * Cache a string so that it can then be used in the vtkParse data
 * structures.  The string will last until the application exits.
 * At most 'n' chars will be copied, and the string will be terminated.
 * If a null pointer is provided, then a null pointer will be returned.
 */
const char *vtkParse_CacheString(StringCache *cache, const char *cp, size_t n);

/**
 * Free all strings that were created with vtkParse_NewString() or
 * with vtkParse_CacheString().
 */
void vtkParse_FreeStringCache(StringCache *cache);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
