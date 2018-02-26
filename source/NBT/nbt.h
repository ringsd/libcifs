#ifndef NBT_H
#define NBT_H
/* ========================================================================== **
 *
 *                                   nbt.h
 *
 * Copyright:
 *  Copyright (C) 2001,2004 by Christopher R. Hertel
 *
 * Email:
 *  crh@ubiqx.mn.org
 *
 * $Id: nbt.h,v 0.7 2004/05/30 04:19:50 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *  This is the global header file for the NBT subsystem.  Including this
 *  header will include all of the following:
 *    libcifs/NBT/?*.h
 *    libcifs/NBT/NS/?*.h
 *    libcifs/NBT/DS/?*.h
 *    libcifs/NBT/SS/?*.h
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
 *  The purpose of this file is to offer one-stop shopping.  Simply include
 *  this file and all of the headers for the NBT subsystem will be included
 *  for you.  There is no run-time penalty for including everything.
 *
 * ========================================================================== **
 */

#include "NBT/Names.h"        /* NetBIOS names in the NBT context.         */
#include "NBT/NS/nbt_ns.h"    /* NBT Name Service global header.           */
#include "NBT/DS/nbt_ds.h"    /* NBT Datagram Service global header.       */
#include "NBT/SS/nbt_ss.h"    /* NBT Session Service global header.        */


/* ========================================================================== */
#endif /* NBT_H */
