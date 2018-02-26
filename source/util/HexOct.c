/* ========================================================================== **
 *
 *                                  HexOct.c
 *
 *  Copyright (C) 2002,2003 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: HexOct.c,v 0.15 2010-11-27 23:27:45 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  Utilities for converting to/from Hexidecimal and Octal notations.
 *
 * -------------------------------------------------------------------------- **
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
 *  This module supports the conversion of escape sequences and hex strings
 *  into byte values.  Converting the other way is both simpler and more
 *  complex.
 *
 *  It is simpler because it is easy to do using printf(3) and its family.
 *  Eg.: snprintf( str, strlen, "\\x%.2x", byteval );
 *  It is more complex because:
 *
 *  - You need to figure out which bytes you want to encode (typically
 *    by doing something like:
 *      if( isprint( src[i] ) && !isspace( src[i] ) )
 *        {...}
 *    for each byte in the source string.
 *
 *  - The string may expand by as much as four times, depending upon the
 *    format used to display the hex value strings.
 *
 *  The util_HexDigits[] string and the util_Hexify() function are provided
 *  to help create printable strings from strings containing unprintable
 *  octet values.
 *
 * ========================================================================== **
 */

#include <ctype.h>          /* For isprint(3).                   */

#include "cifs_common.h"    /* CIFS library common include file. */


/* -------------------------------------------------------------------------- **
 * Macros:
 *  hiNibble  - Return the value of a byte's high nibble, shifted of course.
 *  loNibble  - Return the value of a byte's low nibble.
 */

#define hiNibble( I ) ((uchar)(((I) & 0xF0) >> 4))
#define loNibble( I ) ((uchar)((I) & 0x0F))


/* -------------------------------------------------------------------------- **
 * Global constants:
 */

const char util_HexDigits[] = "0123456789ABCDEF";


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int util_XlateOdigit( const uchar digit )
  /* ------------------------------------------------------------------------ **
   * Converts a character that is in the set of oct digits to its respective
   * three-bit value.
   *
   *  Input:  digit - A character, which should be a valid octal digit.
   *                  That is, one of "01234567".
   *
   *  Output: An integer value in the range 0..7, or a negative number.
   *          A negative return value indicates that the input character
   *          was not a valid octal digit.
   *
   *  Errors: No particular error code.  A negative return value should be
   *          interpreted as meaning that the input was invalid.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i;

  i = (int)digit - '0';
  if( i > 7 )
    return( -1 );
  return( i );
  } /* util_XlateOdigit */


int util_XlateXdigit( const uchar digit )
  /* ------------------------------------------------------------------------ **
   * Converts a character that is in the set of hex digits to its respective
   * nibble (four-bit) value.  Eg. 'A' ==> 10.
   *
   *  Input:  digit - A character, which should be a valid hex digit.
   *                  That is, one of "0123456789ABCDEFabcdef".
   *
   *  Output: An integer value in the range 0..15, or a negative number.
   *          A negative return value indicates that the input character
   *          was not a valid hex digit.
   *
   *  Errors: No particular error code.  A negative return value should be
   *          interpreted as meaning that the input was invalid.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  /* This looks bulky, but it should be fairly efficient.
   * Another option would be an array of some sort.
   */
  switch( digit )
    {
    case '0': return(  0 );
    case '1': return(  1 );
    case '2': return(  2 );
    case '3': return(  3 );
    case '4': return(  4 );
    case '5': return(  5 );
    case '6': return(  6 );
    case '7': return(  7 );
    case '8': return(  8 );
    case '9': return(  9 );
    case 'A':
    case 'a': return( 10 );
    case 'B':
    case 'b': return( 11 );
    case 'C':
    case 'c': return( 12 );
    case 'D':
    case 'd': return( 13 );
    case 'E':
    case 'e': return( 14 );
    case 'F':
    case 'f': return( 15 );
    }
  return( -1 );
  } /* util_XlateXdigit */


