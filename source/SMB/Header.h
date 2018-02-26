#ifndef SMB_HEADER_H
#define SMB_HEADER_H
/* ========================================================================== **
 *                                  Header.h
 *
 * Copyright:
 *  Copyright (C) 2003,2004 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: Header.h,v 0.4 2004/05/30 19:24:23 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  A set of utilities for [de]composing SMB messages.
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

#include "smb_common.h"   /* SMB subsystem common include file. */


/* -------------------------------------------------------------------------- **
 * Defined Constants:
 *
 *  smb_HEADER_LEN      - Length of an NBT Session Service message header.
 *                        Always 4.
 */

#define smb_HEADER_LEN   32         /* SMB message header bytelen.  */

/*  --
 *  SMB Command Codes (SMB_COM_*, in the SNIA doc.)
 *  These should really be someplace else (like in per-command modules).
 */

#define SMB_COM_ECHO                0x2B
#define SMB_COM_NEGOTIATE           0x72
#define SMB_COM_SESSION_SETUP_ANDX  0x73

/*  --
 *  The Flags field.  See section 2.5.2 of "Implementing CIFS".
 *
 *  smb_hdrFLAGS_SERVER_TO_REDIR        - Reply flag.  0=request, 1=reply.
 *  smb_hdrFLAGS_REQUEST_BATCH_OPLOCK   - Batch OpLock flag.  (obsolete)
 *  smb_hdrFLAGS_REQUEST_OPLOCK         - Oplock flag.  (obsolete)
 *  smb_hdrFLAGS_CANONICAL_PATHNAMES    - 1=Path in DOS format.
 *  smb_hdrFLAGS_CASELESS_PATHNAMES     - 1=Case insensitive pathnames.
 *  smb_hdrFLAGS_CLIENT_BUF_AVAIL       - Zero for NBT and Naked TCP.
 *  smb_hdrFLAGS_SUPPORT_LOCKREAD       - Support for obsolete SMBs.
 *  smb_hdrFLAGS_MASK                   - Flags mask.
 */

#define smb_hdrFLAGS_SERVER_TO_REDIR        0x80
#define smb_hdrFLAGS_REQUEST_BATCH_OPLOCK   0x40
#define smb_hdrFLAGS_REQUEST_OPLOCK         0x20
#define smb_hdrFLAGS_CANONICAL_PATHNAMES    0x10
#define smb_hdrFLAGS_CASELESS_PATHNAMES     0x08
#define smb_hdrFLAGS_CLIENT_BUF_AVAIL       0x02
#define smb_hdrFLAGS_SUPPORT_LOCKREAD       0x01
#define smb_hdrFLAGS_MASK                   0xFB

/*  --
 *  The Flags2 field.  See section 2.5.2 of "Implementing CIFS".
 *
 *  smb_hdrFLAGS2_UNICODE_STRINGS       - Unicode support.
 *  smb_hdrFLAGS2_32BIT_STATUS          - NT_Status support.
 *  smb_hdrFLAGS2_READ_IF_EXECUTE       - Execute bit implies read privilage.
 *  smb_hdrFLAGS2_DFS_PATHNAME          - DFS support.
 *  smb_hdrFLAGS2_EXTENDED_SECURITY     - Extended Security support.
 *  smb_hdrFLAGS2_IS_LONG_NAME          - Message contains long names.
 *  smb_hdrFLAGS2_SECURITY_SIGNATURE    - Message contains MAC signature.
 *  smb_hdrFLAGS2_EAS                   - Extended Attribute support.
 *  smb_hdrFLAGS2_KNOWS_LONG_NAMES      - Client supports long names.
 *  smb_hdrFLAGS2_MASK                  - Flags2 mask.
 */

#define smb_hdrFLAGS2_UNICODE_STRINGS       0x8000
#define smb_hdrFLAGS2_32BIT_STATUS          0x4000
#define smb_hdrFLAGS2_READ_IF_EXECUTE       0x2000
#define smb_hdrFLAGS2_DFS_PATHNAME          0x1000
#define smb_hdrFLAGS2_EXTENDED_SECURITY     0x0800
#define smb_hdrFLAGS2_IS_LONG_NAME          0x0040
#define smb_hdrFLAGS2_SECURITY_SIGNATURE    0x0004
#define smb_hdrFLAGS2_EAS                   0x0002
#define smb_hdrFLAGS2_KNOWS_LONG_NAMES      0x0001
#define smb_hdrFLAGS2_MASK                  0xF847

/*  --
 *  The field offsets are used to indicate the absolute position of a field
 *  within the header.
 *
 *  smb_hdrOFFSET_CMD         - Location of the SMB Command Code field.
 *  smb_hdrOFFSET_NTSTATUS    - NT_Status code field.
 *  smb_hdrOFFSET_ECLASS      - DOS Error class (overlays NT_Status).
 *  smb_hdrOFFSET_ECODE       - DOS Error code (likewise).
 *  smb_hdrOFFSET_FLAGS       - Location of the Flags field.
 *  smb_hdrOFFSET_FLAGS2      - Location of the Flags2 field.
 *  smb_hdrOFFSET_EXTRA       - The "Extra" block (MAC signatures, etc.).
 *  smb_hdrOFFSET_TID         - Tree ID.
 *  smb_hdrOFFSET_PID         - Process ID.
 *  smb_hdrOFFSET_UID         - User ID.
 *  smb_hdrOFFSET_MID         - Multiplex ID.
 *
 */

