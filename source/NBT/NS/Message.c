/* ========================================================================== **
 *
 *                                 Message.c
 *
 * Copyright:
 *  Copyright (C) 2002-2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: Message.c,v 0.12 2004/10/06 04:46:25 crh Exp $
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

#include <string.h>             /* strlen(3) */

#include "NBT/Names.h"          /* Name management module. */
#include "NBT/NS/Packet.h"      /* Packet parsing module.  */
#include "NBT/NS/Message.h"     /* Message module header.  */


/* -------------------------------------------------------------------------- **
 * Static Functions:
 */

static int ParseQR( nbt_nsMsgBlock *msg, int offset )
  /* ------------------------------------------------------------------------ **
   * Rip apart a Question Record.
   *
   *  Input:  msg     - Pointer to an nbt_nsMsgBlock structure.
   *          offset  - Location, within the raw buffer, of the Question
   *                    Record to be parsed.
   *
   *  Output: The offset of the next byte beyond the question record,
   *          or a negative number indicating an error.
   *
   *  Errors: cifs_errInvalidLblLen - The starting label was invalid.  The
   *                                  first label should always be 32 bytes
   *                                  in length.
   *          cifs_errBadLblFlag    - A non-zero label flag was found.
   *                                  (Could be a label string pointer.)
   *          cifs_errOutOfBounds   - Initial <srcpos> is beyond <srcmax>.
   *          cifs_errTruncatedBufr - Ran out of buffer before we found all
   *                                  of the data.
   *          cifs_errNameTooLong   - NBT name is greater than 255 bytes.
   *
   *  Notes:  This function will return the cifs_errBadLblFlag error even
   *          if there is a label string pointer in the name field.  This
   *          is because RFC1001/1002 do not allow for label LSP's in the
   *          Question Name field.
   *
   *          It's almost silly to pass <offset> since question records
   *          always follow the header and the <offset> is always equal to
   *          nbt_nsHEADER_LEN.  On the other hand, passing <offset> here
   *          makes the interface the same as <ParseRR()>'s interface, which
   *          satisfies my sense of symmetry.  [Sorry, Juan.  I'm wierd that
   *          way.]
   *
   * ------------------------------------------------------------------------ **
   */
  {
  uchar *bufr = msg->block.bufr;
  int    result;

  /* Find the name and its length.
   */
  result = nbt_CheckL2Name( bufr, offset, msg->block.used );
  if( result < 0 )
    return( result );
  msg->QR_name     = &bufr[offset];
  msg->QR_name_len =  result;
  offset += result;

  /* Make sure that the buffer has enough room to hold the Type and Class.
   */
  if( msg->block.used < (offset+4) )
    return( cifs_errTruncatedBufr );

  /* Read the Type.
   * We don't bother reading the Class since it's always 0x0001 (IN).
   */
  msg->QR_type = nbt_GetShort( bufr, offset );

  /* Add four bytes; two for QUESTION_TYPE and two for QUESTION_CLASS.  
   */
  return( offset + 4 );
  } /* ParseQR */


