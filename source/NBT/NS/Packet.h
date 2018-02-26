#ifndef NBT_NS_PACKET_H
#define NBT_NS_PACKET_H
/* ========================================================================== **
 *
 *                                  Packet.h
 *
 * Copyright:
 *  Copyright (C) 2002-2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: Packet.h,v 0.15 2010-11-21 18:43:14 crh Exp $
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

#include "NBT/nbt_common.h"   /* NBT subsystem common include file. */


/* -------------------------------------------------------------------------- **
 * Defines:
 *
 *  nbt_nsHEADER_LEN  - Byte length of a NBT Name Service message header.
 *                      Always 12.
 */

#define nbt_nsHEADER_LEN 12             /* NBT Name Service Header bytelen. */

/*  --
 *  This next batch of defines represents sub-fields, or combinations of
 *  sub-fields, within the header flags field.  The values are relative
 *  to the 16-bit flags field itself, not to the subfields.  For example,
 *  RFC 1002 gives the OPCODE values as 0x00, 0x05, 0x06, 0x07, and 0x08.
 *  The above values are relative to the OPCODE sub-field.  The same
 *  OPCODEs are defined in this module as 0x0000, 0x2800, 0x3000, 0x3800,
 *  and 0x4000, which are equal to the RFC values times 2^11.
 *
 *  See <RFC 1002, 4.2.1.1> for further information on these fields.
 *
 *  nbt_nsHEADER_FLAGMASK   - There are two reserved bits in the NM_FLAGS
 *                            subfield, and one bit of the RCODE subfield
 *                            is never used.  This mask has all but these
 *                            unused bits set.  
 *
 *  nbt_nsR_BIT             - Response bit.
 *
 *  nbt_nsOPCODE_QUERY      - Query op-code.
 *  nbt_nsOPCODE_REGISTER   - Name registration.
 *  nbt_nsOPCODE_RELEASE    - Name release.
 *  nbt_nsOPCODE_WACK       - Wait for ACKnowledgement.
 *  nbt_nsOPCODE_REFRESH    - Name Refresh.
 *  nbt_nsOPCODE_ALTREFRESH - Name Refresh (alternate).
 *  nbt_nsOPCODE_MULTIHOMED - Multi-homed name registration.
 *  nbt_nsOPCODE_MASK       - Opcode mask.  (flags & nbt_nsOPCODE_MASK) will
 *                            return the opcode by itself.  Useful in
 *                            switch() statements.
 *
 *  nbt_nsAA_BIT            - Authoritative Answer bit. (0)
 *  nbt_nsTR_BIT            - Truncation bit.           (1)
 *  nbt_nsRD_BIT            - Recursion Desired.        (2)
 *  nbt_nsRA_BIT            - Recursion Available.      (3)
 *  nbt_nsB_BIT             - Broadcast mode.           (6)
 *  nbt_nsNMFLAG_MASK       - NM_FLAG subfield mask.
 *                            | 0| 1| 2| 3| 4| 5| 6|
 *                            +--+--+--+--+--+--+--+
 *                            |AA|TR|RD|RA|00|00|B |
 */

#define nbt_nsHEADER_FLAGMASK   0xFF97  /* Masks valid flag field bits. */

#define nbt_nsR_BIT             0x8000  /* Request/Response bit. */

/* OpCodes */
#define nbt_nsOPCODE_QUERY      0x0000  /* Query        = 0x00 */
#define nbt_nsOPCODE_REGISTER   0x2800  /* Registration = 0x05 */
#define nbt_nsOPCODE_RELEASE    0x3000  /* Release      = 0x06 */
#define nbt_nsOPCODE_WACK       0x3800  /* WACK         = 0x07 */
#define nbt_nsOPCODE_REFRESH    0x4000  /* Refresh      = 0x08 */
#define nbt_nsOPCODE_ALTREFRESH 0x4800  /* Refresh      = 0x09 */
#define nbt_nsOPCODE_MULTIHOMED 0x7800  /* Multi-homed  = 0x0F */
#define nbt_nsOPCODE_MASK       0x7800  /* Mask         = 0x0F */