uchar util_UnEscSeq( const uchar *src, int *esclen )
  /* ------------------------------------------------------------------------ **
   * Translate a C language escape sequence (a. la. printf()) into its
   * character (byte) value.
   *
   *  Input:  src     - source string to read.
   *          esclen  - pointer to an int that will receive the length of
   *                    the escape sequence.  If NULL, it will be ignored.
   *
   *  Output: The one-byte value indicated by the escape sequence.
   *          If no escape sequence was found, the value of src[0] is
   *          returned and <esclen> will have a value of 1.
   *
   *  Notes:  Escape sequences vary in length.  A real escape sequence has a
   *          minimum length of 2 (eg. "\n") and a maximum length of four
   *          (eg. "\176").  That's why we need to return a value in
   *          <esclen>.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int c;
  int i;
  int myesclen = 0;

  if( !esclen )
    esclen = &myesclen;

  /* If first char is not '\\' then return first character. */
  if( '\\' != src[0] )
    {
    *esclen = 1;
    return( src[0] );
    }

  /* First char was '\\'. */
  *esclen = 2;
  switch( src[1] )
    {
    case 'a': return( '\a' );
    case 'b': return( '\b' );
    case 'f': return( '\f' );
    case 'n': return( '\n' );
    case 'r': return( '\r' );
    case 't': return( '\t' );
    case 'v': return( '\v' );
    case 'x':
    case 'X':
      {
      i = util_XlateXdigit( src[2] );
      if( i < 0 )
        return( src[1] );  /* x or X */
      c = i;
      i = util_XlateXdigit( src[3] );
      if( i < 0 )
        {
        /* Single-char hex number; \xH, where H is the Hex Digit. */
        *esclen = 3;
        return( c );
        }
      /* Two-char hex number; \xHH. */
      c <<= 4;
      c |= i;
      *esclen = 4;
      return( c );
      }
    case '0':   /* Octal */
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
      c = util_XlateOdigit( src[1] );
      i = util_XlateOdigit( src[2] );
      if( i < 0 )
        {
        /* Single-char octal number; \O, where O is the Octal Digit. */
        *esclen = 2;
        return( c );
        }

      /* At least two octal digits; \OO. */
      c <<= 3;
      c |= i;
      i = util_XlateOdigit( src[3] );
      if( i < 0 )
        {
        *esclen = 3;
        return( c );
        }
      /* Three octal digits; \OOO. */
      c <<= 3;
      c |= i;
      *esclen = 4;
      return( c );
      }
    }

  return( src[1] );
  } /* util_UnEscSeq */


int util_XlateInput( const char *src )
  /* ------------------------------------------------------------------------ **
   * Converts from various hex string formats into an integer value.
   *
   *  Input:  src - Pointer to a character array containing the text to be
   *                converted.  It is assumed that the input will be in one
   *                of the following formats:
   *
   *                "A"     = A single non-hex-digit character, which is
   *                          returned as-is.
   *                "X"     = A single hex digit, returned as if it were
   *                          "0X".
   *                "XX"    = Two characters such that isxdigit(src[n])
   *                          is true for n=0 and n=1.
   *                "%XX"   = As above, with a leading % sign.
   *                "#XX"   = As above, with a leading # sign.
   *                "\xXX"  = As above, with a leading "\x".
   *                "<XX>"  = Two hex digit characters within angle brackets.
   *                "0xXX"  - Two hex digit characters with a leading "0x".
   *
   *  Output: The one-byte translation of the input, or a negative value if
   *          there was an error interpreting the input.
   *
   *  Notes:  The goal is to provide a whole mess of easy ways for the user
   *          to specify a byte value, in hex, on the command line.  The code
   *          is quite forgiving.
   *
   *          If the input string is NULL or the empty string ("") the
   *          function returns -1.  A value of -1 is also returned if the
   *          input cannot be translated (invalid hex digits, etc.).
   *
   *          If the input string contains only one non-hex-digit character,
   *          the value of that character is returned.  This allows people
   *          to enter a limited set of literal values (eg. ' ').
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int lonibble = 0;
  int hinibble = 0;
  int start;
  int len;

  /* Check for invalid input. */
  if( (NULL == src) || (len = strlen( src )) < 1 )
    return( -1 );

  /* If the string is only one byte long, and it isn't a hex byte,
   * take that byte literally.
   */
  if( 1 == len )
    {
    hinibble = util_XlateXdigit( *src );
    return( (hinibble < 0) ? *src : hinibble );
    }

  /* Look for our lead-in strings ('%','#','<','\\',"0x").
   * If we get no lead-in then assume that the input is in the form "XX".
   * The result of this switch() statement is that <start> will be set
   * to the starting position of the hex string to be translated.
   */
  switch( *src )
    {
    case '%':
    case '#':
    case '<':
      /* In these three cases, ignore the first byte and attempt to read
       * the next two characters as hex digits.
       */
      start = 1;
      break;
    case '\\':
    case '0':
      /* If the first char is '\\', then the second byte should be 'x' (or 'X').
       * If the first char is '0', then the second byte *might* be 'x' (or 'X').
       */
      if( ('x' == src[1]) || ('X' == src[1]) )
        {
        if( len < 3 )
          return( -1 );
        start = 2;
        }
      else
        {
        if( '\\' == *src )
          return( -1 );
        start = 0;
        }
      break;
    default:
      start = 0;
      break;
    }

  /* Okay, now translate one or two hex digits into a single byte value. */
  hinibble = util_XlateXdigit( src[start] );
  if( hinibble < 0 )
    return( hinibble );

  lonibble = util_XlateXdigit( src[start+1] );
  if( lonibble < 0 )
    return( hinibble );

  return( (hinibble << 4) | lonibble );
  } /* util_XlateInput */