#define smb_hdrOFFSET_CMD        4
#define smb_hdrOFFSET_NTSTATUS   5
#define smb_hdrOFFSET_ECLASS     5
#define smb_hdrOFFSET_ECODE      7
#define smb_hdrOFFSET_FLAGS      9
#define smb_hdrOFFSET_FLAGS2    10
#define smb_hdrOFFSET_EXTRA     12
#define smb_hdrOFFSET_TID       24
#define smb_hdrOFFSET_PID       26
#define smb_hdrOFFSET_UID       28
#define smb_hdrOFFSET_MID       30


/* -------------------------------------------------------------------------- **
 * Macros:
 *
 * These macros may be used to write data to specific locations within an
 * SMB header block (indicated by <bufr> in the macro definitions).
 *
 *  smb_hdrSetCmd( bufr, cmd )
 *    Input:  bufr  - (uchar *) Pointer to an SMB header block.
 *            cmd   - (uchar) SMB_COM_* command code.
 *    Notes:  Writes a command byte to the header buffer.
 *
 *  smb_hdrGetCmd( bufr )
 *    Input:  bufr  - (uchar *) Pointer to an SMB header block.
 *    Output: (uchar) SMB_COM_* command code.
 *    Notes:  Reads a command byte from an SMB header.
 */

#define smb_hdrSetCmd( bufr, cmd )  (bufr)[smb_hdrOFFSET_CMD] = (cmd)
#define smb_hdrGetCmd( bufr )  (uchar)((bufr)[smb_hdrOFFSET_CMD])

/*  --
 *  smb_hdrSetEclassDOS( bufr, Eclass )
 *    Input:  bufr    - (uchar *) Pointer to an SMB header block.
 *            Eclass  - (uchar) A DOS error class.
 *    Notes:  Writes a DOS Error Class to the header buffer.
 *
 *  smb_hdrGetEclassDOS( bufr )
 *    Input:  bufr  - (uchar *) Pointer to an SMB header block.
 *    Output: (uchar) The DOS Error Class stored in the header.
 *    Notes:  Read a DOS Error Class; returns uchar.
 */

#define smb_hdrSetEclassDOS( bufr, Eclass ) \
          (bufr)[smb_hdrOFFSET_ECLASS] = (Eclass)
#define smb_hdrGetEclassDOS( bufr )  (uchar)((bufr)[smb_hdrOFFSET_ECLASS])

/*  --
 *  smb_hdrSetEcodeDOS( bufr, Ecode )
 *    Input:  bufr  - (uchar *) Pointer to an SMB header block.
 *            Ecode - (uint16_t) A DOS error code.
 *    Notes:  Writes a DOS Error Code to the header buffer.
 *
 *  smb_hdrGetEcodeDOS( bufr )
 *    Input:  bufr  - (uchar *) Pointer to an SMB header block.
 *    Output: (uint16_t) A DOS error code.
 *    Notes:  Reads the DOS Error Code from the header.
 */

#define smb_hdrSetEcodeDOS( bufr, Ecode ) \
          smb_SetShort( bufr, smb_hdrOFFSET_ECODE, Ecode )
#define smb_hdrGetEcodeDOS( bufr )  smb_GetShort( bufr, smb_hdrOFFSET_ECODE )

#define smb_hdrSetNTStatus( bufr, nt_status ) \
          smb_PutLong( bufr, smb_hdrOFFSET_NTSTATUS, nt_status )
#define smb_hdrGetNTStatus( bufr )  smb_GetLong( bufr, smb_hdrOFFSET_NTSTATUS )

#define smb_hdrSetFlags( bufr, flags )  (bufr)[smb_hdrOFFSET_FLAGS] = (flags)
#define smb_hdrGetFlags( bufr )  (uchar)((bufr)[smb_hdrOFFSET_FLAGS])

#define smb_hdrSetFlags2( bufr, flags2 ) \
          smb_SetShort( bufr, smb_hdrOFFSET_FLAGS2, flags2 )
#define smb_hdrGetFlags2( bufr )  smb_GetShort( bufr, smb_hdrOFFSET_FLAGS2 )

#define smb_hdrSetTID( bufr, TID )  smb_SetShort( bufr, smb_hdrOFFSET_TID, TID )
#define smb_hdrGetTID( bufr )  smb_GetShort( bufr, smb_hdrOFFSET_TID )

#define smb_hdrSetPID( bufr, PID )  smb_SetShort( bufr, smb_hdrOFFSET_PID, PID )
#define smb_hdrGetPID( bufr )  smb_GetShort( bufr, smb_hdrOFFSET_PID )

#define smb_hdrSetUID( bufr, UID )  smb_SetShort( bufr, smb_hdrOFFSET_UID, UID )
#define smb_hdrGetUID( bufr )  smb_GetShort( bufr, smb_hdrOFFSET_UID )

#define smb_hdrSetMID( bufr, MID )  smb_SetShort( bufr, smb_hdrOFFSET_MID, MID )
#define smb_hdrGetMID( bufr ) smb_GetShort( bufr, smb_hdrOFFSET_MID )


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int smb_hdrInit( uchar *bufr, int bsize );
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


int smb_hdrCheck( uchar *bufr, int bsize );
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


/* ========================================================================== */
#endif /* SMB_HEADER_H */
