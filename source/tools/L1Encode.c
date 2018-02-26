/* ========================================================================== **
 *                                 L1Encode.c
 *
 * Copyright:
 *  Copyright (C) 2000, 2001, 2004 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: L1Encode.c,v 0.10 2004/05/30 19:37:38 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  Encodes strings into NBT Level 1 encoded format.
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
 *  A simple program to read a NetBIOS name and optional padding character
 *  and suffix from the command line and convert to an encoded NBT name using
 *  First Level Encoding.
 *
 * ========================================================================== **
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "cifs.h"


/* -------------------------------------------------------------------------- **
 * Static Variables...
 *
 *  helpmsg - An array of strings, terminated by a NULL pointer value.
 *            This is the text, line by line, of the help message that is
 *            displayed when util_Usage() is called.
 */

static const char *helpmsg[] =
  {
  "Usage: %s <name> [<pad> [<suffix>]]",
  "\t<name>   == NetBIOS name to translate using L1 encoding.",
  "\t<pad>    == Padding character to use (default \" \" (space)).",
  "\t<suffix> == Suffix byte.  Enter as numeric value (default \"\\0\").",
  "The <pad> and <suffix> values are read as strings, and the first",
  "character of the string is used.  If the first two characters of",
  "the string are \"\\x\" or \"0x\", then the next two characters are",
  "expected to be hex digits and will be translated accordingly.",
  "",
  "EG:",
  "$ L1Encode \\* \\\\x0 \\\\x0",
  "CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
  "$ L1Encode ubiqx \" \" \"\\x1D\"",
  "FFECEJFBFICACACACACACACACACACABN",
  "",
  NULL
  };


/* -------------------------------------------------------------------------- **
 * Functions...
 */

int main( int argc, char *argv[] )
  /* ------------------------------------------------------------------------ **
   * Level-one encode a NetBIOS name and display the wire format of the
   * name itself (not the full NBT name).
   *
   *  Input:  argc  - argument count.  We are expecting 2, 3, or 4.
   *          argv  - argv[0] is the program name.
   *                  argv[1] is the name to be encoded.
   *                  argv[2] is the optional padding byte.
   *                  argv[3] is the optional suffix byte.
   *
   *  Output: EXIT_SUCCESS on success, EXIT_FAILURE on failure.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int         tmp;
  nbt_NameRec namerec[1] = { { 0, NULL, ' ', '\0', NULL } };
  nbt_Name bufr;

  /* Make sure we have something close to the right number of parameters. */
  /* Check for "-?" or -h as the only parameter. */
  if( (argc < 2 || argc > 4)
   || (strncmp( "-?", argv[1], 2 ) == 0)
   || (strncmp( "-h", argv[1], 2 ) == 0) )
    {
    (void)util_Usage( stdout, helpmsg, argv[0] );
    exit( EXIT_SUCCESS );
    }

  /* Capture parameter values. */
  namerec->name    = (uchar *)strdup( argv[1] );
  namerec->namelen = util_UnEscStr( namerec->name );

  if( argc > 2 )
    {
    if( (tmp = util_XlateInput( argv[2] )) < 0 )
      Fail( "Invalid padding character expression: %s\n", argv[2] );
    namerec->pad = (uchar)tmp;
    }
  if( argc > 3 )
    {
    if( (tmp = util_XlateInput( argv[3] )) < 0 )
      Fail( "Invalid suffix byte expression: %s\n", argv[3] );
    namerec->sfx = (uchar)tmp;
    }

  /* Check for errors and warnings. */
  switch( nbt_CheckNbName( namerec->name, namerec->namelen ) )
    {
    case cifs_errNullInput:
      Fail( "Internal Error: NULL parameter.\n" );
      break;
    case cifs_errNameTooLong:
      Fail( "Syntax Error: NetBIOS Name too long.\n" );
      break;
    case cifs_warnContainsDot:
      Warn( "NetBIOS Name contains a dot ('.').\n" );
      break;
    }

  /* Do the work. */
  (void)nbt_UpCaseStr( namerec->name, NULL, namerec->namelen );
  (void)nbt_L1Encode( bufr, namerec );

  Say( "%s\n", (char *)bufr );

  return( EXIT_SUCCESS );
  } /* main */
