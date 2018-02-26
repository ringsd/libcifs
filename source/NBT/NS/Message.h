#ifndef NBT_NS_MESSAGE_H
#define NBT_NS_MESSAGE_H
/* ========================================================================== **
 *
 *                                 Message.h
 *
 * Copyright:
 *  Copyright (C) 2002-2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: Message.h,v 0.10 2004/05/30 06:22:20 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  NBT Name Service Message construction/destruction tools.
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
 *  As with the Packet module, a great deal of fussing went into this.
 *
 *  The <nbt_nsMsgBlock> structure is a descendent of the <cifs_Block> type.
 *
 * ========================================================================== **
 */

#include "NBT/nbt_common.h"   /* NBT subsystem common include file. */


/* -------------------------------------------------------------------------- **
 * Typedefs:
 */

typedef enum
  {
  nbt_nsNAME_QUERY_REQST = 1,
  nbt_nsNAME_QUERY_REPLY_POS,
  nbt_nsNAME_QUERY_REPLY_NEG,
  nbt_nsNODE_STATUS_REQST,
  nbt_nsNODE_STATUS_REPLY,
  nbt_nsNAME_REG_REQST,
  nbt_nsNAME_OVERWRITE_DEMAND,
  nbt_nsNAME_REG_REPLY_POS,
  nbt_nsNAME_REG_REPLY_NEG,
  nbt_nsNAME_CONFLICT_DEMAND,
  nbt_nsNAME_RELEASE_REQST,
  nbt_nsNAME_RELEASE_REPLY_POS,
  nbt_nsNAME_RELEASE_REPLY_NEG,
  nbt_nsWACK_REPLY,
  nbt_nsNAME_REFRESH_REQST,
  nbt_nsMULTI_REG_REQST
  } nbt_nsMsgType;


typedef struct
  {
  cifs_Block    block;
  nbt_nsMsgType type;
  uint16_t      tid;
  uint16_t      flags;
  uint8_t       rmap;
  uchar        *QR_name;
  uint8_t       QR_name_len;
  uint16_t      QR_type;
  uchar        *RR_name;
  uint8_t       RR_name_len;
  uint16_t      RR_type;
  uint32_t      ttl;
  uchar        *rdata;
  uint16_t      rdata_len;
  } nbt_nsMsgBlock;


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int nbt_nsParseMsg( nbt_nsMsgBlock *msg );
  /* ------------------------------------------------------------------------ **
   * Carve up the contents of a raw NBT message buffer.
   *
   *  Input:  msg - A pointer to an nbt_nsMsgBlock structure:
   *                msg->block.bufr - Points to the data block being parsed.
   *                msg->block.size - Total allocated size of the data block.
   *                msg->block.used - Size of the message being parsed
   *                                  (number of bytes actually used).
   *
   *  Output: A positive value from the nbt_nsMsgType enum list, or a
   *          negative error code.
   *
   *  Errors: cifs_errInvalidLblLen   - While checking an NBT Name, the
   *                                    starting label was found to be
   *                                    invalid.  The first label should
   *                                    always be 32 bytes in length.
   *          cifs_errBadLblFlag      - A non-zero label flag was found.
   *                                    (Could be a label string pointer.)
   *          cifs_errOutOfBounds     - Initial <srcpos> is beyond <srcmax>.
   *          cifs_errTruncatedBufr   - Ran out of buffer before we found all
   *                                    of the data.
   *          cifs_errNameTooLong     - NBT name is greater than 255 bytes.
   *          cifs_errNullInput       - NULL buffer.  Cannot parse.
   *          cifs_errInvalidPacket   - Conflicting data discovered within
   *                                    the packet during parsing.
   *          cifs_errUnknownCommand  - Unknown Header.OpCode.
   *
   *  Notes:  Rule of thumb: ( msg->block.used <= msg->block.size ).
   *
   * ------------------------------------------------------------------------ **
   */


/* ========================================================================== */
#endif /* NBT_NS_MESSAGE_H */
