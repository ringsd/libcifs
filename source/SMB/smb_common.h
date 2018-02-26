#ifndef SMB_COMMON_H
#define SMB_COMMON_H
/* ========================================================================== **
 *
 *                                smb_common.h
 *
 * Copyright:
 *  Copyright (C) 2001,2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: smb_common.h,v 0.5 2004/05/30 19:24:23 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  General include file for the Server Message Block (SMB) subsystem.
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
 *  This is the general-purpose header for the SMB subsystem of the ubiqx
 *  CIFS toolkit.
 *
 * ========================================================================== **
 */

#include "cifs_common.h"


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
 *      val = smb_GetLong( GetNextPacket( param1, param2 ), 12 );
 *
 *    Would call the GetNextPacket() function four times!  Don't let this
 *    happen to you!  Instead of the above, do this:
 *
 *      uchar *src = GetNextPacket( param1, param2 );
 *      val = smb_GetLong( src, 12 );
 *
 *    Got it?
 *
 *  smb_GetShort( src, offset )
 *      - Read two bytes, starting at src[offset], returning a uint16_t.
 *        Input:  src     - Pointer of type (uchar *) to the data source.
 *                offset  - Integer offset within src[] of the first byte
 *                          to read.
 *        Output: An unsigned short integer in host byte order equal to
 *                the two byte value read (in network byte order) from <src>.
 *        Notes:  Similar to ntohs(3), but it is written as a macro and does
 *                not care about the alignment of the source.
 *
 *  smb_SetShort( dst, offset, val )
 *      - Write two bytes in network order to dst[offset].
 *        Input:  dst     - Pointer of type (uchar *) to the destination.
 *                offset  - Integer offset value, indicating the starting
 *                          point within dst[] to start writing.
 *                val     - Read as an unsigned short, this is the value
 *                          that will be written to dst[offset].
 *
 *  smb_GetLong( src, offset )
 *      - Read four bytes, starting at src[offset], returning a uint32_t.
 *        Input:  src     - Pointer of type (uchar *) to the data source.
 *                offset  - Integer offset within src[] of the first byte
 *                          to read.
 *        Output: An unsigned long integer in host byte order equal to
 *                the four byte value read (in network byte order) from <src>.
 *        Notes:  Similar to ntohl(3), but it is written as a macro and does
 *                not care about the alignment of the source.
 *
 *  smb_SetLong( dst, offset, val )
 *      - Write four bytes in network order to dst[offset].
 *        Input:  dst     - Pointer of type (uchar *) to the destination.
 *                offset  - Integer offset value, indicating the starting
 *                          point within dst[] to start writing.
 *                val     - Read as an unsigned long, this is the value
 *                          that will be written to dst[offset].
 */

#define smb_GetShort( src, offset )  \
  ( \
  (uint16_t)(((uchar *)(src))[offset]) | \
  ( ((uint16_t)(((uchar *)(src))[(offset)+1])) << 8 ) \
  )

#define smb_SetShort( dst, offset, val ) \
  (void)( \
  (((uchar *)(dst))[offset]     = (uchar)(((uint16_t)(val)) & 0xFF)) ^ \
  (((uchar *)(dst))[(offset)+1] = (uchar)((((uint16_t)(val)) >> 8) & 0xFF)) \
  )

#define smb_GetLong( src, offset )  \
  ( \
  (uint32_t)(((uchar *)(src))[offset]) | \
  ((uint32_t)(((uchar *)(src))[(offset)+1]) <<  8) | \
  ((uint32_t)(((uchar *)(src))[(offset)+2]) << 16) | \
  ((uint32_t)(((uchar *)(src))[(offset)+3]) << 24) \
  )

#define smb_SetLong( dst, offset, val ) \
  (void)( \
  (((uchar *)(dst))[offset]     = (uchar)(((uint32_t)(val)) & 0xFF))         ^ \
  (((uchar *)(dst))[(offset)+1] = (uchar)((((uint32_t)(val)) >>  8) & 0xFF)) ^ \
  (((uchar *)(dst))[(offset)+2] = (uchar)((((uint32_t)(val)) >> 16) & 0xFF)) ^ \
  (((uchar *)(dst))[(offset)+3] = (uchar)((((uint32_t)(val)) >> 24) & 0xFF)) \
  )

/* ========================================================================== */
#endif /* SMB_COMMON_H */
