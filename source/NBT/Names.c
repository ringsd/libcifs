/* ========================================================================== **
 *                                   Names.c
 *
 * Copyright:
 *  Copyright (C) 2000-2004 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: Names.c,v 0.9 2004/10/06 04:30:03 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  This module implements NBT name encoding, decoding, and syntax checking.
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
 *  This module provides functions for syntax checking, encoding, and 
 *  decoding NetBIOS Names and Scope IDs.  The encoded form is known as
 *  an NBT name.
 *
 *  Most of the work done with NBT names received over the wire can be
 *  done without decoding them.  Use the wire format for sorting and
 *  comparing names.  The only reason to decode the name is for display
 *  purposes, which is not as speed-critical as, say, data transfer.
 *
 *  One assumption made by this code is that once a NetBIOS name and scope ID
 *  are encoded, the encoded form will be used in almost all cases.  This
 *  seems to be the way that Windows works but Samba will decode a name
 *  before storing it or comparing it against other names.  This makes Samba
 *  a bit more robust (or more accepting of bad client code) at the expense
 *  of a few CPU cycles.
 *
 *  Old documentation claims that the nul byte (0x00) is an illegal character
 *  in a NetBIOS name.  Unicode has caused this rule to be broken so this
 *  module must accomodate names that contain nul bytes.  That makes the
 *  code a bit more annoying to work with.  NetBIOS names must be handled as
 *  length-delimited strings rather than nul-terminated strings.
 *
 * Bugs:
 *
 *  This module uses the toupper(3) function from the ctype.h library.
 *  The toupper() function works fine with ASCII strings.  I'm not sure how
 *  it works with with byte values greater than 127, but it should interact
 *  with the locale subsystem.  Unfortunately, there is a chance that
 *  toupper() mappings may differ between systems.  If this is the case,
 *  then I'll probably need to write an NBT-specific ToUpper() function.
 *  The toupper() function is used only in nbt_UpCaseStr().
 *
 *  Note that NetBIOS names are 16-byte arrays, so we must deal with them
 *  in a bytewise fashion.  Also, the 16th byte is the suffix byte, so it
 *  must not be converted by toupper().
 *
 * ========================================================================== **
 */

#include <ctype.h>      /* Need toupper() and isalnum() functions.  */
#include <string.h>     /* We use strlen(). */
#include "NBT/Names.h"  /* Module header.   */


/* -------------------------------------------------------------------------- **
 * Macros:
 *
 * EncHiNibble( I )
 * EncLoNibble( I )
 *    - Macros for encoding bytes using RFC1001/1002 half-ascii encoding.
 *
 *      Input:  I - Byte value to be encoded.
 *      Output: The encoded nibble, returned as an integer.
 *      Notes:  EncHiNibble() encodes the high-order four bits,
 *              EncLoNibble() encodes the low-order four bits.
 *      Example:
 *              Given the NetBIOS name NEKO<00>, the first byte would be
 *              encoded as follows:
 *
 *              target[0] = EncHiNibble( 'N' );
 *              target[1] = EncLoNibble( 'N' );
 *
 */