int util_UnEscStr( uchar *str )
  /* ------------------------------------------------------------------------ **
   * Convert escape sequences in a string to bytes.
   *
   *  Input:  str - String to translate.
   *
   *  Output: The length of the resultant string.
   *
   *  Notes:  The new string will be of equal or shorter length.
   *
   *          This function operates *on* the input string.
   *          If you want to create a new string, use strdup(3) on the
   *          original string, then pass in the new duplicate.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int    i, j;
  int    len;
  int    esclen;

  i = j = 0;
  len = strlen( (char *)str );
  while( i <= len )
    {
    if( str[i] == '\\' )
      {
      str[j++] = util_UnEscSeq( &str[i], &esclen );
      i += esclen;
      }
    else
      str[j++] = str[i++];
    }

  return( j-1 );
  } /* util_UnEscStr */


int util_Hexify( uchar *dst, const uchar *src, int len )
  /* ------------------------------------------------------------------------ **
   * Nasty function to write a hex-escaped string from a source string.
   *
   *  Input:  dst - Target string.  To accomodate the worst case, this
   *                buffer should be a minimum of 1 + (4 x len) bytes long.
   *          src - Source string.
   *          len - Number of bytes of the source string to be converted.
   *
   *  Output: The length of the resulting string.
   *
   *  Errors: cifs_errNullInput - either the source or destination string
   *                              was NULL.
   *
   *  Notes:  This function is specific to ascii strings.  It converts all
   *          bytes except those in the range 0x20..0x7E (inclusive) to
   *          an escape string.
   *
   *        - The resulting string is *not* necessarily a correctly formatted
   *          C string.  The C standard is quirky with regard to the
   *          interpretation of escape sequences.
   *
   *          This function converts nul bytes to "\0", and other
   *          non-printing characters to a string of the form '\xhh' where
   *          'hh' is specifically two hex digits.  The C standard, in
   *          contrast, reads octal escape sequences in the form '\ooo'
   *          where 'ooo' is 1 to 3 octal digits (in the range 0..7).  Also,
   *          the C language reads hex escapes in the form '\xhh' where 'hh'
   *          is *one or more* (with no specified maximum) hex digits.
   *
   *          Consider the string { '\1', '7', '\t', '\0' }.  How would you
   *          write that in C?  Some options:
   *              GOOD                  BAD
   *            "\0017\t"             "\x017\t"
   *            "\1\x37\11"           "\17\11"
   *            "\x01\x37\t"          "\x01\7\t"
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i, j;

  if( NULL == dst || NULL == src )
    return( cifs_errNullInput );

  for( i = j = 0; i < len; i++ )
    {
    if( (src[i] < 0x20) || (0x7E < src[i]) )
      {
      dst[j++] = '\\';
      if( '\0' == src[i] )
        dst[j++] = '0';
      else
        {
        dst[j++] = 'x';
        dst[j++] = util_HexDigits[(src[i] & 0xF0) >> 4];
        dst[j++] = util_HexDigits[src[i] & 0x0F];
        }
      }
    else
      {
      dst[j++] = src[i];
      if( '\\' == src[i] )  /* Must escape the escape. */
        dst[j++] = '\\';
      }
    }
  dst[j] = '\0';

  return( j );
  } /* util_Hexify */


int util_HexDumpLn( uchar *dst, const uchar *src, int len )
  /* ------------------------------------------------------------------------ **
   * Generates one line of hexdump output from the given input.
   *
   *  Input:  dst - Destination string (minimum 68 bytes).
   *          src - Array of bytes to be dumped.
   *          len - Length of the array (maximum number of bytes to dump).
   *
   *  Output: The number of bytes dumped (which is max( len, 16 )).
   *
   *  Notes:  This function builds a string that is at most 67 bytes long,
   *          plus one byte for the nul terminator.  There is no newline
   *          character.
   *
   *          The string resulting is typical of hex dump output (e.g., from
   *          Wireshark).  The input octets are given in hex notation
   *          followed by dot/character notation (dots for non-printing
   *          characters).
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i, j;
  int maxbytes = (len > 16) ? 16 : len;

  for( i = j = 0; i < 16; i++ )
    {
    if( i < len )
      {
      dst[j++] = util_HexDigits[hiNibble( src[i] )];
      dst[j++] = util_HexDigits[loNibble( src[i] )];
      dst[j++] = ' ';
      }
    else
      {
      strcat( (char *)dst, "   " );
      j += 3;
      }
    if( 7 == i )
      dst[j++] = ' ';
    }

  for( i = 0; i < maxbytes; i++ )
    {
    dst[j++] = (isprint( src[i] )) ? src[i] : '.';
    }

  dst[j] = '\0';

  return( maxbytes );
  } /* util_HexDumpLn */


/* ========================================================================== */
