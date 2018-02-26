/* ========================================================================== **
 *                                 L1Decode.c
 *
 * Copyright:
 *  Copyright (C) 2000, 2001, 2004 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: L1Decode.c,v 0.11 2004/05/30 19:37:38 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 * Description:
 *  Decodes an NBT level one encoded NetBIOS name.
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
  "Usage:\t%s <name>",
  "\t<name> == NBT name, in L1 encoded form.\n",
  "\tFor example:",
  "\t$ L1Decode CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
  "\tCKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ==> * <00>\n",
  "\t$ L1Decode FFECEJFBFICACACACACACACACACACABN",
  "\tFFECEJFBFICACACACACACACACACACABN ==> UBIQX <1d>",
  NULL
  };


/* -------------------------------------------------------------------------- **
 * Functions...
 */

int main( int argc, char *argv[] )
  /* ------------------------------------------------------------------------ **
   * Command-line utility to translate an Level-one encoded NetBIOS name
   * into a more readable form.
   *
   *  Input:  argc  - argument count.  We are expecting a value of 2.
   *          argv  - The 0th entry is the program name, as collected from
   *                  the command line.  The 1st entry should be the string
   *                  to translate.
   *
   *  Output: EXIT_SUCCESS on success, EXIT_FAILURE on error.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int   result;
  uchar suffix;
  uchar decoded[16];
  uchar hexified[64];

  /* Make sure we have something close to the right number of parameters. */
  /* Check for "-?" or -h as the only parameter. */
  if( (argc != 2)
   || (strncmp( "-?", argv[1], 2 ) == 0)
   || (strncmp( "-h", argv[1], 2 ) == 0) )
    {
    (void)util_Usage( stdout, helpmsg, argv[0] );
    exit( EXIT_SUCCESS );
    }

  /* Decode the name, then check the results. */
  result = nbt_L1Decode( decoded, (uchar *)argv[1], 0, ' ', &suffix );
  if( result < 0 )
    {
    switch( result )
      {
      case cifs_errBadL1Value:
        Fail( "Invalid character in encoded name.\n" );
      default:
        Fail( "Unknown error %d decoding name %s.\n", result, argv[1] );
      }
    }

  (void)util_Hexify( hexified, decoded, strlen( (char *)decoded ) );
  Say( "%s ==> %s <%.2x>\n", argv[1], (char *)hexified, suffix );

  return( EXIT_SUCCESS );
  } /* main */
