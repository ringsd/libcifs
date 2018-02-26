/* ========================================================================== **
 *
 *                                   Parse.c
 *
 * Copyright:
 *  Copyright (C) 2001-2002 by Christopher R. Hertel
 *
 * Email: crh@ubiqx.mn.org
 *
 * $Id: Parse.c,v 0.9 2002/12/06 06:32:12 crh Exp $
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

#include <ctype.h>                /* We use toupper().                     */
#include <string.h>               /* strncasecmp() and strcspn() are used. */
#include "SMB/URL/Parse.h"        /* Module header.                        */
#include "util/HexOct.h"          /* Support for hex encode/decode.        */


/* -------------------------------------------------------------------------- **
 * Typedefs:
 *
 *  keymap  - structure used to map smb_urlCTXToken tokens to the appropriate
 *            name in string form.  An array of keymaps (CTX_name) is
 *            defined below.  The mapping is two-way.
 */

typedef struct
  {
  char           *name;
  smb_urlCTXToken token;
  } keymap;


/* -------------------------------------------------------------------------- **
 * Static Constants:
 *
 * CTX_name - These must be kept in sync with the smb_urlCTXToken enum
 *            type.  This is an array of strings matching the Context
 *            tokens, defined in the header file.  The array is used
 *            to map between the tokens and their string values.
 *            Note that the initial set of entries is in smb_urlCTXToken
 *            order, so CTX_name[<token>].name will return the preferred
 *            name for the token.
 */

static const keymap CTX_name[] =
  {
    { "NBNS",      smb_urlCTX_NBNS },
    { "CALLED",    smb_urlCTX_CALLED },
    { "CALLING",   smb_urlCTX_CALLING },
    { "WORKGROUP", smb_urlCTX_WORKGROUP },
    { "BROADCAST", smb_urlCTX_BROADCAST },
    { "NODETYPE",  smb_urlCTX_NODETYPE },
    { "SCOPEID",   smb_urlCTX_SCOPEID },
    /* Aliases below. */
    { "WINS",      smb_urlCTX_NBNS },
    { "NTDOMAIN",  smb_urlCTX_WORKGROUP },
    { "TYPE",      smb_urlCTX_NODETYPE },
    { "SCOPE",     smb_urlCTX_SCOPEID },
    { NULL,        smb_urlCTX_MAX }
  };


/* -------------------------------------------------------------------------- **
 * Static Functions:
 */

static smb_urlCTXToken FindKey( const char *keyname )
  /* ------------------------------------------------------------------------ **
   * Use the CTX_name array to match a key, in string form, to it's correct
   * token.
   *
   *  Input:  keyname - A string which, we hope, matches an NBT context key
   *                    name.
   *
   *  Output: A token value.
   *          If there is no match, then smb_urlCTX_MAX is returned.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int i;

  for( i = 0; NULL != CTX_name[i].name; i++ )
    {
    if( 0 == strcasecmp( CTX_name[i].name, keyname ) )
      return( CTX_name[i].token );
    }
  return( smb_urlCTX_MAX );
  } /* FindKey */


/* -------------------------------------------------------------------------- **
 * Functions:
 */