/* NM_FLAGS subfield bits */ 
#define nbt_nsAA_BIT            0x0400  /* Authoritative Answer = 0x40 */
#define nbt_nsTR_BIT            0x0200  /* Truncation Flag      = 0x20 */
#define nbt_nsRD_BIT            0x0100  /* Recursion Desired    = 0x10 */
#define nbt_nsRA_BIT            0x0080  /* Recursion Available  = 0x08 */
#define nbt_nsB_BIT             0x0010  /* Broadcast Flag       = 0x01 */
#define nbt_nsNMFLAG_MASK       0x0790  /* Subfield mask.       = 0x79 */

/*  --
 *  This set represents RCODE (return code) flag values.  Return codes are
 *  part of the flags field.
 *
 *  nbt_nsRCODE_POS_RSP   - Positive Response.  (No error.)
 *  nbt_nsRCODE_FMT_ERR   - Format Error.
 *  nbt_nsRCODE_SRV_ERR   - Server failure.
 *  nbt_nsRCODE_IMP_ERR   - Unsupported request error.
 *  nbt_nsRCODE_RFS_ERR   - Refused error.
 *  nbt_nsRCODE_ACT_ERR   - Active error.
 *  nbt_nsRCODE_CFT_ERR   - Name in conflict error.
 *  nbt_nsRCODE_MASK      - Subfield mask.
 */

#define nbt_nsRCODE_POS_RSP     0x0000  /* Positive Response    */
#define nbt_nsRCODE_FMT_ERR     0x0001  /* Format Error         */
#define nbt_nsRCODE_SRV_ERR     0x0002  /* Server failure       */
#define nbt_nsRCODE_NAM_ERR     0x0003  /* Name Error           */
#define nbt_nsRCODE_IMP_ERR     0x0004  /* Unsupported request  */
#define nbt_nsRCODE_RFS_ERR     0x0005  /* Refused              */
#define nbt_nsRCODE_ACT_ERR     0x0006  /* Active error         */
#define nbt_nsRCODE_CFT_ERR     0x0007  /* Name in conflict     */
#define nbt_nsRCODE_MASK        0x0007  /* Subfield mask        */

/*  --
 *  These are passed to/from functions to indicate which subrecords are
 *  (to be) contained within the packet.  Consider them boolean flags.
 *
 *  nbt_nsQUERYREC        - Query Record.
 *  nbt_nsANSREC          - Answer Record.
 *  nbt_nsNSREC           - Name Server Record (never used).
 *  nbt_nsADDREC          - Additional Record.
 */

#define nbt_nsQUERYREC            0x01  /* Query Record         */
#define nbt_nsANSREC              0x02  /* Answer Record        */
#define nbt_nsNSREC               0x04  /* NS Rec (never used)  */
#define nbt_nsADDREC              0x08  /* Additional Record    */

/*  --
 *  This next batch represents query section values.
 *
 *  nbt_nsQTYPE_NB        - Name Query.
 *  nbt_nsQTYPE_NBSTAT    - Adapter Status Query.
 *  nbt_nsQTYPE_MASK      - Query Type bitmask.  The result of AND-ing either
 *                          of the above query types with this mask will be
 *                          the original query type.
 *
 *  nbt_nsQCLASS_IN       - Internet query class (the only class defined for
 *                          NBT name queries).
 */

#define nbt_nsQTYPE_NB          0x0020  /* Name Query           */
#define nbt_nsQTYPE_NBSTAT      0x0021  /* Adapter Status       */
#define nbt_nsQTYPE_MASK        0x0021  /* Mask                 */

/* Query Class */
#define nbt_nsQCLASS_IN         0x0001  /* Internet Class       */

