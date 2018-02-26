#ifndef UTIL_HEXOCT_H
#define UTIL_HEXOCT_H
/* ========================================================================== **
 *
 *                                  HexOct.h
 *  Copyright (C) 2002,2003 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: HexOct.h,v 0.12 2008/08/10 03:25:33 crh Exp $
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


/* -------------------------------------------------------------------------- **
 * Global constants.
 */

extern const char util_HexDigits[];


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int util_XlateOdigit( const uchar digit );
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


int util_XlateXdigit( const uchar digit );
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


uchar util_UnEscSeq( const uchar *src, int *esclen );
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


int util_XlateInput( const char *src );
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


int util_UnEscStr( uchar *str );
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


int util_Hexify( uchar *dst, const uchar *src, int len );
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

int util_HexDumpLn( uchar *dst, const uchar *src, int len );
  /* ------------------------------------------------------------------------ **
   * Generates one line of hexdump output from the given input.
   *
   *  Input:  dst - Destination string (minimum 68 bytes).
   *          src - Array of bytes to be dumped.
   *          len - Length of the array (maximum number of bytes to dump).
   *
   *  Output: The number of bytes dumped (which is max( len, 16 )).
   *
   *  Notes:  This function returns a string that is 67 bytes long, plus one
   *          byte for the nul terminator.  There is no newline character.
   *          The string is typical of hex dump output (e.g., from
   *          Wireshark).  The input octets are given in hex notation
   *          followed by dot/character notation (dots for non-printing
   *          characters).
   *
   * ------------------------------------------------------------------------ **
   */


/* ========================================================================== */
#endif /* UTIL_HEXOCT_H */
