/* ========================================================================== **
 *                                  Header.c
 *
 * Copyright:
 *  Copyright (C) 2003,2004 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: Header.c,v 0.3 2004/05/30 19:24:23 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  A set of utilities for [de]composing SMB headers.
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
 * ========================================================================== **
 */

#include "SMB/Header.h"         /* Module header.                          */


/* -------------------------------------------------------------------------- **
 * Constants:
 *
 *  smb_hdrSMBString  - Used internally only, this is the standard SMB
 *                      prefix string.
 */

const uchar *smb_hdrSMBString = "\xFFSMB";


/* -------------------------------------------------------------------------- **
 * Functions:
 */


int smb_hdrInit( uchar *bufr, int bsize )
  /* ------------------------------------------------------------------------ **
   * Initialize an empty header structure.
   *
   *  Input:  bufr  - pointer to the buffer to be initialized as an SMB
   *                  header.
   *          bsize - Size of <bufr>, so that we can be sure it's big
   *                  enough to be an SMB header.
   *
   *  Output: Returns -1 on error, the SMB header size on success.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i;

  if( bsize < smb_HEADER_LEN )
    return( -1 );

  for( i = 0; i < 4; i++ )
    bufr[i] = smb_hdrSMBString[i];
  for( i = 4; i < smb_HEADER_LEN; i++ )
    bufr[i] = '\0';

  return( smb_HEADER_LEN );
  } /* smb_hdrInit */


int smb_hdrCheck( uchar *bufr, int bsize )
  /* ------------------------------------------------------------------------ **
   * Validate an SMB header.
   *
   *  Input:  bufr  - pointer to a block of bytes that may be an SMB header.
   *          bsize - number of bytes in <bufr>.
   *
   *  Output: A negative number if the header is malformed,
   *          else the normal SMB header size (a positive value).
   *
   *  Errors: cifs_errNullInput     - the input <bufr> pointer is NULL.
   *          cifs_errBufrTooSmall  - the input <bufr> is too small.
   *          cifs_errInvalidPacket - The content does not begin with the
   *                                  standard SMB prefix ("\xffSMB").
   *
   *  Notes:  This function performs some quick, simple checks on a received
   *          buffer to make sure it's safe to read.  This function returns
   *          a negative value if the SMB header is invalid.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i;

  if( NULL == bufr )
    return( cifs_errNullInput );

  if( bsize < smb_HEADER_LEN )
    return( cifs_errBufrTooSmall );

  for( i = 0; i < 4; i++ )
    if( bufr[i] != smb_hdrSMBString[i] )
      return( cifs_errInvalidPacket );

  return( smb_HEADER_LEN );
  } /* smb_hdrCheck */


/* ========================================================================== */