#define EncHiNibble( I ) ('A' + (uchar)(((I) & 0xF0) >> 4))
#define EncLoNibble( I ) ('A' + (uchar)((I) & 0x0F))


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int nbt_UpCaseStr( uchar *src, uchar *dst, int max )
  /* ------------------------------------------------------------------------ **
   * Copy a string, converting it to to upper case in the process.
   *
   *  Input:  src - String to be copied and upcased.
   *          dst - Destination string into which the <src> string will
   *                be copied.  This may be the same as <src>, in which
   *                case the original value of <src> will be overwritten.
   *                A value of NULL will also cause <src> to be converted
   *                in place.
   *          max - maximum number of bytes to copy.  Should be one less
   *                than the number of bytes available in <dst> (to allow
   *                for the nul terminator).  Use -1 to upcase until a nul
   *                terminator is encountered (no length limit).
   *
   *  Output: Length of the output string or, if negative, an error code.
   *
   *  Errors: cifs_errNullInput - Indicates that <src> was NULL.
   *
   *  Notes:  This function is intended to be used to convert un-encoded
   *          NetBIOS names and Scope IDs to upper case.  It operates on
   *          ASCII (octet) strings only, and does not handle Unicode.
   *
   *          Scope IDs should always be converted to upper case before use
   *          on the wire (though there are exceptions in the wild).
   *
   *          NetBIOS names are almost always converted to upper case before
   *          they are L1 Encoded.  In some rare instances (eg., the
   *          malformed names that IIS registers, or for testing) you may
   *          need to leave the case of the name intact.  That's why this
   *          function is not called by the L1 and L2 encoding functions.
   *
   *  See also:
   *          <nbt_L1Encode()>, <nbt_L2Encode()>, <nbt_UpString()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i;

  if( NULL == src )
    return( cifs_errNullInput );

  if( NULL == dst )     /* If <dst> is NULL, then we convert in-place. */
    dst = src;

  if( max < 0 )
    {
    for( i = 0; '\0' != src[i]; i++ )
      dst[i] = toupper( src[i] );
    }
  else
    {
    for( i = 0; i < max; i++ )
      dst[i] = toupper( src[i] );
    }

  return( i );
  } /* nbt_UpCaseStr */


int nbt_CheckNbName( const uchar *name, const int len )
  /* ------------------------------------------------------------------------ **
   * Validate an un-encoded NetBIOS name.
   *
   *  Input:  name  - The NetBIOS name to be validated.
   *          len   - The length, in bytes, of the NetBIOS name or a
   *                  a negative number to use a nul byte as terminator.
   *
   *  Output: If positive, the length (in bytes) of the name
   *          (same as <len> if <len> was positive, else strlen( name )).
   *          Otherwise, a warning or error code.
   *
   *  Errors: cifs_errNullInput     - <name> was NULL.
   *          cifs_errNameTooLong   - Name length is greater than 15 bytes.
   *
   *  Warnings:
   *          cifs_warnNulByte      - <name> contains one or more nul bytes.
   *          cifs_warnEmptyStr     - <name> is the empty string, "".
   *          cifs_warnAsterisk     - <name> begins with an asterisk ('*').
   *          cifs_warnContainsDot  - <name> contains a dot.
   *                                  Some Windows systems will interpret
   *                                  names with dots as DNS names.
   *
   *  Notes:  Errors take precedence over warnings.  The warning values
   *          will not be returned unless no errors are detected.
   *
   *          According to IBM documentation, an empty string is not a valid
   *          NetBIOS name.  In practice, all sorts of odd names are used so
   *          we just return a warning if <len> is zero or the string is "".
   *
   *          RFC 1001/1002 state that NetBIOS names may not begin with an
   *          asterisk ('*').  The asterisk is used as a wildcard character,
   *          and the wildcard name ('*' with nul padding and suffix) must
   *          be encoded, so the presense of a leading '*' returns a warning
   *          instead of an error.
   *
   *          Some implementations may choose to follow Microsoft's lead
   *          and interpret either a cifs_errNameTooLong error or a
   *          cifs_warnContainsDot warning as in indication that <name>
   *          should be treated as a DNS name.
   *
   *          The interface to this function seems goofy.  An early version
   *          of this function did not make use of the <len> parameter and,
   *          most of the time, that was good.  Unfortunately, names with
   *          embedded nul bytes were spotted in the wild.  To use this
   *          function the old way, pass -1 via <len>.
   *
   *  See also:
   *          <nbt_CheckScope()>, <nbt_CheckL2Name()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int  i;

  /* Make sure there really is a name.
   */
  if( NULL == name )
    return( cifs_errNullInput );

  /* Long names should never get this far...but they do.
   */
  if( len > 15 )
    return( cifs_errNameTooLong );

  /* If the name is the empty string, no other warnings will apply.
   */
  if( (len == 0) || ((len < 0) && ('\0' == *name)) )
    return( cifs_warnEmptyStr );

  /* Check for leading asterisk.
   */
  if( '*' == *name )
    return( cifs_warnAsterisk );

  /* Look for dots or nuls.
   * If (len < 0), then a nul is a terminator.
   */
  for( i = 0; ((len < 0) || (i < len)); i++ )
    {
    if( '.' == name[i] )
      return( cifs_warnContainsDot );
    if( '\0' == name[i] )
      {
      if( len < 0 )
        return( i );
      else
        return( cifs_warnNulByte );
      }
    }

  return( len );
  } /* nbt_CheckNbName */