/*  --
 *  This set represents Resource Record section values.
 *
 *  nbt_nsLSP             - The one and only Label String Pointer value ever
 *                          used in NBT packets.
 *
 *  nbt_nsRRTYPE_A        - IP address Resource Record.
 *  nbt_nsRRTYPE_NS       - Name Server Resource Record.
 *  nbt_nsRRTYPE_NULL     - NULL Resource Record.  (Probably not used.)
 *  nbt_nsRRTYPE_NB       - NetBIOS general Name Service RR.
 *  nbt_nsRRTYPE_NBSTAT   - NetBIOS NODE STATUS RR.
 *  nbt_nsRRTYPE_MASK     - RR Type bitmask.  The result of AND-ing any
 *                          of the above RR types with this mask will be
 *                          the original RR type.
 *
 *  nbt_nsRRCLASS_IN      - Internet class (the only defined class).
 */

#define nbt_nsLSP               0xC00C  /* Pointer to offset 12 */

#define nbt_nsRRTYPE_A          0x0001  /* IP Addr RR (unused)  */
#define nbt_nsRRTYPE_NS         0x0002  /* Name Server (unused) */
#define nbt_nsRRTYPE_NULL       0x000A  /* NULL RR (unused)     */
#define nbt_nsRRTYPE_NB         0x0020  /* NetBIOS              */
#define nbt_nsRRTYPE_NBSTAT     0x0021  /* NB Status Response   */
#define nbt_nsRRTYPE_MASK       0x002B  /* Mask                 */

#define nbt_nsRRCLASS_IN        0x0001  /* Internet Class       */

/*  --
 *  This set represents RDATA flag values.
 *
 *  nbt_nsGROUP_BIT       - If set, it identifies a NetBIOS group name.
 *  nbt_nsONT_B           - The node is a B-mode node.
 *  nbt_nsONT_P           - The node is a P-mode node.
 *  nbt_nsONT_M           - The node is an M-mode node.
 *  nbt_nsONT_H           - The node is an H-mode node.
 *  nbt_nsONT_MASK        - Owner Node Type mask.
 *  nbt_nsNBFLAG_MASK     - NBFlag mask.
 */

#define nbt_nsGROUP_BIT         0x8000  /* Group indicator      */
#define nbt_nsONT_B             0x0000  /* Broadcast node       */
#define nbt_nsONT_P             0x2000  /* Point-to-point node  */
#define nbt_nsONT_M             0x4000  /* Mixed mode node      */
#define nbt_nsONT_H             0x6000  /* MS Hybrid mode node  */
#define nbt_nsONT_MASK          0x6000  /* Owner Node Type mask */
#define nbt_nsNBFLAG_MASK       0xE000  /* Full flags mask      */

/*  --
 *  This next bunch is used with the RDATA.NODE_NAME.NAME_FLAGS field in
 *  an Node Status Reply.
 *
 *  nbt_nsDRG             - If set, name is being released.
 *  nbt_nsCNF             - If set, name is in conflict. 
 *  nbt_nsACT             - Always set.  Name is active.
 *  nbt_nsPRM             - Indicates machine's permanent name.  Not used.
 *  nbt_nsSTATE_MASK      - Mask used to for (DRG | CNF | ACT | PRM).
 *  nbt_nsNAMEFLAG_MASK   - (nbt_snNBFLAG_MASK | nbt_nsSTATE_MASK).
 */

#define nbt_nsDRG               0x1000  /* Deregister state     */
#define nbt_nsCNF               0x0800  /* Conflict state       */
#define nbt_nsACT               0x0400  /* Active state         */
#define nbt_nsPRM               0x0200  /* Permanent state      */
#define nbt_nsSTATE_MASK        0x1E00  /* State bits mask      */
#define nbt_nsNAMEFLAG_MASK     0xFE00  /* Full NAME_FLAGS mask */


