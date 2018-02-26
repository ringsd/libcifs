/* ========================================================================== **
 *
 *                                  Escape.c
 *
 * Copyright:
 *  Copyright (C) 2002 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: Escape.c,v 0.1 2002/12/06 06:31:01 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *
 *  This module implements URL escaping/unescaping.
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

#include "SMB/URL/Escape.h"       /* Module header.                        */
#include "util/HexOct.h"          /* Support for hex encode/decode.        */


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int smb_urlUnEsc( char *dst, const char *src, const int size )
  /* ------------------------------------------------------------------------ **
   * Convert URL escape sequences into single byte values.
   *
   *  Input:  dst   - Target string to which to write the un-escaped result.
   *          src   - Source string possibly containing URL escape sequences.
   *          size  - Number of bytes available in dst[].
   *
   *  Output: If >= 0, the string length of the un-escaped string.  (Eg. same
   *          as the result of strlen(dst) following conversion.)
   *          If < 0, an error code.  (All error codes are negative values.)
   *
   *  Errors: cifs_warnLenExceeded  - The size of the destination string (as
   *                                  provided via the <size> parameter) was
   *                                  too small, and the converted string was
   *                                  truncated.  The result of strlen(dst)
   *                                  will be (size-1).
   *
   *  Notes:  The converted string will always be nul terminated, and the
   *          string length of the converted string will always be less than
   *          or equal to (size - 1).
   *
   *          The two string parameters, <dst> and <src>, may be pointers to
   *          the same memory location without consequence.
   *
   *          We try to be forgiving.  This function converts sequences in
   *          the form %XX to an octet value.  If the second X is not a hex
   *          numeric we convert %X and if the first X is not a hex numeric
   *          we simply copy '%' and continue on.
   *
   *          There are probably many URL management functions written in C
   *          out there in the wide world.  This is a very simple function
   *          which only serves to convert %XX sequences into their correct
   *          octet value.  URLs are more complex than that, however, so
   *          a more powerful suite of functions may be necessary.
   *
   *          Note that this library is under the LGPL.  Do not simply copy
   *          code from a GPL'd URI/URL management package into this library
   *          unless you wish to convert the entire library to the GPL for
   *          your own use.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int  i;
  int  pos;
  int  val;
  char scratch[] = "x";

  for( i = 0; i < size; i++ )
    {
    if( '\0' == *src )
      {
      dst[i] = '\0';
      return( i );
      }

    if( '%' != *src )
      {
      dst[i] = *src;
      src++;
      }
    else
      {
      *scratch = toupper( *(++src) );
      pos = (int)strcspn( util_HexDigits, scratch );
      if( pos < 16 )
        {
        val = pos;
        *scratch = toupper( *(++src) );
        pos = (int)strcspn( util_HexDigits, scratch );
        if( pos < 16 )
          {
          val = (val * 16) + pos;
          src++;
          }
        dst[i] = (char)val;
        }
      else
        dst[i] = '%';
      }
    }

  dst[size-1] = '\0';
  return( cifs_warnLenExceeded );
  } /* smb_urlUnEsc */


/* ========================================================================== */