int nbt_CheckScope( const uchar *scope )
  /* ------------------------------------------------------------------------ **
   * Validate the syntax of the Scope ID.
   *
   *  Input:  scope - Scope string (unencoded) to be validated.
   *                  This is a nul-terminated string.
   *
   *  Output: If zero or greater, the return value represents the length of
   *          the scope ID string (that is, strlen( scope )).  If negative,
   *          the return value indicates that a syntax problem was detected.
   *
   *  Errors: cifs_errNullInput     - NULL input.
   *          cifs_errLeadingDot    - Starts with empty label (leading dot).
   *          cifs_errDoubleDot     - Contains empty label (dot pairs).
   *          cifs_errEndDot        - Ends with empty label (trailing dot).
   *          cifs_errInvalidLblLen - Contains a label that exceeds 63 bytes.
   *          cifs_errScopeTooLong  - Total Scope ID exceeds maximum length.
   *
   *  Warnings:
   *          cifs_warnNonPrint     - Label contains a non-printing character.
   *          cifs_warnNonAlpha     - Does not start with an alpha character.
   *          cifs_warnInvalidChar  - Contains an invalid character.
   *          cifs_warnNonAlphaNum  - Does not end with an alpha-numeric.
   *
   *  Notes:  A scope length of 0 is perfectly valid.
   *
   *          The warnings are there simply for pedanticism, though it
   *          might be wise to pass them along to a user.  W/95 doesn't
   *          seem to check the scope for valid characters at all.
   *
   *          The errors listed will prevent the scope from being encoded
   *          properly.  Do not pass a scope ID to nbt_L2Encode() if
   *          nbt_CheckScope() returns an error.
   *
   *          Only one error/warning code is returned, so a weak attempt
   *          has been made to prioritize.  Errors, of course, are higher
   *          priority than warnings.
   *
   *  See also:
   *          <nbt_CheckNbName()>, <nbt_CheckL2Name()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int  i;
  int  j;
  bool nonprint     = false;
  bool leadnonalpha = false;
  bool midnonalnum  = false;
  bool endnonalnum  = false;

  /* Check for NULL input.
   */
  if( NULL == scope )
    return( cifs_errNullInput );

  /* Check for leading dot.
   */
  if( '.' == *scope )
    return( cifs_errLeadingDot );

  /* Check for various problems in the body of the scope string.
   */
  for( i = j = 0; ('\0' != scope[i]) && i < 221; i++ )
    {
    if( !isprint( scope[i] ) )
      nonprint = true;          /* Non-printing character. */

    if( '.' == scope[i] )
      {
      if( 0 == j )
        return( cifs_errDoubleDot );  /* Found ".." */
      j = 0;
      if( !isalnum( scope[i-1] ) )    /* Found non-alphanumeric at label end. */
        endnonalnum = true;
      }
    else
      {
      if( ++j > 63 )
        return( cifs_errInvalidLblLen );  /* Label too long. */
      if( 1 == j )
        {
        if( !isalpha( scope[i] ) )
          leadnonalpha = true;            /* First label char non-alpha. */
        }
      else
        {
        if( !(('-' == scope[i]) || isalnum( scope[i] )) )
          midnonalnum = true;             /* Found invalid char in label. */
        }
      }
    }

  /* Check to see if total Scope ID length is too long.
   */
  if( i > 220 )
    return( cifs_errScopeTooLong );

  /* The Scope ID should not end in a dot.
   */
  if( (0 == j) && (i > 0) )
    return( cifs_errEndDot );

  /* Check for non-alphanumeric at final label end.
   */
  if( (i > 0) && !isalnum( scope[i-1] ) )
    endnonalnum = true;

  /* Report any collected warning.
   */
  if( nonprint )
    return( cifs_warnNonPrint );
  if( leadnonalpha )
    return( cifs_warnNonAlpha );
  if( midnonalnum )
    return( cifs_warnInvalidChar );
  if( endnonalnum )
    return( cifs_warnNonAlphaNum );

  return( i );
  } /* nbt_CheckScope */