static int ParseRR( nbt_nsMsgBlock *msg, int offset )
  /* ------------------------------------------------------------------------ **
   * Rip apart a Resource Record.
   *
   *  Input:  msg     - Pointer to an nbt_nsMsgBlock structure.
   *          offset  - Location, within the raw buffer, of the Resource
   *                    Record to be parsed.
   *
   *  Output: The offset of the RDATA section, or a negative number
   *          indicating an error.
   *
   *  Errors: cifs_errInvalidLblLen - The starting label was invalid.  The
   *                                  first label should always be 32 bytes
   *                                  in length.
   *          cifs_errBadLblFlag    - A non-zero label flag was found.
   *                                  (Could be a label string pointer.)
   *          cifs_errOutOfBounds   - Initial <srcpos> is beyond <srcmax>.
   *          cifs_errTruncatedBufr - Ran out of buffer before we found all
   *                                  of the data.
   *          cifs_errNameTooLong   - NBT name is greater than 255 bytes.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  uchar *bufr = msg->block.bufr;
  int    result;

  /* Find the name and its length.
   */
  result = nbt_CheckL2Name( bufr, offset, msg->block.used );
  if( cifs_errBadLblFlag == result)
    {
    /* Label String Pointers are okay in RR_names *if* they're
     * correct LSPs and *if* they point to the right place.
     * We return an error if the QR_name is NULL or if the
     * LSP is incorrect.
     */
    if( (NULL == msg->QR_name) || (nbt_nsLSP != nbt_GetShort( bufr, offset )) )
      return( cifs_errBadLblFlag );
    msg->RR_name     = msg->QR_name;
    msg->RR_name_len = msg->QR_name_len;
    offset += 2;
    }
  else
    {
    /* We're not dealing with an LSP (correct or otherwise).
     * If we got an error in the name check, return that result.
     * Otherwise, we've got a valid L2 name.
     */
    if( result < 0 )
      return( result );
    msg->RR_name     = &bufr[offset];
    msg->RR_name_len = result;
    offset += result;
    }

  /* Make sure that we have room for the
   * Type[2], Class[2], TTL[4], and RDLENGTH[2]
   * before we try to read them.  2+2+4+2=10
   */
  if( msg->block.used < (offset+10) )
    return( cifs_errTruncatedBufr );
  msg->RR_type  = nbt_GetShort( bufr, offset );
  offset += 4;  /* Skip Type *and* Class */
  msg->ttl      = nbt_GetLong(  bufr, offset );
  offset += 4;  /* Skip TTL */
  msg->rdata_len = nbt_GetShort( bufr, offset );
  offset += 2;  /* Skip RDLENGTH */

  /* Now check that there are RDLENGTH bytes in the rest of the buffer.
   */
  if( msg->block.used < (offset+(msg->rdata_len)) )
    return( cifs_errTruncatedBufr );

  /* Point to the RDATA, and return the offset.
   */
  msg->rdata = &bufr[offset];

  return( offset );
  } /* ParseRR */


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int nbt_nsParseMsg( nbt_nsMsgBlock *msg )
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
  {
  uchar *bufr;
  int    opcode;
  int    result;

  if( NULL == msg || NULL == msg->block.bufr )
    return( cifs_errNullInput );
  bufr = msg->block.bufr;   /* shorthand */

  if( msg->block.size <= nbt_nsHEADER_LEN )
    return( cifs_errTruncatedBufr );

  msg->tid   = nbt_nsGetTID( bufr );
  msg->flags = nbt_nsGetFlags( bufr );
  msg->rmap  = nbt_nsGetRmap( bufr );

  opcode = ( msg->flags & nbt_nsOPCODE_MASK );

  if( 0 == (msg->flags & nbt_nsR_BIT) )   /* Requests */
    {
    switch( opcode )
      {
      case nbt_nsOPCODE_QUERY:
        {
        /* Name and Status query Requests.
         * These have only a Query Record, no Resource Record.
         */
        result = ParseQR( msg, nbt_nsHEADER_LEN );
        if( result < 0 )
          return( result );

        /* Clear out the RR portion. */
        msg->RR_name     = NULL;
        msg->RR_name_len = 0;
        msg->rdata_len   = 0;

        /* Determine the query type. */
        if( nbt_nsQTYPE_NBSTAT == msg->QR_type )
          msg->type = nbt_nsNODE_STATUS_REQST;
        else
          msg->type = nbt_nsNAME_QUERY_REQST;
        return( msg->type );
        }

      case nbt_nsOPCODE_REGISTER:
      case nbt_nsOPCODE_REFRESH:
      case nbt_nsOPCODE_ALTREFRESH:
      case nbt_nsOPCODE_MULTIHOMED:
      case nbt_nsOPCODE_RELEASE:
        {
        /* Registration and Release requests.
         * These have both a Query and Resource Record.
         */
        result = ParseQR( msg, nbt_nsHEADER_LEN );
        if( result < 0 )
          return( result );
        result = ParseRR( msg, result );
        if( result < 0 )
          return( result );

        /* Determine which type of request we're seeing. */
        switch( opcode )
          {
          case nbt_nsOPCODE_REGISTER:
            if( msg->flags & nbt_nsRD_BIT )
              msg->type = nbt_nsNAME_OVERWRITE_DEMAND;
            else
              msg->type = nbt_nsNAME_REG_REQST;
            break;
          case nbt_nsOPCODE_REFRESH:
          case nbt_nsOPCODE_ALTREFRESH:
            msg->type = nbt_nsNAME_REFRESH_REQST;
            break;
          case nbt_nsOPCODE_MULTIHOMED:
            msg->type = nbt_nsMULTI_REG_REQST;
            break;
          case nbt_nsOPCODE_RELEASE:
            msg->type = nbt_nsNAME_RELEASE_REQST;
            break;
          }
        return( msg->type );
        }

      case nbt_nsOPCODE_WACK:
        /* WACKs are only supposed to be sent as replies. */
        return( cifs_errInvalidPacket );
      }

    /* If we fall through to this point
     * then we haven't recognized the command.
     */
    return( cifs_errUnknownCommand );
    }

  else  /* Replies */
    {
    /* Quickly check for valid response opcodes.
     */
    switch( opcode )
      {
      case nbt_nsOPCODE_QUERY:
      case nbt_nsOPCODE_REGISTER:
      case nbt_nsOPCODE_RELEASE:
      case nbt_nsOPCODE_WACK:
        break;
      case nbt_nsOPCODE_REFRESH:
      case nbt_nsOPCODE_ALTREFRESH:
      case nbt_nsOPCODE_MULTIHOMED:
        return( cifs_errInvalidPacket );
      default:
        return( cifs_errUnknownCommand );
      }

    /* Responses, do not contain query records,
     * just resource records.  We clear out the QR_name
     * and QR_name_len fields to indicate that these are
     * (intentionally) blank.  See ParseRR(), above, for
     * more notes on this.
     */
    msg->QR_name     = NULL;
    msg->QR_name_len = 0;
    result = ParseRR( msg, nbt_nsHEADER_LEN );
    if( result < 0 )
      return( result );

    /* Okay, now we just need to make sure we know
     * which kind of response we've received.
     */
    switch( opcode )
      {
      case nbt_nsOPCODE_QUERY:
        {
        /* Query Response.
         * Node Status, Negative Name Query, Positive Name Query.
         */
        if( nbt_nsQTYPE_NBSTAT == msg->RR_type )
          msg->type = nbt_nsNODE_STATUS_REPLY;
        else
          if( msg->flags & nbt_nsRCODE_MASK )
            msg->type = nbt_nsNAME_QUERY_REPLY_NEG;
          else
            msg->type = nbt_nsNAME_QUERY_REPLY_POS;
        return( msg->type );
        }

      case nbt_nsOPCODE_REGISTER:
        {
        /* Registraton Response.
         * Name Conflict Demand, Negative Registration Reply,
         * Positive Registration Reply.
         */
        switch( msg->flags & nbt_nsRCODE_MASK )
          {
          case nbt_nsRCODE_POS_RSP:
            msg->type = nbt_nsNAME_REG_REPLY_POS;
            break;
          case nbt_nsRCODE_CFT_ERR:
            msg->type = nbt_nsNAME_CONFLICT_DEMAND;
            break;
          default:
            msg->type = nbt_nsNAME_REG_REPLY_NEG;
            break;
          }
        return( msg->type );
        }

      case nbt_nsOPCODE_RELEASE:
        {
        /* Release Response.
         * Positive/Negative Name Release.
         */
        if( msg->flags & nbt_nsRCODE_MASK )
          msg->type = nbt_nsNAME_RELEASE_REPLY_NEG;
        else
          msg->type = nbt_nsNAME_RELEASE_REPLY_POS;
        return( msg->type );
        }

      case nbt_nsOPCODE_WACK:
        {
        /* WACK.
         */
        result = ParseRR( msg, nbt_nsHEADER_LEN );
        if( result < 0 )
          return( result );
        return( msg->type = nbt_nsWACK_REPLY );
        }
      }
    }

  /* Should never reach here.
   */
  return( cifs_errUnknownCommand );
  } /* nbt_nsParseMsg */


int nbt_nsRegRequest( nbt_nsMsgBlock *msg )
  /* ------------------------------------------------------------------------ **
   * Build an NBT Name Registration Request message from parts.
   *
   *  Input:  msg - Pointer to an nbt_nsMsgBlock structure.
   *
   *  Output: 

need: name, bcast, TTL (for unicast), group, ONT, NB_Address

   * ------------------------------------------------------------------------ **
   */
  {
  return( 0 );
  } /* nbt_nsRegRequest */


int nbt_nsRefRequest( nbt_nsMsgBlock *msg )
  /* ------------------------------------------------------------------------ **
   * Build an NBT Name Refresh Request message from parts.
   *

need: name, TTL (for unicast), group, ONT, NB_Address

   * ------------------------------------------------------------------------ **
   */
  {
  return( 0 );
  } /* nbt_nsRegRequest */


/* ========================================================================== */
