#ifndef UTIL_MSGOUT_H
#define UTIL_MSGOUT_H
/* ========================================================================== **
 *                                  MsgOut.h
 *
 *  Copyright (C) 2002-2004 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: MsgOut.h,v 0.6 2004/10/06 04:26:03 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 * Description:
 *  Get the message out!
 *
 * -------------------------------------------------------------------------- **
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
 *  - The Say() macro and the Fail(), Warn(), Info(), and Err() functions
 *    are intended to reduce typing.  They are cheap short-cuts that I like
 *    to use.  I realize that, until you know what they do, they obfuscate
 *    the code somewhat.  Sorry.  It's just that it makes it *so* much easier
 *    to write this stuff.  Besides, the whole point of writing functions is
 *    to encapsulate repatative tasks.
 *
 *  - The Say() macro and the Fail(), Warn(), Info(), and Err() functions
 *    do not follow the usual naming conventions.  Sorry again.
 *
 *  - The util_Usage function is geared specifically toward dumping a "usage"
 *    message to stdout (or stderr, or where-ever).  A more general-purpose
 *    version could (and probably should) be written some time, if there's a
 *    need.
 *
 * ========================================================================== **
 */

#include <stdio.h>        /* Standard I/O library.             */
#include <stdarg.h>       /* Variable argument lists.          */
#include "cifs_common.h"  /* CIFS library common include file. */


/* -------------------------------------------------------------------------- **
 * Macros:
 *
 *  Say       - Why do I do this?  Isn't printf() good enough?
 *              The printf() function returns an integer.  To be pedantic
 *              (which I am) the "correct" thing to do is to write
 *              "(void)printf" if the retuned value is to be ignored.
 *              That's ...what... 12 characters?
 *              That eats too much space on a line.  "Say" is shorter.
 *              Half the length of "printf" and 1/4 the length of
 *              "(void)printf".
 *              Yeah, I know it's dorky.
 */

#define Say (void)printf


/* -------------------------------------------------------------------------- **
 * Functions:
 */

void Fail( char *fmt, ... );
  /* ------------------------------------------------------------------------ **
   * Format and print a failure message on <stderr>, then exit the process.
   *
   *  Input:  fmt - Format string, as used in printf(), etc.
   *          ... - Variable parameter list.
   *
   *  Output: none
   *
   * ------------------------------------------------------------------------ **
   */


void Warn( char *fmt, ... );
  /* ------------------------------------------------------------------------ **
   * Format and print a warning message on <stderr>, then return.
   *
   *  Input:  fmt - Format string, as used in printf(), etc.
   *          ... - Variable parameter list.
   *
   *  Output: none
   *
   *  Notes:  Adds the prefix "Warning: " to the input.
   *
   *  See Also: <Err()>
   *
   * ------------------------------------------------------------------------ **
   */


void Info( char *fmt, ... );
  /* ------------------------------------------------------------------------ **
   * Format and print an informative message on <stderr>, then return.
   *
   *  Input:  fmt - Format string, as used in printf(), etc.
   *          ... - Variable parameter list.
   *
   *  Output: none
   *
   *  Notes:  Adds the prefix "Info: " to the input.
   *
   *  See Also: <Err()>
   *
   * ------------------------------------------------------------------------ **
   */


void Unk( int err_code, char *fmt, ... );
  /* ------------------------------------------------------------------------ **
   * Format and print an error message based on a libcifs error code.
   * Print to <stderr>, then return.
   *
   *  Input:  err_code  - An error code from ../cifs_errors.h
   *          fmt       - Format string, as used in printf(), etc.
   *          ...       - Variable parameter list.
   *
   *  Output: none
   *
   *  Notes:  Adds a prefix to the input.
   *
   *          At some point I should create text renditions of all of the
   *          error code names so that they can be printed along with the
   *          number.
   *
   * ------------------------------------------------------------------------ **
   */


void Err( char *fmt, ... );
  /* ------------------------------------------------------------------------ **
   * Format and print a message on <stderr>, then return.
   *
   *  Input:  fmt - Format string, as used in printf(), etc.
   *          ... - Variable parameter list.
   *
   *  Output: none
   *
   *  Notes:  Same as Warn() and Info() except that no prefix (eg. "Warning:"
   *          or "Info:" is added.
   *
   *  See Also: <Warn()>, <Info()>
   *
   * ------------------------------------------------------------------------ **
   */


int util_Usage( FILE *outf, const char *helpmsg[], char *prognam );
  /* ------------------------------------------------------------------------ **
   * Prints the the body of <helpmsg> to <outf>, where <helpmsg> is an array
   * of strings.
   *
   *  Input:  outf    - Stream to which to write.
   *                    Typically stdout or stderr.
   *          helpmsg - NULL terminated array of pointers to nul-terminated
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
   *        - This particular function is geared towards printing a help
   *          message or usage message at program startup, assuming that
   *          the user requested help or that they provided invalid input.
   *          A more general-purpose version could be written, at the
   *          expense of simplicity.
   *
   * ------------------------------------------------------------------------ **
   */


/* ========================================================================== */
#endif /* UTIL_MSGOUT_H */