int nbt_CheckL2Name( const uchar *src, int srcpos, const int srcmax )
  /* ------------------------------------------------------------------------ **
   * Verify that a Level Two Encoded NBT name in a NBT NS packet is okay.
   *
   *  Input:  src     - Pointer to a byte array containing the Name Service
   *                    packet.
   *          srcpos  - Starting position of the encoded NBT name within
   *                    the <src> buffer.
   *          srcmax  - Size of buffer.  If <srcpos> is beyond <srcmax>
   *                    the function will return an OutOfBounds error.
   *
   *  Output: If positive, the length of the encoded NBT name *including*
   *          the terminating nul label.  If negative, an error code.
   *
   *  Errors: cifs_errInvalidLblLen - The starting label was invalid.  The
   *                                  first label should always be 0x20,
   *                                  indicating a length of 32 bytes.
   *          cifs_errBadLblFlag    - A non-zero label flag was found.
   *                                  (Could be a label string pointer.)
   *          cifs_errOutOfBounds   - Initial <srcpos> is beyond <srcmax>. 
   *          cifs_errTruncatedBufr - NBT name string exceeds the end of src.
   *          cifs_errNameTooLong   - NBT name is greater than 255 bytes.
   *
   *  Notes:  This function expects to start at the beginning of the L2
   *          encoded NBT name.  The first label is always the encoded
   *          NetBIOS name which is 32 bytes long.  The first length field
   *          will, therefore, have a value of 32 else an error is returned.
   *
   *          In NBT, label string pointers (LSPs) are only used at the
   *          start of names, and only in certain record types.  Use the
   *          <nbt_CheckLSP()> function to determine whether the first
   *          label is an LSP.  If it is *not*, then the <cifs_errBadLblFlag>
   *          error indicates a syntax error in the NBT name.
   *
   *          On success, this function returns the length (including the
   *          final root label) of the encoded NBT name.  You can also get
   *          that value (without the overhead of syntax checking) using
   *          strlen():
   *
   *            1 + strlen( &src[srcpos] );
   *
   *          The <nbt_L2NameLen()> macro implements the above formula.
   *
   *  See also:
   *          <nbt_CheckScope()>, <nbt_CheckNbName()>, <nbt_CheckLSP()>,
   *          <nbt_L2NameLen()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int len;
  int startpos = srcpos;

  /* Make sure we are within the limits.
   */
  if( srcpos >= srcmax )
    return( cifs_errOutOfBounds );

  /* First label should always have length 0x20.
   */
  if( 0 != (src[srcpos] & 0xC0) )
    return( cifs_errBadLblFlag );
  if( 0x20 != src[srcpos] )
    return( cifs_errInvalidLblLen );
  len = 32;

  /* Check each additional label in turn.
   */
  while( len > 0 )
    {
    srcpos += len + 1;
    if( srcpos > srcmax )
      return( cifs_errTruncatedBufr );
    if( 0 != (src[srcpos] & 0xC0) )
      return( cifs_errBadLblFlag );
    len = (int)(src[srcpos] & 0x3F);
    }

  /* We stopped with srcpos indicating the final label.
   */
  len = (srcpos + 1) - startpos;
  if( len > 255 )
    return( cifs_errNameTooLong );

  /* All okay.
   */
  return( len );
  } /* nbt_CheckL2Name */