/* -------------------------------------------------------------------------- **
 * Macros:
 *
 *  nbt_nsSetTID( hdr, TID )
 *    Input:  hdr - (uchar *) pointer to an NBT NS header.
 *            TID - (uint16_t) Transaction ID.
 *    Notes:  Writes the given TID value to the TID field in the header.
 *
 *  --
 *  This set of macros reads the count fields from an NBT NS header.
 *
 *  The nbt_nsGet*COUNT() macros take a short-cut.  They only look at the
 *  second byte of each 2-byte field.  This is generally okay, as the only
 *  values supported by Samba and Windows are 0 and 1.  This is correct
 *  behavior, as indicated in the RFC packet descriptions.  The return value
 *  may be treated as a boolean.
 *
 *  nbt_nsGetQDCOUNT( hdr )
 *    Input:  hdr - A pointer to an NBT NS header.
 *    Output: Either 0 or nbt_nsQUERYREC.
 *
 *  nbt_nsGetANCOUNT( hdr )
 *    Input:  hdr - A pointer to an NBT NS header.
 *    Output: Either 0 or nbt_nsANSREC.
 *
 *  nbt_nsGetNSCOUNT( hdr )
 *    Input:  hdr - A pointer to an NBT NS header.
 *    Output: Either 0 or nbt_nsNSREC.
 *    Note:   The only packet that sets this value is the REDIRECT NAME QUERY
 *            RESPONSE, which is never used.
 *
 *  nbt_nsGetARCOUNT( hdr )
 *    Input:  hdr - A pointer to an NBT NS header.
 *    Output: Either 0 or nbt_nsADDREC.
 *
 *  --
 *  These macros are used to read values from the NBT NS header.
 *
 *  nbt_nsGetTID( hdr )
 *    Input:  hdr - (uchar *) pointer to an NBT NS header.
 *    Output: The Transaction ID of the packet.
 *
 *  nbt_nsGetFlags( hdr )
 *    Input:  hdr - (uchar *) pointer to an NBT NS header.
 *    Output: The two-byte HEADER.FLAGS field, in host byte order.
 *
 *  nbt_nsGetRmap( hdr )
 *    Input:  hdr - (uchar *) pointer to an NBT NS header.
 *    Output: A uchar flag field.  Each set bit corresponds to one of
 *            these values: [nbt_nsQUERYREC, nbt_nsANSREC, nbt_nsNSREC,
 *            nbt_nsADDREC].  In this toolkit, this value is referred to
 *            as a "Record Map".
 */

#define nbt_nsSetTID( hdr, TID ) nbt_SetShort( (hdr), 0, (TID) )

#define nbt_nsGetQDCOUNT( hdr ) ((((uchar *)(hdr))[5]) ? nbt_nsQUERYREC : 0)

#define nbt_nsGetANCOUNT( hdr ) ((((uchar *)(hdr))[7]) ? nbt_nsANSREC : 0)

#define nbt_nsGetNSCOUNT( hdr ) ((((uchar *)(hdr))[9]) ? nbt_nsNSREC : 0)

#define nbt_nsGetARCOUNT( hdr ) ((((uchar *)(hdr))[11]) ? nbt_nsADDREC : 0)

#define nbt_nsGetTID( hdr ) nbt_GetShort( (hdr), 0 )

#define nbt_nsGetFlags( hdr ) nbt_GetShort( (hdr), 2 )

#define nbt_nsGetRmap( hdr ) ( nbt_nsGetQDCOUNT( hdr ) \
                             | nbt_nsGetANCOUNT( hdr ) \
                             | nbt_nsGetNSCOUNT( hdr ) \
                             | nbt_nsGetARCOUNT( hdr ) )


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int nbt_nsSetHdr( uchar         *bufr,
                  const long     bSize,
                  const uint16_t flags,
                  const uint8_t  rmap );
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


/* ========================================================================== */
#endif /* NBT_NS_PACKET_H */
