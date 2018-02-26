/* ========================================================================== **
 *
 *                                  Packet.c
 *
 * Copyright:
 *  Copyright (C) 2002-2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: Packet.c,v 0.14 2010-11-21 18:43:14 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  Low-level NBT Name Service packet construction/destruction tools.
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
 *  A great deal of fussing went into the design of this module.  There are
 *  just too many good ways to do things.
 *
 *  This module doesn't use <cifs_Block> blocks.  Those are used at a higher
 *  level.  See "NBT/NS/Message.h".
 *
 * ========================================================================== **
 */

#include "NBT/NS/Packet.h"


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int nbt_nsSetHdr( uchar         *bufr,
                  const long     bSize,
                  const uint16_t flags,
                  const uint8_t  rmap )
  /* ------------------------------------------------------------------------ **
   * Build an NBT header from parts.
   *
   *  Input:  bufr  - Destination buffer.
   *          bSize - Size of destination buffer (in bytes).  Must have at
   *                  least <nbt_nsHEADER_LEN> bytes available.
   *          flags - NBT Header flags.  This value can be constructed by
   *                  'OR'ing flag field constants.   
   *          rmap  - Indicates which of the Query/Resource records will
   *                  follow the header.  Construct this parameter by 
   *                  'OR'ing a subset of:
   *                  nbt_nsQUERYREC = Query Record. 
   *                  nbt_nsANSREC   = Answer Record.
   *                  nbt_nsNSREC    = Name Server Record (never used).
   *                  nbt_nsADDREC   = Additional Record.
   *
   *  Output: On success: <nbt_nsHEADER_LEN>.
   *          On error, a negative value.
   *
   *  Errors: cifs_errBufrTooSmall - Indicates that the amount of storage
   *                                 available in <bufr> is not enough
   *                                 to fit an NBT name service header.
   *                                 (That is <bSize> is less than
   *                                 <nbt_nsHEADER_LEN>.)
   *
   *  Notes:  This function leaves the Transaction ID field untouched.
   *          All other header fields are overwritten.
   *
   *  See Also: <nbt_nsSetTID()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int      i;
  int      offset;
  uint16_t tmp;
  uint8_t  rmap_array[] =
    {
    nbt_nsQUERYREC,
    nbt_nsANSREC,
    nbt_nsNSREC,
    nbt_nsADDREC
    };

  /* Check boundaries. */
  if( bSize < nbt_nsHEADER_LEN )
    return( cifs_errBufrTooSmall );

  /* Cauterize the flags value, then write it to the flags field. */
  tmp = flags & nbt_nsHEADER_FLAGMASK;
  nbt_SetShort( bufr, 2, tmp );

  /* Set each count field to 0 or 1, depending on whether or not
   * the flag is set in the <rmap> parameter.  Note that we also
   * write zeros where appropriate, thus ensuring no garbage in
   * the header.  Probably just as fast as a bzero() or the like.
   */
  offset = 4;
  for( i = 0; i < 4; i++ )
    {
    tmp = (rmap & rmap_array[i]) ? 1 : 0;
    nbt_SetShort( bufr, offset, tmp );
    offset += 2;
    }

  return( nbt_nsHEADER_LEN );
  } /* nbt_nsSetHdr */


/* ========================================================================== */