int nbt_CheckLSP( uchar lablen )
  /* ------------------------------------------------------------------------ **
   * Check a label length octet to see if it contains a label string pointer.
   *
   *  Input:  lablen  - An octet, assumed to be a label length field from
   *                    a 2nd level encoded NBT name.
   *
   *  Output: If positive, the label length is a label string pointer and
   *          the return value is the offset given in the LSP.
   *          If zero, then <lablen> was not an LSP and the label length
   *          is simply (int)lablen.
   *          If negative, an error.
   *
   *  Errors: cifs_errInvalidLblLen - Inidcates that <lablen> has an invalid
   *                                  flag set, so it cannot be a valid
   *                                  label length or label string pointer.
   *
   *  See also:
   *          <nbt_CheckL2Name()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int flags = lablen & 0xC0;

  if( 0xC0 == flags )
    return( lablen & 0x3F );

  if( flags )
    return( cifs_errInvalidLblLen );

  return( 0 );
  } /* nbt_CheckLSP */


int nbt_L1Encode( uchar *dst, const nbt_NameRec *src )
  /* ------------------------------------------------------------------------ **
   * Encode a NetBIOS name using First Level Encoding.
   * <RFC 1001, Section 14.1>
   *
   *  Input:  dst - Pointer to a target buffer into which the encoded
   *                name will be written.  Minimum buffer size is 33
   *                bytes.
   *          src - A pointer to an nbt_NameRec structure.
   *                The src->scope_id field is ignored, but all other
   *                fields are used to create the L1 encoded name.
   *
   *  Output: String length of the resultant string (always 32).
   *
   *  Errors: None defined.
   *
   *  Notes:  This function does *no* syntax checking.  Use
   *          <nbt_CheckNbName()> to check the name before calling this
   *          function.  (Syntax checking is skipped so that unusual names
   *          can be encoded.)
   *
   *          This function does *not* convert the NetBIOS name to uppercase.
   *          That must be done in a separate step, prior to encoding the
   *          name.  (There are odd cases in which names are not up-cased
   *          before being encoded.)
   *
   *          To be pedantic, when the RFCs talk about First Level Encoding
   *          they mean both the encoded (32-byte form) NetBIOS name *and*
   *          the appended Scope ID.  The fully qualified name is rarely
   *          used in its First Level Encoded form, however, so it seems
   *          simpler to perform only the half-ascii encoding in this
   *          function.  If you need the complete L1 name, it's simply
   *            L1_NetBIOS_name + "." + scope_id
   *
   *          The encoded NetBIOS name is always 33 bytes long, including
   *          the terminating nul byte.  If the Scope ID is the the empty
   *          string, then those 33 bytes represent the Second Level Encoded
   *          NBT name (sans the leading length byte).
   *
   *  See Also:
   *          <nbt_UpCaseStr()>, <nbt_L2Encode()>, <nbt_L1Decode()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int   i,  j;
  uchar hi, lo;

  /* Encode the name using RFC 1001/1002 First Level Encoding.
   */
  hi = ( src->namelen > 15 ) ? 15 : src->namelen;   /* Ensure max 15 bytes. */
  for( i = 0, j = 0; i < hi; i++ )
    {
    dst[j++] = EncHiNibble( src->name[i] );
    dst[j++] = EncLoNibble( src->name[i] );
    }

  /* Encode the pad byte and fill in any unused name bytes.
   */
  hi = EncHiNibble( src->pad );
  lo = EncLoNibble( src->pad );
  while( j < 30 )
    {
    dst[j++] = hi;
    dst[j++] = lo;
    }

  /* Encode the suffix byte and place it at the end,
   * then terminate the string.
   */
  dst[30] = EncHiNibble( src->sfx );
  dst[31] = EncLoNibble( src->sfx );
  dst[32] = '\0';

  /* Return the resulting string length. */
  return( 32 );
  } /* nbt_L1Encode */