int smb_urlParse( char *src, smb_urlList list )
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
   *  Errors: None Defined.
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
  {
  int   i;
  int   count = 0;
  char *p;

  /* Initialize the parse field list.
   */
  for( i = 0; i < smb_urlTK_MAX; i++ )
    list[i] = NULL;

  /* It's okay if the source is NULL.
   * We just can't parse anything if it is.
   */
  if( NULL == src )
    return( 0 );

  /* We are treating the "smb:" protocol identifier as optional at this
   * level of the code.  Since this function is an smb URL parser, we
   * figure folks are talking SMB.  A GUI application will likely fix
   * (that is, correct or repair) the user's (invalid) input anyway,
   * just as Netscape does for "http://"
   */
  p = NULL;
  if( 0 == strncasecmp( "smb:", src, 4 ) )
    p = src + 3;                /* Skip leading "smb" if found.  */
  else
    if( 0 == strncasecmp( "cifs:", src, 5 ) )
      p = src + 4;              /* Skip leading "cifs" if found. */
  if( NULL != p )
    {
    *p = '\0';
    list[smb_urlTK_SCHEME] = src;
    src = p+1;
    count++;
    }

  /* We have skipped the "smb:" prefix, if it exists.  We are now looking
   * for the double slashes ("//").  These are also optional.
   */
  if( 0 == strncmp( "//", src, 2 ) )
    src += 2;

  /* If there is nothing left, then there are no fields to parse.
   * That would mean that the job is finished.
   */
  if( '\0' == *src )
    return( count );

  /* Next thing to do is to locate the '?' delimiter, if there is one.
   * The '?' delimiter separates the URI query section from the rest of
   * the URL string.  In the SMB URL, the URI query section is used to
   * provide NBT context and, so, is called the "Context" field.
   * According to RFC 2396, the '?' character is reserved and should not
   * be used within the fields of the URL unless it is escaped (%3F).
   * Still, we will be 'careful' and look for the last '?' in the string.
   */
  p = strrchr( src, '?' );
  if( NULL != p )
    {
    *p = '\0';                          /* Replace the '?' with nul. */
    list[smb_urlTK_CONTEXT] = ++p;      /* Set context pointer.      */
    count++;
    }

  /* If there is a single slash in the remaining string, then we can divide
   * the string into two pieces based on that slash.  The first part will be
   * the server access string, and the second will be the share/pathname
   * string.
   *
   * If there is no single slash, or if the single slash is followed by a
   * nul, then the share/pathname string does not exist and we don't have
   * to parse it.
   *
   * If there *is* a single slash, and it *is* followed by a non-nul byte,
   * then the share/pathname string exists and will be parsed here.
   */
  i = (int)strcspn( src, "/" );
  if( '/' == src[i] )
    {
    src[i] = '\0';  /* Terminate the first half: the server access string. */
    i++;            /* <i> marks the start of the share/pathname string.   */

    /* Parse the share/path/file string first.  It starts at src[i]. */
    if( '\0' != src[i] )  /* The share/pathname string exists. */
      {
      char *sharepath;

      sharepath = list[smb_urlTK_SHARE] = &(src[i]);
      count++;

      /* Find the next '/' character; the remainder is the pathname.
       */
      i = (int)strcspn( sharepath, "/" );
      if( '/' == sharepath[i] )
        {
        sharepath[i] = '\0';
        i++;
        if( '\0' != sharepath[i] )
          {
          list[smb_urlTK_PATHNAME] = &(sharepath[i]);
          count++;
          }
        }
      }
    }

  /* All that should be left of src is the server access string.
   * That is, the optional authentication info, the host name/IP, and
   * optional port number:
   *
   *		[[[ntdomain;]user[:password]@]host[:port]]
   *
   * Start by separating the host name from the authentication
   * information.
   */
  i = (int)strcspn( src, "@" );
  if( '@' == src[i] )
    {
    /* Got auth info. */
    list[smb_urlTK_HOST] = &(src[i+1]);  /* Host name follows auth info. */
    src[i] = '\0';
    count++;

    /* Okay, we have the auth string:  [ntdomain;]user[:password]
     * Pull out ntdomain, user, and password.  Password first.
     */
    i = (int)strcspn( src, ":" );
    if( ':' == src[i] )
      {
      list[smb_urlTK_PASSWORD] = &(src[i+1]);
      src[i] = '\0';
      count++;
      }

    /* [ntdomain;]user remain.  Separate the NTDomain and the user fields.
     */
    i = (int)strcspn( src, ";" );
    if( ';' == src[i] )
      {
      list[smb_urlTK_NTDOMAIN] = src;
      list[smb_urlTK_USER] = &(src[i+1]);
      src[i] = '\0';
      count += 2;
      }
    else
      {
      list[smb_urlTK_USER] = src;
      count++;
      }
    }
  else
    {
    /* If we found no '@', then there is no auth info.
     */
    list[smb_urlTK_HOST] = src;
    count++;
    }

  /* Final bit is to separate the port number (if any) from the host name.
   * Thing is, the separator is a colon (':').  The colon may also exist
   * in the host portion if the host is specified as an IPv6 address (see
   * RFC 2732).  If that's the case, then we need to skip past the IPv6
   * address, which should be contained within square brackets ('[',']').
   */
  p = strchr( list[smb_urlTK_HOST], '[' );      /* Look for '['.            */
  if( NULL != p )                               /* If found, look for ']'.  */
    p = strchr( p, ']' );
  if( p == NULL )                               /* If '['..']' not found,   */
    p = list[smb_urlTK_HOST];                   /* scan the whole string.   */

  /* Starting at <p>, which is either the start of the host substring
   * or the end of the IPv6 address, find the last colon character.
   */
  p = strchr( p, ':' );
  if( NULL != p )
    {
    *p = '\0';
    list[smb_urlTK_PORT] = p + 1;
    count++;
    }

  return( count );
  } /* smb_urlParse */


int smb_urlContext( char *src, smb_urlNBT_CTX context )
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
  {
  int   i;
  int   count       = 0;
  int   dupecount   = 0;
  int   badkeycount = 0;
  char *p, *eq;
  smb_urlCTXToken ctx_tok;

  /* Initialize the parse field list.
   */
  for( i = 0; i < smb_urlCTX_MAX; i++ )
    context[i] = NULL;

  do
    {
    /* Find the end of the current {key,value} pair
     * and chop it off.
     */
    p = strchr( src, ';' );
    if( NULL != p )
      {
      *p = '\0';
      p++;
      }

    /* Find the equal sign.
     * If there is none, then we'll need to return a warning.
     */
    eq = strchr( src, '=' );
    if( NULL != eq )
      {
      *eq = '\0';       /* Terminate...       */
      eq++;             /* ...and move on.    */
      if( '\0' == *eq ) /* If it's the end,   */
        eq = NULL;      /* then it's the end. */
      }

    /* Store the value with the key.
     * Catch duplicates and unknown keys.
     */
    ctx_tok = FindKey( src );
    if( ctx_tok != smb_urlCTX_MAX )
      {
      if( NULL != context[ctx_tok] )
        dupecount++;
      else
        count++;
      context[ctx_tok] = eq;
      }
    else
      badkeycount++;

    src = p;
    } while( NULL != src );

  /* Do our best to return a useful warning message, if needed.
   */
  if( badkeycount > 0 )
    return( cifs_warnUnknownKey );
  if( dupecount > 0 )
    return( cifs_warnDuplicateKey );
  return( count );
  } /* smb_urlContext */


char *smb_urlCTX_Key_Name( int tok )
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
  {
  if( tok < 0 || tok >= smb_urlCTX_MAX )
    return( "Unknown" );
  return( CTX_name[tok].name );
  } /* smb_urlCTX_Key_Name */

/* ========================================================================== */
