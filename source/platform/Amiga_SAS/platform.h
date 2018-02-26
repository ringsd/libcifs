#ifndef PLATFORM_H
#define PLATFORM_H
/* ========================================================================== **
 *                                 platform.h
 *
 * Copyright:
 *  Copyright (C) 2001, 2004 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: platform.h,v 0.6 2004/05/30 23:38:26 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *
 *  This is the system-specific header file for Amiga systems using SAS C.
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
 *  See: ../../cifs_system.h
 *
 * ========================================================================== **
 */

/* SAS C does not appear to have stdint.h and stdbool.h.  There may be some
 * other header that provides these types.  If so, please let me know.
 */

#define NO_STDINT_H
#define NO_STDBOOL_H

typedef signed char             int8_t;       /* Signed byte.      */
typedef signed short int        int16_t;      /* Signed short.     */
typedef signed long int         int32_t;      /* Signed long.      */

typedef unsigned char           uint8_t;      /* Unsigned byte.    */
typedef unsigned short int      uint16_t;     /* Unsigned short.   */
typedef unsigned long int       uint32_t;     /* Unsigned long.    */

typedef enum { false = 0, true  = 1 } bool;   /* C99 Boolean type. */


/* SAS C has strnicmp() instead of strncasecmp().
 */

#define strncasecmp( A, B, N ) strnicmp( (A), (B), (N) )


/* Debugging macros.
 *
 * __FILE__ and __LINE__ are specified in K&R Second Edition (ANSI C).
 *
 * The __FUNC__ macro is not particularly standard (GCC, for instance, uses
 *     __FUNCTION__ instead of __FUNC__).
 */

#define dbg_FILENAME (__FILE__)  
#define dbg_LINENO   (__LINE__)
#define dbg_FUNCNAME (__FUNC__)


/* ========================================================================== */
#endif /* PLATFORM_H */
