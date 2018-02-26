/* ========================================================================== **
 *                                  hexify.c
 *
 * Copyright:
 *  Copyright (C) 2002, 2004 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: hexify.c,v 0.7 2012-10-10 02:48:07 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  Quick tool to hexify strings.
 *
 * -------------------------------------------------------------------------- **
 *
 * License:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * -------------------------------------------------------------------------- **
 *
 * Notes:
 *
 *  I use this to figure out the hex sequence of WEP passwords for
 *  wireless devices that require that you enter a hex string, and
 *  for other odd stuff.
 *
 * ========================================================================== **
 */

#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- **
 * Static Global Variables:
 */

static const char *helpmsg[] =
  {
  "Usage: %s string {string...}",
  "",
  "Each string will be converted into a series of hex values, separated by",
  "colons.  For example:",
  "  $ %s foo",
  "  66:6F:6F",
  "If a stand-alone dash is the only input, the program will read from stdin:",
  "  $ echo -n \"foo\" | %s -",
  "  66:6F:6F",
  "If the first character of the first input string is a dash ('-'), and the",
  "string isn't a stand-alone dash, then the program will print this help",
  "message.  (This is a cheap way of catching -h, -?, etc.)  Bypass the help",
  "message by adding an empty string ahead of the string with the leading dash:",
  "  $ %s \"\" -foo",
  "  2D:66:6F:6F",
  "Multiple input strings will generate output on separate lines, eg.:",
  "  $ hexify foo bar",
  "  66:6F:6F",
  "  62:61:72",
  "Join the strings by using quotation marks:",
  "  $ hexify \"foo bar\"",
  "  66:6F:6F:20:62:61:72",
  NULL
  };


/* -------------------------------------------------------------------------- **
 * Functions:
 */

static void usage( const char *helpmsg[], char *prognam )
  /* ------------------------------------------------------------------------ **
   * Prints the the body of <helpmsg> to stderr.
   *
   *  Input:  helpmsg - NULL terminated array of pointers to nul-terminated
   *                    strings.
   *          prognam - An additional string, generally the name of the
   *                    program being executed, that will be used to
   *                    replace a single %s in an input string.
   *
   *  Output: The number of strings printed.
   *
   *  Notes:  A newline ('\n') will be added to each string in <helpmsg>
   *          as it is written out.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int   i;
  char *rev = "$Revision: 0.7 $";

  /* If <progname> is NULL, provide a default value.  */
  prognam = ( (NULL == prognam) ? "hexify" : prognam );

  /* Extract and print the version number.
   *  This assumes CVS or a CVS-style Revision string.
   *  Note that <i> is a state variable, not a counter.
   */
  (void)fprintf( stderr, "[hexify v" );
  for( i = 0; ('\0' != *rev) && (i < 3); rev++ )
    {
    switch( i )
      {
      case 0:
        /* 0: Initial state.  Transition to 1 on ':'. */
        if( ':' == *rev )
          i = 1;
        break;
      case 1:
      case 2:
        /* 1: Output permitted state.  Valid characters may be printed.
         * 2: Output happened state.  We wrote something.
         */
        if( ('.' == *rev) || (('0' <= *rev) && (*rev <= '9')) )
          {
          i = 2;
          putc( *rev, stderr );
          }
        else
          {
          /* 3: Completed writing state.  If we have written anything, and
           *    encounter a non-version character (not in [0..9,'.']), then
           *    we are done.  Otherwise, keep going.  3 is the terminating
           *    state.
           */
          i = ((2==i) ? 3 : 1);
          }
        break;
      default:
        /* Should never happen, but we need to keep the compiler happy. */
        break;
      }
    }
  /* Complete the line. */
  (void)fprintf( stderr, "%s]\n", ((i < 2) ? "0.0" : "") );

  /* Now dump the help string.  */
  for( i = 0; NULL != helpmsg[i]; i++ )
    {
    (void)fprintf( stderr, helpmsg[i], prognam );
    (void)fputc( '\n', stderr );
    }
  } /* usage */


int main( int argc, char *argv[] )
  /* ------------------------------------------------------------------------ **
   * Quick program to convert strings into hex sequences.
   *
   *  Input:  argc  - number of argv[] entries.
   *          argv  - array of pointers to strings.
   *
   *  Output: EXIT_FAILURE on error, else EXIT_SUCCESS.
   *
   *  Notes:  Geez, these are lame comments.  :)
   *          See the help message for usage.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int            i, j;
  unsigned char *s;

  /* If the only command-line input is a solitary dash, we hexify stdin
   * and then exit.
   */
  if( (2 == argc) && ('-' == argv[1][0]) && ('\0' == argv[1][1]) )
    {
    for( i = 0; EOF != (j = getchar()); i = 1 )
      (void)printf( "%s%.2X", i?":":"", j );
    if( i )
      putchar( '\n' );
    exit( EXIT_SUCCESS );
    }

  /* If we have no input parameters, or if the first parameter starts
   * with a dash, then dump help and exit.
   */
  if( argc < 2 || ('-' == *argv[1] ) )
    {
    usage( helpmsg, argv[0] );
    exit( EXIT_FAILURE );
    }

  /* If we've avoided all of the special cases, just hexify all of the
   * strings on the command line.
   */
  for( i = 1; i < argc; i++ )
    {
    s = (unsigned char *)argv[i];
    for( j = 0; '\0' != s[j]; j++ )
      (void)printf( "%s%.2X", (j ? ":" : ""), s[j] );
    if( j )
      putchar( '\n' );
    }

  return( EXIT_SUCCESS );
  } /* main */

/* ========================================================================== */