int nbt_L1Decode( uchar       *dst,
                  const uchar *src,
                  const int    srcpos,
                  const uchar  pad,
                  uchar       *sfx )
  /* ------------------------------------------------------------------------ **
   * Decode a Level One Encoded NetBIOS name.
   *
   *  Input:  dst     - Target into which the name will be written.  This
   *                    should be a minimum of 16 bytes.  The decoded name
   *                    will be a maximum of 15 bytes, plus one byte for the
   *                    nul byte string terminator.
   *          src     - Pointer to a buffer which contains an L1 encoded
   *                    NetBIOS name.
   *          srcpos  - Offset into <src> at which the L1 encoded NetBIOS
   *                    name begins.
   *          pad     - The padding character.
   *                    Trailing <pad> bytes will be replaced with nul.
   *                    Eg., if <pad> is the space character (0x20 == ' '),
   *                    then trailing spaces will be replaced with nuls in
   *                    the decoded NetBIOS name.  If the nul byte is given,
   *                    then no trailing bytes will be trimmed.
   *                    Typically, you should use either ' ' or '\0'.
   *          sfx     - Pointer to an unsigned byte to receive the value
   *                    of the suffix byte.
   *
   *  Output: If non-negative, then it is the length of the decoded NetBIOS
   *          name.  Otherwise, an error code.
   *
   *  Errors: cifs_errBadL1Value  - Indicates that the source name contains
   *                                a byte value outside of the 'A'..'P'
   *                                range of valid values.
   *
   *  Notes:  <src> may either point to the Level One encoded form of the
   *          NBT name or to the second byte of the Level Two Encoded form.
   *          (The second byte of an L2 encoded name starts the L1 name.)
   *
   *          See the notes in <nbt_L2Decode()> regarding decoding of a
   *          Level Two Encoded scope ID.
   *
   *          You can use this function to decode a string into the same
   *          location as the source.  That is (dst == &src[srcpos]) is
   *          okay.
   *
   *          The resulting string *may* contain non-printing characters.
   *          Consider, for example, the "\x1\x2__MSBROWSE__\x2<01>" name,
   *          commonly used by Browser nodes.
   *
   *          The resulting <dst> string may also include nul bytes (because
   *          some systems encode Unicode strings).  Be sure to use the
   *          return value (assuming it's not an error code) as the string
   *          length.
   *
   *  See Also:   <nbt_L2Decode()>, <nbt_L1Encode()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i, j;
  int nibble;

  /* Every two encoded bytes reduces to a single NetBIOS name byte.
   */
  for( i = 0, j = srcpos; i < nbt_NB_NAME_MAX; i++ )
    {
    nibble = (src[j++] - 'A');              /* First nibble. */
    if( (nibble < 0) || (nibble > 0x0F) )
      return( cifs_errBadL1Value );
    dst[i] = (uchar)(nibble << 4);

    nibble = (src[j++] - 'A');              /* Second nibble. */
    if( (nibble < 0) || (nibble > 0x0F) )
      return( cifs_errBadL1Value );
    dst[i] |= nibble;
    }

  /* Move the suffix out of the way and terminate the string.
   */
  *sfx = dst[15];
  dst[15] = '\0';

  /* Return 15 if we're not stripping the padding.
   */
  if( '\0' == pad )
    return( 15 );

  /* Trim padding from end and return the resulting string length.
   */
  for( i = 14; (i >= 0) && (pad == dst[i]); i-- )
    dst[i] = '\0';
  return( i + 1 );

  } /* nbt_L1Decode */


