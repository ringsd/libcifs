#ifndef PLATFORM_H
#define PLATFORM_H
/* ========================================================================== **
 *                                 platform.h
 *
 * Copyright:
 *  Copyright (C) 2001,2004 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: platform.h,v 0.5 2004/06/01 23:51:46 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *
 *  This is the default system-specific header file.
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
 *  If this header file doesn't work for you, and if there is no platform
 *  header for your platform then create a new subdirectory, copy this
 *  default platform.h file, and modify as needed.  Once you get it working
 *  send it back to me and I'll include it in the tree.  LGPL please, with
 *  your name added to the copyright list.
 *
 *  See: ../../cifs_system.h
 *
 * ========================================================================== **
 */

/* Debugging macros.
 *
 * __FILE__ and __LINE__ are specified in K&R Second Edition (ANSI C).
 * The __FUNCTION__ macro is not particularly standard, but is common.
 * If your platform doesn't have __FUNCTION__ macro, create a new platform.h
 * file for your platform and remove the definition of dbg_FUNCNAME, or
 * change it to whatever your platform supports.
 */

#define dbg_FILENAME (__FILE__)  
#define dbg_LINENO   (__LINE__)
#define dbg_FUNCNAME (__FUNCTION__)


/* ========================================================================== */
#endif /* PLATFORM_H */
