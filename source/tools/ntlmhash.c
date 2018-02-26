/* ========================================================================== **
 *                                 ntlmhash.c
 *
 *  Copyright (C) 2007 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: ntlmhash.c,v 0.1 2007/11/06 21:13:10 crh Exp $
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
 * Description:
 *
 *  This program generates the LM and NTLM hashes from a given cleartext
 *  password input.
 *
 * Bugs:
 *
 *  The program should accept Unicode input and handle it in UCS-2LE format.
 *  Instead, it takes ASCII input and pads the string with nul bytes to
 *  approximate UCS-2LE.  Fudge.
 *
 * Compile:
 *
 * This program uses the MsgOut module in the libcifs util/ directory, as
 * well as the LMhash, DES, and MD4 modules in the Auth/ directory. 
 *
 * $ cc -I ../ -o ntlmhash ntlmhash.c ../util/MsgOut.c ../Auth/LMhash.c \
 *   ../Auth/DES.c ../Auth/MD4.c
 *
 * ========================================================================== **
 */

#include <stdio.h>      /* Standard I/O.     */
#include <stdlib.h>     /* Standard C stuff. */
#include <poll.h>       /* Check for waiting input. */
#include <ctype.h>      /* For toupper(3).          */

#include "cifs.h"       /* CIFS toolkit header.     */


/* -------------------------------------------------------------------------- **
 * Constants:
 *
 *  bSIZE - size to be used when creating generic, utilitarian buffers.
 */

#define bSIZE 1024
    

/* -------------------------------------------------------------------------- **
 * Static Variables:
 *  helpmsg - An array of strings, terminated by a NULL pointer value.
 * 
 *  Copyright - Copyright string.  It's a difficult world...
 *  License   - License under which the software is released.
 *  ID        - Long-hand string providing revision information.
 */

static const char *helpmsg[] =
  {
  "",
  "Usage: %s [-h|-V]",
  "  This program will prompt for a cleartext password (which will be read",
  "  from standard input), and produce both the LM and NTLM hashes of that",
  "  password.",
  "  ",
  "  -h : Causes this message to be displayed then exits the program.",
  "  -V : Displays version and license information, then exits.",
  "",
  NULL
  };

static const char *Copyright = "Copyright (c) 2007 by Christopher R. Hertel";
static const char *License   = "GNU General Public License Version 2 or Later";
static const char *ID        = "$Id: ntlmhash.c,v 0.1 2007/11/06 21:13:10 crh Exp $";


/* -------------------------------------------------------------------------- **
 * Static Functions...
 */

static void usage( char *prognam, int status )
  /* ------------------------------------------------------------------------ **
   * Prints the usage message, then exits with the given <status>.
   *
   *  Input:  prognam - The name of the program (via argv[0]).
   *          status  - Exit status (typically EXIT_SUCCESS or EXIT_FAILURE).
   *
   *  Output: <none>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  (void)util_Usage( stderr, helpmsg, prognam );
  exit( status );
  } /* usage */


static void version( char *prognam, int status )
  /* ------------------------------------------------------------------------ **
   * Print version and license information, the bail out.
   *
   *  Input:  prognam - The name of the program (via argv[0]).
   *          status  - Exit status (typically EXIT_SUCCESS or EXIT_FAILURE).
   *
   *  Output: <none>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  Err( "%s: %s\n", prognam, ID );
  Err( " License: %s\n", License );
  Err( "%s\n\n", Copyright );
  exit( status );
  } /* version */


static bool anyInput( FILE *stream )
  /* ------------------------------------------------------------------------ **
   * Quick check for availability of input on a stream.
   *
   *  Input:  stream  - File stream to check.
   *
   *  Output: Returns true if there is output waiting, else false.
   *
   *  Notes:  We'll wait up to 0.05 second for input, just in case it's
   *          slow to arrive.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int           result;
  struct pollfd fds[1];

  fds[0].fd      = fileno( stream );
  fds[0].events  = POLLIN;
  result = poll( fds, 1, 50 );

  if( result > 0 )
    return( true );
  return( false );
  } /* anyInput */


/* -------------------------------------------------------------------------- **
 * Functions...
 */

int main( int argc, char *argv[] )
  /* ------------------------------------------------------------------------ **
   * Mainline
   *
   *  Input:  argc  - You know what this is.
   *          argv  - You know what to do.
   *
   *  Output: EXIT_SUCCESS in most cases, or EXIT_FAILURE if the user needs
   *          some help.
   *
   *  Notes:  Input is taken from <stdin> so that you can pipe lines of text
   *          to the program.  The input is read using fgets(3), so there
   *          are some limitations to the values that can be provided (in
   *          particular, nul bytes or newline characters will terminate
   *          the input.
   *
   *          The LM and NTLM hash are both generated, and then printed in
   *          hex string format for the user to read.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  uchar iBufr[bSIZE];
  uchar oBufr[bSIZE];
  int   len;
  int   max;
  int   i;

  /* Very simple command-line parsing. */
  if( argc > 1 )
    {
    if( (argc > 2) || ('-' != argv[1][0]) )
      usage( argv[0], EXIT_FAILURE );
    if( 'V' == argv[1][1] )
      version( argv[0], EXIT_SUCCESS );
    usage( argv[0], EXIT_FAILURE );
    }

  /* Prompt for and then read the input. */
  if( !anyInput( stdin ) )
    Say( "Plaintext: " );
  len = strlen( fgets( (char *)iBufr, bSIZE, stdin ) );
  if( '\n' == iBufr[len-1] )
    iBufr[--len] = '\0';

  /* Generate the LM Hash
   * - First we need to convert to upper case.  We'll copy the 14 max bytes
   *   to the output buffer <oBufr> converting them as we go.
   * - Then call auth_LMhash() from the ../Auth/LMhash module.  The buffer
   *   can be safely overwritten so we can use <oBufr> as both input and
   *   output.
   */
  max = (len > 14) ? 14 : len;
  for( i = 0; i < max; i++ )
    oBufr[i] = toupper( iBufr[i] );
  (void)auth_LMhash( oBufr, oBufr, max );

  Say( "  LM Hash [" );
  for( i = 0; i < 16; i++ )
    Say( "%s%.2x", i?":":"", (unsigned char)oBufr[i] );
  Say( "]\n" );

  /* Generate the NTLM Hash
   * - First step is to convert the input <iBufr> to Unicode.
   *   *Cheat!* This program simply places nul bytes following
   *   each byte of the input password.  Ick!  *Not* real UCS-2LE conversion.
   * - Next, we perform an MD4 on the newly expanded password.
   */
  max = (len > (bSIZE/2) ? (bSIZE/2) : len);
  iBufr[(2*max)-1] = '\0';
  for( i = max-1; i > 0; i-- )
    {
    iBufr[(i*2)]   = iBufr[i];
    iBufr[(i*2)-1] = '\0';
    }
  (void)auth_md4Sum( oBufr, iBufr, 2*max );

  Say( "NTLM Hash [" );
  for( i = 0; i < 16; i++ )
    Say( "%s%.2x", i?":":"", oBufr[i] );
  Say( "]\n" );

  return( EXIT_SUCCESS );
  } /* main */

/* ========================================================================== */