int nbt_L2Encode( uchar *dst, const nbt_NameRec *namerec )
  /* ------------------------------------------------------------------------ **
   * Encode a NetBIOS name and Scope ID using Second Level Encoding.
   * <RFC 1002, Section 4.1>
   *
   *  Input:  dst     - Pointer to a target buffer into which the encoded
   *                    name will be written.  Minimum buffer size is 255
   *                    bytes.
   *          namerec - A pointer to an nbt_NameRec structure which contains
   *                    all of parts to be assembled into an L2 NBT name.
   *
   *  Output: The number of bytes in the resultant encoded name.  This
   *          count includes the final nul byte, because the trailing
   *          nul represents the root of the DNS namespace and is,
   *          therefore, part of the NBT name.
   *
   *  Errors: None defined.
   *
   *  Notes:  No syntax checking is performed on the NetBIOS name or the
   *          Scope ID.  They should be checked before they get this far.
   *          Syntax checks are skipped at this stage to accomdate unusual
   *          or non-standard values.
   *
   *          Neither the Scope ID nor the NetBIOS name are upper-cased
   *          before they are encoded.  That should be done in a separate
   *          step before this function is called.
   *
   *          The Scope ID in <namerec> may be NULL.  A value of NULL is
   *          handled as if it were the empty string ("").
   *
   *  See Also:
   *          <nbt_UpCaseStr()>, <nbt_L2Decode()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int    lenpos;
  int    i;
  int    j;
  uchar *scope = namerec->scope_id;

  /* First-level encode the NetBIOS name,
   * add label length, and move lenpos to the end.
   */
  (void)nbt_L1Encode( &dst[1], namerec );
  dst[0] = 0x20;
  lenpos = 33;

  /* Encode each label in the scope ID.
   */
  if( (NULL != scope) && ('\0' != *scope) )
    {
    do
      {
      /* Count label lenght as it is being copied to dst.
       */
      for( i = 0, j = (lenpos + 1);
           ('.' != scope[i]) && ('\0' != scope[i]);
           i++, j++)
        dst[j] = scope[i];

      /* Write the length one byte before of the copied label,
       * advance lenpos to the next writeable space, and
       * advance scope to the next terminator.  If that terminator
       * is a '.', then we will increment scope and continue.
       */
      dst[lenpos] = (uchar)(0x3F & i);
      lenpos     += i + 1;
      scope      += i;
      } while( '.' == *(scope++) );
    dst[lenpos] = '\0';
    }

  /* Add one to lenpos to get actual length of the encoded name.
   */
  return( lenpos + 1 );
  } /* nbt_L2Encode */


int nbt_L2Decode( uchar *dst, const uchar *src, int srcpos )
  /* ------------------------------------------------------------------------ **
   * Decode an L2 encoded NBT name string.
   *
   *  Input:  dst     - Target buffer.  To be safe, this should be at least
   *                    <nbt_NAME_MAX> bytes in size.  It is sufficient to
   *                    provide a buffer that is strlen( &(src[srcpos]) )
   *                    bytes long (that is, the same length as the string
   *                    being decoded).
   *          src     - Source buffer. Probably a Name Service packet.
   *          srcpos  - Location of the NBT name within the packet.
   *                    This function does *not* follow Label String
   *                    Pointers.
   *
   *  Output: If non-negative, the string length of the decoded NBT name.
   *          If negative, an error code.
   *
   *  Errors: none returned.
   *
   *  Notes:  This function does not validate the NBT name.
   *          Use the <nbt_CheckL2Name()> function.
   *
   *          To decode just the scope of an NBT name, set <srcpos> 33 bytes
   *          beyond the start of the name in <src>.  The first label should
   *          always be the 32-byte encoded NetBIOS name plus its label
   *          length byte.  You can L2Decode the entire NBT name, but then
   *          you still need to decode the L1-encoded NetBIOS name.  It is
   *          easier to decode the two parts separately.
   *
   *          Regarding the size of the <dst> buffer:  One byte is lost when
   *          decoding L2 encoding.  For example, consider the L2 encoded
   *          name:
   *            "\x20EOGFGLGPCACACACACACACACACACACAAA\x03CAT\x03ORG\0"
   *          The length of that name is 42 bytes.  If you use the strlen()
   *          function on the above string you will get 41, because the
   *          strlen() function sees the trailing '\0' as a terminator, not
   *          a label.  That's all okay, though, because when the string is
   *          decoded the result will be:
   *            "EOGFGLGPCACACACACACACACACACACAAA.CAT.ORG"
   *          which has an strlen() of 40, but requires 41 bytes (one to
   *          store the terminating nul).  So, strlen( &(src[srcpos]) ) is
   *          the minimum required to store the resultant string.
   *
   *  See Also:
   *          <nbt_CheckL2Name()>, <nbt_L2Encode()>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int len, i, j;

  i   = 0;
  len = src[srcpos++];
  while( len > 0 )
    {
    for( j = 0; j < len; j++ )
      dst[i++] = src[srcpos++];
    len = src[srcpos++];
    if( len > 0 )
      dst[i++] = '.';
    }
  dst[i] = '\0';

  return( i );
  } /* nbt_L2Decode */


