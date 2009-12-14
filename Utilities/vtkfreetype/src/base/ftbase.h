/***************************************************************************/
/*                                                                         */
/*  ftbase.h                                                               */
/*                                                                         */
/*    The FreeType private functions used in base module (specification).  */
/*                                                                         */
/*  Copyright 2008 by                                                      */
/*  David Turner, Robert Wilhelm, Werner Lemberg, and suzuki toshiya.      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTBASE_H__
#define __FTBASE_H__


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H


FT_BEGIN_HEADER


  /* Assume the stream is sfnt-wrapped PS Type1 or sfnt-wrapped CID-keyed */
  /* font, and try to load a face specified by the face_index.            */
  FT_LOCAL_DEF( FT_Error )
  open_face_PS_from_sfnt_stream( FT_Library     library,
                                 FT_Stream      stream,
                                 FT_Long        face_index,
                                 FT_Int         num_params,
                                 FT_Parameter  *params,
                                 FT_Face       *aface );


  /* Create a new FT_Face given a buffer and a driver name. */
  /* From ftmac.c.                                          */
  FT_LOCAL_DEF( FT_Error )
  open_face_from_buffer( FT_Library   library,
                         FT_Byte*     base,
                         FT_ULong     size,
                         FT_Long      face_index,
                         const char*  driver_name,
                         FT_Face     *aface );


FT_END_HEADER

#endif /* __FTBASE_H__ */


/* END */
