#ifndef PLATFORM_H
#define PLATFORM_H
/* ========================================================================== **
 *                                 platform.h
 *
 * Copyright:
 *  Copyright (C) 2001-2004 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: platform.h,v 0.8 2004/06/02 22:09:44 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *
 *  This is the system-specific header file for SGI Irix using the SGI C
 *  compiler.
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

/* Instead of <stdint.h>, my Indy has <inttypes.h>.
 */

#define NO_STDINT_H
#include <inttypes.h>


/* I can't find <stdbool.h> on my Indy, nor can I find a general-purpose
 * boolean type defined in the Irix C headers, so here's the typedef.
 */

#define NO_STDBOOL_H
typedef enum { false = 0, true = 1 } bool;


/* Older version of the SGI C headers did not define socklen_t.  Following
 * a system upgrade, my Indy started recognizing this type.  Magic.
 * Look through sys/socket.h on your SGI box to see if it's defined.
 * If you get errors saying that socklen_t is not defined, uncomment
 * this typedef:
 */

/*
typedef int socklen_t;
*/


/* Debugging macros.
 *
 * __FILE__ and __LINE__ are specified in K&R Second Edition (ANSI C).
 * SGI does not appear to offer anything for reporting function name
 * (no __FUNC__ or __FUNCTION__ macro).
 */

#define dbg_FILENAME (__FILE__)  
#define dbg_LINENO   (__LINE__)


/* ========================================================================== */
#endif /* PLATFORM_H */
