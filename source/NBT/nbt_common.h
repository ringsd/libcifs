#ifndef NBT_COMMON_H
#define NBT_COMMON_H
/* ========================================================================== **
 *
 *                                nbt_common.h
 *
 * Copyright:
 *  Copyright (C) 2001,2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: nbt_common.h,v 0.10 2004/05/30 04:19:50 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  General utility include file for the NetBIOS over TCP/IP (NBT) subsystem.
 *  Part of the ubiqx "Implementing CIFS" library and toolkit.
 *
 * -------------------------------------------------------------------------- **
 *
 * License:
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public   
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of   
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public   
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * -------------------------------------------------------------------------- **
 *
 * Notes:
 *
 *  Do *not* include this file in application code.  Instead:
 *
 *  #include "NBT/nbt.h"
 *
 *  The *_common.h include files contain bits and pieces required by modules
 *  at the same level and below in the toolkit directory hierarchy.
 *
 *  This module is specific to the NBT subsystem of the ubiqx CIFS library.
 *
 *  NBT uses network byte order.
 *  Byte-order functions such as htonl(3) convert the byte order of a
 *  integer value, returning an integer value.  These values must be
 *  read/written from/to the packet buffers in a separate step.
 *
 *  The byte-order macros in this module provide a short-cut.  They
 *  read/write directly from/to the data buffers.  Also, since they are
 *  macros, they are operate in-line, avoiding all of that messing about
 *  on the stack when a function is called.
 *
 * ========================================================================== **
 */

#include "cifs_common.h"    /* CIFS library common include file. */


/* -------------------------------------------------------------------------- **
 * Macros:
 *
 *  The following macros are all a bit kludgey.  It is important to be
 *  careful when using them.
 *  + Make sure that the input data is of the correct type.
 *  + Make sure that you won't overrun your buffers.
 *  + Evaluate expressions before passing them to the macro, or problems
 *    may occur.  For example, the call:
 *
 *      val = nbt_GetLong( GetNextPacket( param1, param2 ), 12 );
 *
 *    Would call the GetNextPacket() function four times!  Don't let this
 *    happen to you!  Instead of the above, do this:
 *
 *      uchar *src = GetNextPacket( param1, param2 );
 *      val = nbt_GetLong( src, 12 );
 *
 *    Got it?
 *
 *  nbt_GetShort( src, offset )
 *      - Read two bytes, starting at src[offset], returning a uint16_t.
 *        Input:  src     - Pointer of type (uchar *) to the data source.
 *                offset  - Integer offset within src[] of the first byte
 *                          to read.
 *        Output: An unsigned short integer in host byte order equal to
 *                the two byte value read (in network byte order) from <src>.
 *        Notes:  Similar to ntohs(3), but it is written as a macro and does
 *                not care about the alignment of the source.
 *
 *  nbt_SetShort( dst, offset, val )
 *      - Write two bytes in network order to dst[offset].
 *        Input:  dst     - Pointer of type (uchar *) to the destination.
 *                offset  - Integer offset value, indicating the starting
 *                          point within dst[] to start writing.
 *                val     - Read as an unsigned short, this is the value
 *                          that will be written to dst[offset].
 *
 * nbt_GetLong( src, offset )
 *      - Read four bytes, starting at src[offset], returning a uint32_t.
 *        Input:  src     - Pointer of type (uchar *) to the data source.
 *                offset  - Integer offset within src[] of the first byte
 *                          to read.
 *        Output: An unsigned long integer in host byte order equal to
 *                the four byte value read (in network byte order) from <src>.
 *        Notes:  Similar to ntohl(3), but it is written as a macro and does
 *                not care about the alignment of the source.
 *
 *  nbt_SetLong( dst, offset, val )
 *      - Write four bytes in network order to dst[offset].
 *        Input:  dst     - Pointer of type (uchar *) to the destination.
 *                offset  - Integer offset value, indicating the starting
 *                          point within dst[] to start writing.
 *                val     - Read as an unsigned long, this is the value
 *                          that will be written to dst[offset].
 */

#define nbt_GetShort( src, offset )  \
  ( \
  ((uint16_t)(((uchar *)(src))[offset]) << 8) | \
  ((uint16_t)(((uchar *)(src))[(offset)+1])) \
  )

#define nbt_SetShort( dst, offset, val ) \
  (void)( \
  (((uchar *)(dst))[offset] = (uchar)((((uint16_t)(val)) >> 8) & 0xFF)) ^ \
  ((((uchar *)(dst))[(offset)+1]) = (uchar)(((uint16_t)(val)) & 0xFF)) \
  )

#define nbt_GetLong( src, offset )  \
  ( \
  ((uint32_t)(((uchar *)(src))[offset])     << 24) | \
  ((uint32_t)(((uchar *)(src))[(offset)+1]) << 16) | \
  ((uint32_t)(((uchar *)(src))[(offset)+2]) <<  8) | \
  ((uint32_t)(((uchar *)(src))[(offset)+3])) \
  )

#define nbt_SetLong( dst, offset, val ) \
  (void)( \
  (((uchar *)(dst))[offset]     = (uchar)((((uint32_t)(val)) >> 24) & 0xFF)) ^ \
  (((uchar *)(dst))[(offset)+1] = (uchar)((((uint32_t)(val)) >> 16) & 0xFF)) ^ \
  (((uchar *)(dst))[(offset)+2] = (uchar)((((uint32_t)(val)) >>  8) & 0xFF)) ^ \
  ((((uchar *)(dst))[(offset)+3]) = (uchar)(((uint32_t)(val)) & 0xFF)) \
  )


/* ========================================================================== */
#endif /* NBT_COMMON_H */
