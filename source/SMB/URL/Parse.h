#ifndef SMB_URL_PARSE_H
#define SMB_URL_PARSE_H
/* ========================================================================== **
 *
 *                                   Parse.h
 *
 * Copyright:
 *  Copyright (C) 2001-2002 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: Parse.h,v 0.9 2002/12/06 06:32:12 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 *
 * Description:
 *
 *  This module implements smb:// URL decoding.
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
 *  Usage: smburl <URL>, where <URL> is formatted roughly as follows:
 *
 *  scheme://[[[ntdomain;]user[:passwd]@]host[:port][/share{/path}[/file]]]
 *           [?context]
 *
 *  This string can be broken slopily into the following pieces:
 *  scheme    :== "smb" | "cifs"
 *  server    :== [[ntdomain;]user[:password]@]host[:port]
 *                ntdomain  - NT Domain for authentication.
 *                user      - Username, for authentication.
 *                password  - Password for user in domain.
 *                host      - NetBIOS or DNS name of server, or IP of server.
 *                port      - Server's TCP port number.
 *  share     :== [share]
 *                share - Server share to be accessed.
 *  pathname  :== {/path}[/file]
 *                path  - Directory path within the share.
 *                file  - Filename.
 *  context   :== param {;param}
 *                param - a key/value pair, eg. NBNS=favog.ubiqx.mn.org
 *
 *  This format is in no way official.  It was discussed an agreed upon
 *  on the samba-technical and jCIFS mailing lists.
 *  See http://jcifs.samba.org/
 *
 *  The SMB URL is intended to fit within the standard URI syntax.  It is
 *  probably better to find full-featured URL parser and use that.  This
 *  module is currently a bit of a hack.  It needs a redesign and overhaul.
 *
 * ========================================================================== **
 */

#include "SMB/smb_common.h"   /* ubiqx CIFS library generic SMB header file. */


/* -------------------------------------------------------------------------- **
 * Typedefs:
 *
 *  smb_urlToken      - A set of tokens to identify SMB URL fields.
 *  smb_urlList       - A set of pointers to parsed-out URL fields.
 *  smb_urlCTXToken   - A set of tokens to identify SMB URL context keywords.
 *  smb_urlNBT_CTX    - A set of pointers mapping keywords to string values.
 *  smb_urlCTX_Error  - A data type used for reporting batches of warnings/
 *                      errors in an SMB URL context string.
 */

typedef enum
  {
  smb_urlTK_SCHEME = 0,
  smb_urlTK_NTDOMAIN,
  smb_urlTK_USER,
  smb_urlTK_PASSWORD,
  smb_urlTK_HOST,
  smb_urlTK_PORT,
  smb_urlTK_SHARE,
  smb_urlTK_PATHNAME,
  smb_urlTK_CONTEXT,
  smb_urlTK_MAX
  } smb_urlToken;

typedef char *smb_urlList[smb_urlTK_MAX];

typedef enum
  {
  smb_urlCTX_NBNS  = 0,
  smb_urlCTX_CALLED,
  smb_urlCTX_CALLING,
  smb_urlCTX_WORKGROUP,
  smb_urlCTX_BROADCAST,
  smb_urlCTX_NODETYPE,
  smb_urlCTX_SCOPEID,
  smb_urlCTX_MAX
  } smb_urlCTXToken;

typedef char *smb_urlNBT_CTX[smb_urlCTX_MAX];


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int smb_urlParse( char *src, smb_urlList list );
  /* ------------------------------------------------------------------------ **
   * Brute-force parsing of an smb: URL string.
   *
   *  Input:  src   - URL string to be parsed.  Note that the string itself
   *                  will be edited.  If <src> is NULL, this function will
   *                  initialize <list> as an empty list.
   *          list  - An array of pointers of type (char *).  These pointers
   *                  will point to the parsed subfields within the <src>
   *                  string.
   *
   *  Output: If negative, an error code.
   *          If non-negative, the number of fields found within <src>.
   *
   *  Errors: None defined.
   *
   *  Notes:  The contents of <src> are modified by this function.  If you
   *          need an intact copy of the original URL string, use strdup()
   *          to make a copy and pass the copy.  Do not forget to free the
   *          new string's memory when you are finished using the parsed
   *          URL.
   *
   *          This function only chops the URL into pieces.  It does not
   *          validate the content of those pieces, nor does it translate
   *          any URL escapes.  Escape sequences should be translated
   *          *after* the string is parsed.
   *
   *          Remind users to use shell escapes when using the smb:// URL
   *          format at a shell prompt.  The sh family will, for example,
   *          recognize the semicolon as a command separator.
   *
   * ------------------------------------------------------------------------ **
   */


int smb_urlContext( char *src, smb_urlNBT_CTX context );
  /* ------------------------------------------------------------------------ **
   * Brute-force parsing of the SMB URL NBT context.
   *
   *  Input:  src     - The context string to be parsed.  Note that the
   *                    string itself will be edited.
   *          context - An array of pointers of type (char *).  These
   *                    pointers will, as a result of this function, point
   *                    to the parsed subfields within the <src> string.
   *
   *  Output: If negative, an error code.
   *          If non-negative, the number of context variables found in <src>.
   *
   *  Errors:
   *
   *  Notes:  The contents of <src> are modified by this function.  If you
   *          need an intact copy of the original URL string, use strdup()
   *          to make a copy and pass the copy.  Do not forget to free the
   *          new string's memory when you are finished using the parsed
   *          Context.
   *
   *          This function only chops <src> into pieces.  It does not
   *          validate the content of those pieces, nor does it translate
   *          any URL escapes.  Escape sequences should be translated
   *          *after* the string is parsed.
   *  
   *          Remind users to use shell escapes when using the smb:// URL
   *          format at a shell prompt.  The sh family will, for example,
   *          recognize the semicolon as a command separator.  The semicolon
   *          is particulary annoying as it is the delimiter used to
   *          separate elements in the context.
   *  
   *          Known context variables are:
   *            NBNS      (alias WINS)
   *            CALLED
   *            CALLING
   *            WORKGROUP (alias NTDOMAIN)
   *            BROADCAST
   *            NODETYPE  (alias TYPE)
   *            SCOPEID   (alias SCOPE)
   *          
   * ------------------------------------------------------------------------ **
   */


char *smb_urlCTX_Key_Name( int tok );
  /* ------------------------------------------------------------------------ **
   * Given a context token, return the name.
   *
   *  Input:  tok - An integer which should be one of the smb_urlCTXToken
   *                values.  If not, the string "Unknown" will be returned.
   *
   *  Output: A pointer to a string containing a context key name.
   *
   * ------------------------------------------------------------------------ **
   */


/* ========================================================================== */
#endif /* SMB_URL_PARSE_H */