int nbt_EncodeName( uchar             *dst,
                    const int          dstpos,
                    const int          dstlen,
                    const nbt_NameRec *namerec )
  /* ------------------------------------------------------------------------ **
   * Fully encode an NBT name using parts provided in the input name record.
   *
   *  Input:  dst     - Pointer to the destination buffer into which the
   *                    encoded name will be written.
   *          dstpos  - Offset into <dst> at which to write the encoded
   *                    name.
   *          dstlen  - Total bytes in <dst>.  The encoded name must fit
   *                    into (dstlen - dstpos) bytes.
   *          namerec - A pointer to a name record.
   *                    The <name>, <pad>, <sfx>, and <scope_id> fields
   *                    should be properly initialized (a <scope_id> of
   *                    NULL is equivalent to the empty scope: "").
   *
   *  Output: If negative, an error or warning code.  If positive, the
   *          number of bytes in the resultant encoded name.  This
   *          count includes the final nul byte, because the trailing
   *          nul represents the root of the DNS namespace and is,
   *          therefore, part of the NBT name.
   *
   *  Errors: cifs_errNullInput     - <namerec> or <dst> was NULL, or the
   *                                  <name> field of <namerec> was NULL.
   *          cifs_errNameTooLong   - The length of <name> exceeded 15 bytes.
   *          cifs_errLeadingDot    - The <scope_id> starts with empty label
   *                                  (leading dot).
   *          cifs_errDoubleDot     - The <scope_id> contains an empty label
   *                                  (multiple dots).
   *          cifs_errEndDot        - <scope_id> ends with empty label
   *                                  (trailing dot).
   *          cifs_errInvalidLblLen - A <scope_id> label exceeds 63 bytes.
   *          cifs_errScopeTooLong  - <scope_id> exceeds maximum length.
   *          cifs_errBufrTooSmall  - Not enough room in <dst> to hold the
   *                                  encoded result.
   *
   *  Warnings:
   *          cifs_warnContainsDot  - <name> contains a dot.
   *                                  Some Windows systems will interpret
   *                                  names with dots as DNS names.
   *          cifs_warnEmptyStr     - <name> is the empty string, "".
   *          cifs_warnNonPrint     - <scope_id> contains a non-printing
   *                                  character.
   *          cifs_warnNonAlpha     - A <scope_id> label does not start with
   *                                  an alpha character (as it should).
   *          cifs_warnInvalidChar  - A <scope_id> label contains an invalid
   *                                  character (use alphanumeric or '-').
   *          cifs_warnNonAlphaNum  - A <scope_id> label does not end with
   *                                  an alpha-numeric.
   *
   *  Output: If a positive value is returned, it is the length of the
   *          fully encoded name string (including the final nul label).
   *          If the return value is negative, it is an error or warning
   *          indicator.
   *
   *  Notes:  This function is a convenient front-end to the nbt_L2Encode()
   *          function (above).  It also performs all of the recommended
   *          syntax checks.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int result;
  int length  = nbt_L2_NB_NAME_MIN;
  int warning = 0;

  /* Check for valid input.
   */
  if( NULL == namerec || NULL == dst )
    return( cifs_errNullInput );

  /* Validate the NetBIOS name.
   */
  result = nbt_CheckNbName( namerec->name, namerec->namelen );
  if( result < 0 )
    return( result );

  /* If there's a non-empty Scope ID, check it.
   * Store warnings, but continue with the encoding anyway.
   */
  if( (NULL != namerec->scope_id) && ('\0' != *(namerec->scope_id)) )
    {
    result = nbt_CheckScope( namerec->scope_id );
    if( result < 0 && cifs_errIsError( result ) )
      return( result );
    if( cifs_errIsWarn( result ) )
      {
      warning = result;
      result  = strlen( (char *)namerec->scope_id );
      }
    length += result + 1;
    }

  /* Verify that the buffer is large enough.
   */
  if( ((dstlen - dstpos) - length) < 0 )
    return( cifs_errBufrTooSmall );

  /* Okay.  Let's go.
   */
  result = nbt_L2Encode( dst + dstpos, namerec );
  if( result > 0 && warning )
    return( warning );
  return( result );
  } /* nbt_EncodeName */


/* ========================================================================== */
