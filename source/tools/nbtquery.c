/* ========================================================================== **
 *                                 nbtquery.c
 *
 * Copyright:
 *  Copyright (C) 2001-2008, 2010 by Christopher R. Hertel
 *
 *  Email: crh@ubiqx.mn.org
 *
 *  $Id: nbtquery.c,v 0.60 2011-01-06 15:52:34 crh Exp $
 *
 * -------------------------------------------------------------------------- **
 * Description:
 *
 *  An NBT name service query tool.
 *
 * -------------------------------------------------------------------------- **
 * License:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful.
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * -------------------------------------------------------------------------- **
 * Notes:
 *
 *  This program is example code.  It was written to run on Linux, but should
 *  be fairly portable to other Unix flavors.  It should also serve as a
 *  guide to folks using the libcifs library on various platforms.
 *
 * ========================================================================== **
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/ioctl.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "cifs.h"


/* -------------------------------------------------------------------------- **
 * Constants:
 *
 *  bSIZE     - Buffer size.
 *
 *  Copyright - Copyright string.  It's a difficult world...
 *  Revision  - Short-hand string providing revision information (-V).
 *  ID        - Long-hand string providing revision information (-vV).
 *
 *  helpmsg   - An array of strings, terminated by a NULL pointer value.
 *              This is the text, line by line, of the help message output
 *              when usage() is called.
 *
 *  verbosemsg- A more detailed help message.
 */

#define bSIZE 1024

static const char *Copyright
                = "Copyright (c) 2001-2008, 2010 by Christopher R. Hertel";
static const char *Revision
                = "$Revision: 0.60 $";
static const char *ID
                = "$Id: nbtquery.c,v 0.60 2011-01-06 15:52:34 crh Exp $";

static const char *helpmsg[] =
  {
  "Name Lookup Queries:",
  "  nbtquery [-crRv][-w <w>][(-B|-U) <IP>][-p <pad>][-s <sfx>][-S <scp>] <Name>",
  "Adapter Status Queries:",
  "  nbtquery -A [-rv][-w <w>][-S <scp>] <IP>",
  "  nbtquery -a [-crv][-w <w>][(-B|-U) <IP>][-p <pad>][-s <sfx>][-S <scp>] <Name>",
  "Locate Browser nodes:",
  "  nbtquery -b [-crRv][-w <w>][(-B|-U) <IP>][-S <scp>][[-D|-L] <Name>]",
  "Options:",
  "  -c         Do not convert <Name> to upper case before sending query",
  "  -R <0|1>   Set or clear Recursion Desired (RD) (default: query dependent)",
  "  -r         Listen on port 137 (may require special privileges)",
  "  -B <IP>    Send to address <IP> with broadcast flag set",
  "  -U <IP>    Send to address <IP> with broadcast flag unset (clear)",
  "  -p <pad>   Use <pad> as the padding character (default is space, 0x20)",
  "  -s <sfx>   Use <sfx> as the suffix character (default is nul, 0x00)",
  "  -S <scp>   Append Scope ID <scp> to the NetBIOS name",
  "  -D, -L     Look for Domain Master Browser; Look for Local Master Browser",
  "  -w <w:i,r> Wait <w> ms for replies, add <i> ms per retry, max retries <r>",
  "  -v[v], -V  -v[v] = Be [very] verbose;  -V = Display Version and exit.",
  "<Name> is either an asterisk ('*') or NetBIOS name.  If '*', then the",
  "default <pad> is nul (0x00).  <IP> may be an IP address or a DNS name.",
  "",
  "For detailed information:  nbtquery -vh | more",
  NULL
  };

static const char *verbosemsg[] =
  {
  "Source code is available.  See the LibCIFS project web page:",
  "  http://ubiqx.org/libcifs/",
  "",
  "This program is an NBT testing utility.  It generates a variety of NBT",
  "Name Service query messages and reports the results.",
  "",
  "Name Lookup Queries:",
  "  nbtquery [-crRv][-w <w>][(-B|-U) <IP>][-p <pad>][-s <sfx>][-S <scp>] <Name>",
  "",
  "    The default behavior is to send a broadcast name query for <Name>,",
  "    with the 'B' bit set in the NBT Name Service message header.",
  "    The -B and -U flags are similar, except that -B forces the 'B' bit on",
  "    and -U forces the 'B' bit off.",
  "",
  "Adapter Status Queries:",
  "  nbtquery -A [-rv][-w <w>][-S <scp>] <IP>",
  "",
  "    The -A option sends a Node Status Request to the specified IP address,",
  "    using the wildcard NetBIOS name.  This is identical to:",
  "",
  "      nbtquery -a [-rv][-w <w>][-S <scp>] -U <IP> \"*\"",
  "",
  "  nbtquery -a [-crv][-w <w>][(-B|-U) <IP>][-p <pad>][-s <sfx>][-S <scp>] <Name>",
  "",
  "    The -a option causes nbtquery to send a Node Status Request using the",
  "    (required) NetBIOS name you provide.  The default behavior is to send",
  "    the query as a broadcast.",
  "    WARNING:  Many implementations will not respond to Node Status Queries",
  "              that are broadcast, regardless of the state of the 'B' bit",
  "              in the request's NBT header.",
  "",
  "    If you use Samba's nmblookup tool with the -S option, or Microsoft's",
  "    nbtstat with the -a option, you may notice that these tools attempt",
  "    name resolution first, and then send the Node Status Request as a",
  "    unicast message to the (first) IP they receive.  This tool (nbtquery)",
  "    is intended as a testing tool, and it does *not* send the initial",
  "    name query.  You need to do that yourself.",
  "",
  "    As mentioned above, the default behavior is to broadcast the Node",
  "    Status Request, and many implementations will not respond if the",
  "    request is sent to something other than their interface IP address.",
  "    So, by default, many nodes will *not* respond to 'nbtquery -a <Name>'.",
  "    To fix this, use the -U option to perform a unicast query, directed at",
  "    the node you wish to query.",
  "",
  "Locate Browser nodes:",
  "  nbtquery -b [-crRv][-w <w>][(-B|-U) <IP>][-S <scp>][[-D|-L] <Name>]",
  "",
  "    The -b option causes the program to search for browser nodes.",
  "    If no NetBIOS name is specified the program will search for the special",
  "    \\x01\\x02__MSBROWSE__\\x02\\x01 name.  (It is easier to type",
  "    'nbtquery -b' than 'nbtquery -s 01 \"\\x01\\x02__MSBROWSE__\\x02\"', which is",
  "    why the -b option exists.)  Some additional things you can do with -b:",
  "      + If a name is specified, the suffix byte will be set to 0x1E.",
  "        This results in a query for all local browsers for the named",
  "        workgroup.",
  "      + The -D option replaces 0x1E with 0x1B, which is the suffix used",
  "        by the Domain Master Browser.",
  "      + The -L option replaces 0x1E with 0x1D, which is the suffix used",
  "        by the Local Master Browser.",
  "    The command 'nbtquery -b -L ubiqx' will send a broadcast query for the",
  "    local master browser for the UBIQX workgroup.  It is equivalent to",
  "    'nbtquery -s 1D ubiqx'.  Likewise, 'nbtquery -b -D ubiqx' is equivalent",
  "    to 'nbtquery -s 1B ubiqx'.",
  "    If either -L or -D are specified, the -b option is, er, optional.  :)",
  "",
  "Options:",
  "  -c         Do not convert <Name> to upper case before sending query",
  "  -h         Print a help message.  Use -vh for verbose help.",
  "  -R <0|1>   Set the Recursion Desired (RD) flag (default: query dependent)",
  "  -r         Listen on port 137 (may require special privileges)",
  "  -B <IP>    Send to address <IP> with broadcast flag set",
  "  -U <IP>    Send to address <IP> with broadcast flag unset (clear)",
  "  -p <pad>   Use <pad> as the padding character (default is space, 0x20)",
  "  -s <sfx>   Use <sfx> as the suffix character (default is nul, 0x00)",
  "  -S <scp>   Append Scope ID <scp> to the NetBIOS name",
  "  -D, -L     Look for Domain Master Browser; Look for Local Master Browser",
  "  -w <w:i,r> Wait <w> ms for replies, add <i> ms per retry, max retries <r>",
  "  -v[v], -V  -v[v] = Be [very] verbose;  -V = Display Version and exit.",
  "<Name> is either an asterisk ('*') or NetBIOS name.  If '*', then default",
  "<pad> is nul (0x00).  <IP> may be an IP address or a DNS name.",
  "",
  "The options above allow you to modify the behavior of the program.  You can",
  "even create non-standard query messages.  Option usage is detailed below:",
  "",
  "-c        It is standard practice to convert a NetBIOS name and Scope ID",
  "          to upper case  before translating them to wire form and sending",
  "          the query.  This switch disables that behavior, leaving the",
  "          name and scope ID as you typed them.  Some NBT implementations",
  "          (Samba) decode NBT Names and perform case-insensitive string",
  "          comparisons, others (Windows) do not.  This option allows you",
  "          to distinguish between the two styles.",
  "          There are also some applications (eg. Microsoft's IIS) which",
  "          register NetBIOS names in mixed UPPER/lower case.",
  "",
  "-R <0|1>  Sets the Recursion Desired bit.",
  "          RFC1002 shows the RD bit always set in NAME QUERY REQUEST",
  "          messages.  In practice, this bit is set *except* in unicast",
  "          queries to an end node (name verification queries).  If a query",
  "          is sent to a node which is also the NBNS then:",
  "            If RD is set, the host's local name table is checked first",
  "                          followed by the NBNS database.",
  "            If RD is clear, the query is answered from the host's local",
  "                            NBT name table.",
  "          By default, RD is set in all NAME QUERY REQUEST messages sent",
  "          by this program.  Use \"-R 0\" to override this behavior when",
  "          sending a name verification query.  See:",
  "            http://ubiqx.org/cifs/NetBIOS.html#NBT.4.3.2",
  "          for a detailed discussion.",
  "",
  "          RD is always clear in NODE STATUS REQUESTs (but you can use this",
  "          flag to override standard behavior to test what happens if the",
  "          RD bit is set).",
  "",
  "-r        By default, this program will open a random, high-numbered UDP",
  "          port for sending messages.  Normally, this will work just fine.",
  "          Unfortunately, some early versions of Windows/95 had a bug which",
  "          causes them to reply to port UDP/137 no matter what the source",
  "          port of the query.  Some of those systems are still around.",
  "          This option allows you to force the nbtquery to attempt to use",
  "          port UDP/137.  If the port is in use, or the user (you) does not",
  "          have sufficient privilege to open that port, the program will",
  "          print a failure message and exit.",
  "",
  "-p <pad>  The default padding character is the space (0x20) unless the",
  "          given NetBIOS name is '*', in which case the default padding",
  "          character is the nul byte (0x00).  This option allows you to",
  "          specify a different padding byte, and override the default.",
  "",
  "-s <sfx>  The default suffix value is nul (0x00).  This option allows",
  "          you to specify a different suffix value.",
  "",
  "-w <w:i,r>  The <w> value indicates the minimum amount of time (in ms)",
  "          to wait for a reply after sending a query.  The maximum wait is",
  "          increased by <i> ms for each retry.  (1 ms = 1/1000 second).",
  "          The <r> value is the maximum number of query attempts to send.",
  "          Additional queries are only sent if all previous queries failed.",
  "",
  "          The default is: 250:250,3.  Maximum total wait is 0xFFFF ms.",
  "          That is: <w> + (<i> * (<r> - 1)) must be less than 65,535.",
  "          Individual fields may be omitted, and defaults will be used.",
  "          For example:  -w 500     == -w 500:250,3",
  "                        -w :750    == -w 250:750,3",
  "                        -w ,5      == -w 250:250,5",
  "                        -w 500,4   == -w 500:250,4",
  "                        -w 500:750 == -w 500:750,3",
  "                        -w :750,2  == -w 250:750,2",
  "          Note that repeat queries may result in multiple replies from the",
  "          same source.",
  "",
  "          The program will wait for replies at least <w> ms per query",
  "          attempt.  The minimum value allows multiple responses to be",
  "          received (eg. from a broadcast query for a group name).",
  "          The program will wait at most (<w> + ((<n>-1) * <i>)) ms per",
  "          query, where <n> is the retry number in the range 0..(<r>-1).",
  "          For example, given -w 250:150,3, the program will wait:",
  "            First query:   250 ms",
  "            Second query:  At least 250 ms, but at most 400 ms.",
  "            Third query:   At least 250 ms, but at most 550 ms.",
  "",
  "Name syntax:",
  "  NetBIOS names must be no more than 15 bytes in length.  Within that 15",
  "  bytes, the program will accept any characters that can be entered via",
  "  the command line, including escaped characters.  Place quotation marks",
  "  around the string if necessary.  You can use most 'C' escape sequences",
  "  including \\xhh hex and \\ooo octal sequences.",
  "",
  "  If the name begins with an asterisk ('*') then the default padding",
  "  character is changed to nul (0x00).  This can be overridden using the",
  "  -p option as described above.",
  "",
  "  Use the -s option for specifying the suffix.  The program does not allow",
  "  the specification of the suffix byte as part of the name string.",
  "",
  "  Example:",
  "    nbtquery -s 0x01 \"\\x01\\x02__MSBROWSE__\\x02\"",
  "",
  "  The above will send a query for the special MSBROWSE group name used",
  "  to identify local master browsers.",
  "",
  "Specifying <pad> and <sfx> bytes:",
  "  The -p and -s options each take a parameter that specifies a one-byte",
  "  value in hexadecimal notation.  Several notation variations are",
  "  supported.  All of the following examples are equivalent and all specify",
  "  a suffix value of 0x1E:",
  "    nbtquery -s 1e ubiqx",
  "    nbtquery -s 0x1E ubiqx",
  "    nbtquery -s \"\\x1e\" ubiqx",
  "    nbtquery -s \"%%1E\" ubiqx",
  "    nbtquery -s \"#1E\" ubiqx",
  "    nbtquery -s \"<1e>\" ubiqx",
  "  Note that in the all of the above forms, the input is *always* read",
  "  as a hexadecimal number.  If the number cannot be interpreted then",
  "  the program will exit with an error message.",
  "",
  "Output format:",
  "  If the -v option is *not* specified, the results of the query will be",
  "  presented in a somewhat terse format.",
  "  If the -v (verbose) option is used, more detail and additional",
  "  diagnostics are provided.",
  "  If -vv (very verbose) is given, the output will be presented in a",
  "  fully exploded format, detailing the contents of the received replies.",
  "  Very verbose output should be redirected to a file or to a pager program.",
  "  For example:",
  "    nbtquery -vv \\* | more",
  "",
  "  In any case, the program decodes and escapes NBT names.  Nul bytes in",
  "  the output are represented as '\\0', and other non-printing characters",
  "  are represented as '\\xhh', where 'hh' is exactly two hex digits.",
  ""
  "  Note that the escaped output *does not* match standard C escape syntax.",
  "  For example, the string \"UBIQX\\077\" would be interpreted by C as",
  "  \"UBIQX?\" (because \\077 == '?').  As output from this program, however,",
  "  the correct interpretation is equivalent to the C string \"UBIQX\\00077\".",
  "  (That's the concatenation of \"UBIQX\" + \"\\0\" + \"77\".)",
  "",
  "For abbreviated help, omit the -v option.",
  "To see all of the extended help, use 'nbtquery -vh | more'",
  "",
  NULL
  };


/* -------------------------------------------------------------------------- **
 * Typedefs:
 *
 *  ValName - Value to name mapping structure.
 */

typedef struct
  {
  int   val;
  char *name;
  } ValName;


/* -------------------------------------------------------------------------- **
 * Enumerated types:
 *
 *  querytype - Identifies the kind of query being attempted:
 *              NameQuery       - Simple Name Query.
 *              NodeStatusIP    - Adapter Status using wildcard name, sent
 *                                to the given IP.
 *              NodeStatusName  - Adapter Status using a NetBIOS name.
 *              BrowserFind     - Locate browser nodes.
 *              Version         - Just print version and exit.
 *
 *  trilean  - Like a boolean, but with a third option.  Yes, no, maybe.
 */

typedef enum
  {
  NameQuery = 0,
  NodeStatusIP,
  NodeStatusName,
  BrowserFind,
  Version
  } querytype;

typedef enum
  {
  t_false,
  t_true,
  t_other
  } trilean;


/* -------------------------------------------------------------------------- **
 * Static Global Variables:
 *
 *  Bcast     - Default true.  If true, the Broadcast bit will be set in the
 *              NBT Name Service header and the message will be sent as an
 *              IP broadcast.  If false, the bit will be clear and the
 *              message is sent Unicast.
 *
 *  UpCase    - Default true.  If false, do not upcase the NetBIOS name and
 *              Scope ID before encoding.
 *
 *  Port137   - Default false.  If true, attempt to use for I/O port UDP/137.
 *
 *  ForcePad  - Default false.  True if pad set on command line via -p.
 *
 *  ForceSfx  - Default false.  True if sfx set on command line via -s.
 *
 *  DMBQuery  - Default false.  Used with -b.  If true use suffix 0x1B.
 *
 *  LMBQuery  - Default false.  Used with -b.  If true use suffix 0x1D.
 *
 *  RecDes    - Default behavior is to turn on RD for name queries and
 *              clear RD for node status queries.  RD should, however,
 *              be clear for unicast name verification queries (which
 *              are unicast queries to the local name table of the
 *              target node).
 *              See:  http://ubiqx.org/cifs/NetBIOS.html#NBT.4.3.2
 *              This is a trilean type variable.  If set to t_other,
 *              then the default behavior for the query type is used.
 *
 *  NameRec   - An nbt_NameRec structure, used for converting between
 *              encoded and unencoded forms.
 *
 *  DestAddr  - Struct in_addr form of the destination address.  Generated
 *              by resolving <DestIP>.  Default is 255.255.255.255.
 *
 *  DestIP    - String form of the destination address.  Default NULL.
 *              If NULL and Bcast is true, then we send to the local
 *              broadcast address.
 *
 *  Verbose   - Verbosity level.  Default zero (0).
 *
 *  RetryWait - Number of ms to wait for a reply following the initial query.
 *
 *  RetryInc  - Number of ms to add to RetryWait before retrying each query.
 *
 *  RetryCnt  - Number of times to send the query.
 *
 *  ScopeID   - Scope ID string.  Default NULL ("" will be used if NULL).
 *
 *  QueryName - NetBIOS name to be queried.
 *
 *  OurSocket - Socket we open in order to send the query & get the response.
 *
 *  SendBufr  - Outgoing packet buffer.
 *
 *  RecvBufr  - Incoming packet buffer.
 */

static bool    Bcast      = true;
static bool    UpCase     = true;
static bool    Port137    = false;
static bool    ForcePad   = false;
static bool    ForceSfx   = false;
static bool    DMBQuery   = false;
static bool    LMBQuery   = false;

static trilean RecDes     = t_other;

nbt_NameRec    NameRec[1] = { { 0, NULL, ' ', '\0', NULL } };

struct in_addr  DestAddr  = { INADDR_BROADCAST };
static char    *DestIP    = NULL;
static char    *ScopeID   = NULL;
static char    *QueryName = NULL;
static int      Verbose   = 0;
static int      RetryWait = 250;
static int      RetryInc  = 250;
static int      RetryCnt  = 3;

static int      OurSocket = -1;

static uchar    SendBufr[bSIZE];
static uchar    RecvBufr[bSIZE];

static uint16_t TID = 0xF00D;


/* -------------------------------------------------------------------------- **
 * Functions:
 */

static void usage( char *prognam )
  /* ------------------------------------------------------------------------ **
   * Prints a usage header, then the body of <helpmsg> or <verbosemsg>.
   *
   *  Input:  prognam - Pointer to a string containing the program name.
   *  Output: none.
   *
   *  Notes:  This function calls exit(3) which closes the process.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  if( Verbose )
    {
    Say( "%s\n%s\n", Copyright, Revision );
    (void)util_Usage( stdout, verbosemsg, prognam );
    }
  else
    (void)util_Usage( stdout, helpmsg, prognam );

  exit( EXIT_FAILURE );
  } /* usage */


static char *qtoptchar( querytype qt )
  /* ------------------------------------------------------------------------ **
   * Returns a short string matching the flag value used to set a particular
   * query mode.
   * 
   *  Input:  qt  - A query type.
   *
   *  Output: A pointer to a string containing the flag value that maps to
   *          the given query type.
   *
   *  Notes:  Used to convert a query type back to a command line flag for
   *          feedback in error messages.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  switch( qt )
    {
    case NameQuery:       return( "" );
    case NodeStatusIP:    return( "-A" );
    case NodeStatusName:  return( "-a" );
    case BrowserFind:     return( "-b" );
    case Version:         return( "-V" );
    }
  return( "?!" );
  } /* qtoptchar */


static const char *RecTypeName( uint16_t rectype )
  /* ------------------------------------------------------------------------ **
   * Return the name of the Query or Resource Record record type.
   *
   *  Input:  rectype - An unsigned short that *should* be a valid record
   *                    type.  Only NB and NBSTAT are actually used by NBT.
   *
   *  Output: A pointer to a string indicating the record type.
   *
   *  Notes:  This needs locale support of some sort.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  switch( rectype )
    {
    case nbt_nsRRTYPE_A:      return( "IP Addr <unused>" );
    case nbt_nsRRTYPE_NS:     return( "NS <unused>" );
    case nbt_nsRRTYPE_NULL:   return( "NULL" );
    case nbt_nsRRTYPE_NB:     return( "NB" );
    case nbt_nsRRTYPE_NBSTAT: return( "NBSTAT" );
    }
  return( "<unknown>" );
  } /* RecTypeName */


static const char *OpCodeName( uint16_t Flags )
  /* ------------------------------------------------------------------------ **
   * Return the name of the opcode in the given Flags field.
   *
   *  Input:  Flags - The Flags field from an NBT Name Service header.
   *                  (This function will apply the appropriate mask.)
   *
   *  Output: A pointer to a string indicating the OpCode type.
   *
   *  Notes:  This needs locale support of some sort.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  switch( nbt_nsOPCODE_MASK & Flags )
    {
    case nbt_nsOPCODE_QUERY:      return( "Query" );
    case nbt_nsOPCODE_REGISTER:   return( "Name Registraton" );
    case nbt_nsOPCODE_RELEASE:    return( "Name Release" );
    case nbt_nsOPCODE_WACK:       return( "Wait for ACK" );
    case nbt_nsOPCODE_REFRESH:    return( "Name Refresh" );
    case nbt_nsOPCODE_ALTREFRESH: return( "Name Refresh <Alt>" );
    case nbt_nsOPCODE_MULTIHOMED: return( "Multi-homed Registration" );
    }
  return( "<unknown>" );
  } /* OpCodeName */


static const char *RCodeName( uchar Rcode )
  /* ------------------------------------------------------------------------ **
   * Return a string indicating the meaning of the Rcode in the given Flags
   * field.
   *
   *  Input:  Flags - The Flags field from an NBT Name Service header.
   *                  (This function will apply the appropriate mask.)
   *
   *  Output: A pointer to a string indicating the Rcode meaning.
   *
   *  Notes:  This needs locale support of some sort.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  switch( nbt_nsRCODE_MASK & Rcode )
    {
    case nbt_nsRCODE_POS_RSP: return( "Positive Response" );
    case nbt_nsRCODE_FMT_ERR: return( "Format Error" );
    case nbt_nsRCODE_SRV_ERR: return( "Server Failure" );
    case nbt_nsRCODE_NAM_ERR: return( "Name Error" );
    case nbt_nsRCODE_IMP_ERR: return( "Unsupported Request" );
    case nbt_nsRCODE_RFS_ERR: return( "Request Refused" );
    case nbt_nsRCODE_ACT_ERR: return( "Name Active Error" );
    case nbt_nsRCODE_CFT_ERR: return( "Name In Conflict" );
    }
  return( "<unknown>" );
  } /* RCodeName */


static char *ListFlags( uint16_t flags )
  /* ------------------------------------------------------------------------ **
   * Create a string listing the flags represented in the input.
   *
   *  Input:  flags - Message header flags field.
   *
   *  Output: Pointer to static string space containing a string listing the
   *          set of flags that are turned on (enabled) in the <flags> field.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  static char FlgStr[18];
  int         i;
  int         count;
  ValName     FlgArray[5] =
    {
      { nbt_nsAA_BIT, "AA" },
      { nbt_nsTR_BIT, "TR" },
      { nbt_nsRD_BIT, "RD" },
      { nbt_nsRA_BIT, "RA" },
      { nbt_nsB_BIT,  "B"  }
    };

  flags &= nbt_nsNMFLAG_MASK;
  if( 0 == flags )
    return( strcpy( FlgStr, "<none>" ) );

  FlgStr[0] = '\0';
  for( i = count = 0; i < 5; i++ )
    {
    if( ((FlgArray[i].val) & flags) )
      {
      if( count )
        strcat( FlgStr, ", " );
      strcat( FlgStr, FlgArray[i].name );
      count++;
      }
    }
  return( FlgStr );
  } /* ListFlags */


static void CheckNbName( const uchar *qname, const int qnamelen )
  /* ------------------------------------------------------------------------ **
   * Calls the nbt_CheckNbName() function and reports any syntax errors
   * or warnings.
   *
   *  Input:  qname     - Name to be checked.
   *                      (Probably the global QueryName.)
   *          qnamelen  - Length, in bytes, of the query name.  We need this
   *                      because some names may include embedded nul bytes.
   *
   *  Output: none.
   *
   *  Notes:  This is a wrapper around <nbt_CheckNbName()>.  This wrapper
   *          prints diagnostics if the lower-level function returns an
   *          error or warning code.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int result;

  result = nbt_CheckNbName( qname, qnamelen );
  if( result >= 0 )
    return;

  switch( result )
    {
    /* Failures */
    case cifs_errNullInput:
      Fail( "NetBIOS Name is NULL.\n" );
      break;
    case cifs_errNameTooLong:
      Fail( "NetBIOS Name \"%s\" is too long (15 byte max).\n", qname );
      break;

    /* Warnings */
    case cifs_warnNulByte:
      Warn( "NetBIOS Name \"%s\" contains a nul (0x00) byte.\n", qname );
      break;
    case cifs_warnContainsDot:
      Warn( "NetBIOS Name \"%s\" contains one or more dots.\n", qname );
      break;
    case cifs_warnEmptyStr:
      Warn( "NetBIOS Name is the empty string.\n" );
      break;
    case cifs_warnAsterisk:
      if( 0 != strncmp( QueryName, "*", 15 ) )
        Warn( "NetBIOS Name \"%s\" begins with an asterisk ('*').\n", qname );
      break;

    /* Default: Fail because we don't know what went wrong. */
    default:
      Fail( "Unknown error code from nbt_CheckNbName(): %d\n", result );
      break;
    }
  } /* CheckNbName */


static void CheckScope( const uchar *scope )
  /* ------------------------------------------------------------------------ **
   * Calls the nbt_CheckScope() function and reports any syntax errors
   * or warnings.
   *
   *  Input:  scope - scope string to be checked.
   *
   *  Output: none.
   *
   *  Notes:  It turns out that we *must* fail if there is a NULL label in
   *          the scope string, or if a label or the scope ID itself is too
   *          long.  If we don't then the scope may be written incorrectly
   *          into the message buffer, resulting in a malformed packet and
   *          possible errors on the remote end.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int result;

  result = nbt_CheckScope( scope );
  if( result >= 0 )
    return;

  switch( result )
    {
    case cifs_errNullInput:     /* This is okay. */
      break;

    /* Failures */
    case cifs_errLeadingDot:
      Fail( "Invalid Scope.  String starts with empty label (leading dot).\n" );
      break;
    case cifs_errDoubleDot:
      Fail( "Invalid Scope.  String contains empty label (multiple dots).\n" );
      break;
    case cifs_errEndDot:
      Fail( "Invalid Scope.  String ends with empty label (trailing dot).\n" );
      break;
    case cifs_errNameTooLong:
      Fail( "Invalid Scope.  Contains label that exceeds 63 bytes.\n" );
      break;
    case cifs_errScopeTooLong:
      Fail( "Invalid Scope.  Scope ID exceeds maximum length.\n" );
      break;

    /* Warnings */
    case cifs_warnNonPrint:
      Warn( "Scope label contains a non-printing character.\n" );
      break;
    case cifs_warnNonAlpha:
      Warn( "Scope label does not start with an alpha character.\n" );
      break;
    case cifs_warnInvalidChar:
      Warn( "Scope ID contains an invalid character.\n" );
      break;
    case cifs_warnNonAlphaNum:
      Warn( "Scope label does not end with an alpha-numeric.\n" );
      break;

    /* Fail (Steinbach's Guideline for Systems Programming) */
    default:
      Fail( "Unknown error code from nbt_CheckScope(): %d\n", result );
      break;
    }
  } /* CheckScope */


static querytype ReadOpts( int argc, char * const argv[] )
  /* ------------------------------------------------------------------------ **
   * Read the command-line options and verify that they make sense.  Ouch.
   *
   *  Input:  argc  - Argument count.
   *          argv  - Argument vector.  Basic C stuff.
   *
   *  Output: A querytype, identifying the type of query to be performed.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int          c;
  int          tmp;
  extern int   optind;
  extern char *optarg;
  querytype    qt         = NameQuery;
  bool         PrintUsage = false;

  if( argc <= 1 )
    usage( argv[0] );

  while( (c = getopt( argc, argv, "AaB:bcDhLp:R:rS:s:U:Vvw:" )) >= 0 )
    {
    switch( c )     /* Read the options. */
      {
      case 'A':
        if( (NameQuery == qt) || (NodeStatusIP == qt) )
          qt = NodeStatusIP;
        else
          Fail( "Option -A conflicts with %s.\n", qtoptchar( qt ) );
        break;
      case 'a':
        if( (NameQuery == qt) || (NodeStatusName == qt) )
          {
          qt = NodeStatusName;
          if( NULL == DestIP )
            Bcast = false;
          }
        else
          Fail( "Option -a conflicts with %s.\n", qtoptchar( qt ) );
        break;
      case 'b':
        if( (NameQuery == qt) || (BrowserFind == qt) )
          qt = BrowserFind;
        else
          Fail( "Option -b conflicts with %s.\n", qtoptchar( qt ) );
        break;
      case 'V':
        if( (NameQuery == qt) || (Version == qt) )
          qt = Version;
        else
          Fail( "Option -V conflicts with %s.\n", qtoptchar( qt ) );
        break;

      case 'B':
      case 'U':
        if( NULL == DestIP )
          DestIP = optarg;
        else
          Fail( "Destination address assigned twice (-B and/or -U).\n" );
        Bcast = ( 'U' == c ) ? false : true;
        break;

      case 'D':
        if( LMBQuery )
          Fail( "-D conflicts with -L -- choose one or the other.\n" );
        DMBQuery = true;
        if( (NameQuery == qt) || (BrowserFind == qt) )
          qt = BrowserFind;
        else
          Fail( "Option -D conflicts with %s.\n", qtoptchar( qt ) );
        break;

      case 'L':
        if( DMBQuery )
          Fail( "-L conflicts with -D -- choose one or the other.\n" );
        LMBQuery = true;
        if( (NameQuery == qt) || (BrowserFind == qt) )
          qt = BrowserFind;
        else
          Fail( "Option -L conflicts with %s.\n", qtoptchar( qt ) );
        break;

      case 'S':
        ScopeID = optarg;
        break;

      case 'p':
        tmp = util_XlateInput( optarg );
        if( tmp < 0 )
          Fail( "Invalid pad value.\n" );
        NameRec->pad = (uchar)tmp;
        ForcePad = true;
        break;
      case 's':
        tmp = util_XlateInput( optarg );
        if( tmp < 0 )
          Fail( "Invalid suffix value.\n" );
        NameRec->sfx = tmp;
        ForceSfx = true;
        break;

      case 'c':
        UpCase = false;
        break;
      case 'R':
        if( (0 == strcmp( "0", optarg )) || (0 == strcmp( "false", optarg )) )
          RecDes = t_false;
        else
          {
          if( (0 == strcmp( "1", optarg )) || (0 == strcmp( "true", optarg )) )
            RecDes = t_true;
          else
            Fail( "Cannot interpret '-R %s'; please use '0' or '1'.\n",
                   optarg );
          }
        break;
      case 'r':
        Port137 = true;
        break;
      case 'v':
        Verbose++;
        break;

      case 'w':
        {
        char *delim;

        /* FIX: cheesey code */
        if( isdigit( *optarg ) || ('-' == *optarg) )
          RetryWait = atoi( optarg );
        delim = strchr( optarg, ':' );
        if( NULL != delim )
          RetryInc = atoi( delim + 1 );
        delim = strchr( optarg, ',' );
        if( NULL != delim )
          RetryCnt = atoi( delim + 1 );
        break;
        }

      case 'h':
        PrintUsage = true;
        break;

      default:
        /* If the user makes an input error,
         * provide only the simple help message.
         */
        Verbose = 0;
        usage( argv[0] );
        break;
      }
    }

  /* If help was requested print the usage message and exit.
   * We do this after we have read all opts, in case there was a -v.
   */
  if( PrintUsage )
    usage( argv[0] );

  /* Grab any left over values (not associated with an option)
   * at the end of the command line.
   */
  if( optind < argc )
    QueryName = argv[optind];
  else
    {
    switch( qt )
      {
      case BrowserFind:
      case Version:
        /* NetBIOS name or IP address not required. */
        break;
      case NodeStatusIP:
        /* We need an IP address for NodeStatusIP (-A). */
        Fail( "Missing required input: destination IP address.\n" );
        break;
      default:
        /* We need a NetBIOS name for all others. */
        Fail( "Missing required input: destination NetBIOS name.\n" );
        break;
      }
    }

  /* Special handling for -D|-L, which are only valid with -b. */
  if( LMBQuery && (BrowserFind != qt) )
    Warn( "The -L option is only valid with browser queries (-b).\n%s\n",
          "\t Use '-s 1D' instead.  (-L ignored)" );
  if( DMBQuery && (BrowserFind != qt) )
    Warn( "The -D option is only valid with browser queries (-b).\n%s\n",
          "\t Use '-s 1B' instead.  (-D ignored)" );

  /* Now validate the options for each query type. */
  switch( qt )
    {
    case NodeStatusIP:
      if( !UpCase )
        Warn( "-c ignored with -A.\n" );
      if( NULL != DestIP )
        Warn( "-[B|U] <IP> ignored with -A.\n" );
      if( ForcePad )
        Warn( "-p <pad> ignored with -A.\n" );
      if( ForceSfx )
        Warn( "-s <sfx> ignored with -A.\n" );
      if( t_true == RecDes )
        Warn( "-R 1 used with -A; RD bit will be set.\n" );

      /* Shuffle some stuff around. */
      DestIP    = QueryName;      /* It's an IP (or DNS name). */
      QueryName = "*";            /* Wildcard query.           */
      ForcePad  = false;          /* Must use 0x00.            */
      ForceSfx  = false;          /* Must use 0x00.            */
      Bcast     = false;          /* Unicast to DestIP.        */
      qt        = NodeStatusName; /* We can pretend...         */
      break;

    case NodeStatusName:
      if( t_true == RecDes )
        Warn( "-R 1 used with -a; RD bit will be set.\n" );
      if( Bcast && (NULL != DestIP) )
        Warn( "-B <IP> used with -a; B bit will be set.\n" );
      break;

    case BrowserFind:
      if( !UpCase && (NULL != QueryName) )
        Warn( "-c used with -b.\n" );
      if( !Bcast && (NULL != DestIP) )
        Warn( "Checking node %s for Local Master Browser status.\n", DestIP );
      if( ForcePad )
        Warn( "-p <pad> ignored with -b.\n" );
      if( ForceSfx )
        Warn( "-s <sfx> ignored with -b.\n" );
      if( LMBQuery && (NULL == QueryName) )
        Fail( "-L option requires a NetBIOS Name.\n" );
      if( DMBQuery && (NULL == QueryName) )
        Fail( "-D option requires a NetBIOS Name.\n" );
      if( t_false == RecDes )
        Warn( "-R 0 used with browser query; RD bit will be clear.\n" );

      ForceSfx = true;
      if( NULL != QueryName )
        {
        if( LMBQuery )
          NameRec->sfx = '\x1D';
        else
          if( DMBQuery )
            NameRec->sfx = '\x1B';
          else
            NameRec->sfx = '\x1E';
        }
      else
        {
        QueryName = "\x01\x02__MSBROWSE__\x02";
        NameRec->sfx = '\x01';
        }
      break;

    default:
      break;
    }

  /* Validate the RetryWait, Increment, and Count values. */
  if( (RetryWait + ((RetryCnt - 1) * RetryInc)) > 0xFFFF )
    {
    Fail( "Total timeout may exceed maximum: -w %d:%d,%d.\n",
          RetryWait, RetryInc, RetryCnt );
    }
  if( (RetryWait < 0) || (RetryWait > 0xFFFF) )
    Fail( "Invalid reply timeout value: -w %d.\n", RetryWait );
  if( (RetryInc < 0) || (RetryInc > 0xFFFF) )
    Fail( "Invalid reply timeout increment value: -w :%d.\n", RetryInc );
  if( RetryCnt < 0 )
    Fail( "Invalid query retry value: -w ,%d.\n", RetryCnt );

  /* Clean up and validate the Scope ID. */
  if( ScopeID )
    {
    NameRec->scope_id = (uchar *)strdup( ScopeID );
    tmp = util_UnEscStr( NameRec->scope_id );
    if( UpCase )
      if( nbt_UpCaseStr( NameRec->scope_id, NULL, tmp ) < 0 )
        Fail( "Error value returned by UpCaseStr( %s ).\n", ScopeID );
    CheckScope( NameRec->scope_id );
    }

  /* Clean up and validate the query name,
   * and fill in all remaining NameRec fields
   */
  if( QueryName )
    {
    /* Unescape the name, copying the unescaped version into NameRec. */
    NameRec->name    = (uchar *)strdup( QueryName );
    NameRec->namelen = util_UnEscStr( NameRec->name );
    if( UpCase )
      if( nbt_UpCaseStr( NameRec->name, NULL, NameRec->namelen ) < 0 )
        Fail( "Error value returned by nbt_UpCaseStr( %s ).\n", QueryName );
    CheckNbName( NameRec->name, NameRec->namelen );
    /* Special conditions for the wildcard query. */
    if( 0 == strncmp( (char *)NameRec->name, "*", 15 ) )
      {
      if( !ForcePad )
        NameRec->pad = '\0';
      if( !ForceSfx )
        NameRec->sfx = '\0';
      }
    }

  return( qt );
  } /* ReadOpts */


static struct in_addr ResolveDestAddr( char *Dest )
  /* ------------------------------------------------------------------------ **
   * Resolve destination to an IP address via DNS lookup.
   *
   *  Input:  Dest  - Destination address in string form.  May be a DNS name
   *                  or the string representation of an IPv4 address (in
   *                  dotted quad notation).
   *
   *  Output: The resolved address.  If <Dest> was NULL, then this function
   *          will return INADDR_BROADCAST.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  struct in_addr bcast = { INADDR_BROADCAST };

  if( NULL != Dest )
    {
    struct hostent *entry;
    extern int      h_errno;

    entry = gethostbyname( Dest );
    if( NULL == entry )
      Fail( "DNS name or IP address lookup failure: %s\n", hstrerror(h_errno) );
    return(  *((struct in_addr *)(entry->h_addr_list[0])) );
    }
  return( bcast );
  } /* ResolveDestAddr */


static uchar *Hexify( uchar *str, int len )
  /* ------------------------------------------------------------------------ **
   * Rewrite a string, converting any non-printing characters to hex escape
   * sequences.
   *
   *  Input:  str - Source string to be converted.
   *          len - length of the source string.
   *
   *  Output: A pointer to a buffer containing the converted string.
   *          You don't own this buffer.  Don't free it or otherwise
   *          mess with it.  Just read the contents (copy if desired).
   *
   *  Notes:  This function does *not* terminate when it finds a nul byte.
   *          It will hexify string terminators... and keep on going.
   *          Make sure to pass in a valid value for <len>.
   *
   *          In rare cases (an empty NetBIOS name "") len will be zero
   *          and, as a result, we skip the whole buffer allocation step
   *          above.  That would make bufr==NULL, which causes the
   *          util_Hexify() function to return an error.  We check for this
   *          error and return "" if it shows up.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  static uint32_t  bsize = 0;
  static uchar    *bufr  = NULL;
  uint32_t         i     = 4 * len;

  /* Ensure we have a local buffer that is large enough. */
  if( bsize < i )
    {
    if( bufr )
      free( bufr );
    bufr = (uchar *)malloc( i );
    if( bufr )
      bsize = i;
    else
      Fail( "Failed to allocate %ld bytes in Hexify().\n", i );
    }

  if( (len < 1) || (util_Hexify( bufr, str, len ) < 0) )
    return( (uchar *)"" );

  return( bufr );
  } /* Hexify */


static uchar *FormatName( uchar *src )
  /* ------------------------------------------------------------------------ **
   * Given an L2Encoded NBT Name, format it for printing purposes.
   *
   *  Input:  src - Pointer to the starting byte of the NBT Name.
   *
   *  Output: Pointer to a static buffer containing the formatted string.
   *
   *  Notes:  One of the side-effects of being pedantic is that my own
   *          sense of correctness doesn't always mach that of the rest
   *          of the world.  Standard functions such as strcpy(3) and
   *          sprintf(3) expect parameters of type char *, but I prefer
   *          to use unsigned char * (uchar *) types to make it clear
   *          that I am dealing with eight-bit-wide characters.  I seem
   *          to recall that some systems handled the 'char' type as a
   *          signed value, while others view it as unsigned by default.
   *
   *          Anyway, the point of all that discussion is to explain how
   *          the <charstr> local variable is used in the code below.
   *          Basically, I have two pointers to the same chunk of memory,
   *          one of each signage.  This allows me to pass the same block
   *          of memory to different function which require different
   *          signage, all without generating compiler warnings.  In the
   *          end, the compiled code is probably the same as if I just did
   *          typecasts.  It's just that the typecasts were confusing to
   *          read and took up a lot of space in the text of the code.  crh
   *
   * ------------------------------------------------------------------------ **
   */
  {
  uchar        suffix;
  int          namestrlen;
  static uchar namestr[300];
  char        *charstr = (char *)namestr;  /* for type compatibility */

  namestrlen = nbt_L1Decode( namestr, src, 1, ' ', &suffix );
  if( namestrlen < 0 )
    {
    (void)sprintf( charstr, "***Garbled NBT name: [%-32.32s]", &src[1] );
    return( namestr );
    }

  /* We count on the fact that Hexify() has its own storage
   * for the hexified string.
   */
  (void)strcpy( charstr, (char *)Hexify( namestr, namestrlen ) );
  namestrlen  = (int)strlen( charstr );
  namestrlen += (int)sprintf( &charstr[namestrlen], "<%.2x>", suffix );

  /* Decode the scope ID to the string buffer starting one byte beyond
   * the nul that terminates the NetBIOS name.  If there is a scope ID,
   * then go back and replace that nul byte with a dot.
   */
  if( 0 < nbt_L2Decode( &namestr[namestrlen+1], src, nbt_L1_NB_NAME_MAX ) )
    namestr[namestrlen] = '.';

  return( namestr );
  } /* FormatName */


static void OpenSocket( void )
  /* ------------------------------------------------------------------------ **
   * Open the UDP socket so that we can send/receive message.
   *
   *  Input:  none
   *  Output: none
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int                tmp = 1;
  int                result;
  struct sockaddr_in sox;

  OurSocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
  if( OurSocket < 0 )
    Fail( "Failed to create socket(); %s.\n", strerror( errno ) );

  result = setsockopt( OurSocket,
                       SOL_SOCKET,
                       SO_BROADCAST,
                       &tmp,
                       sizeof(int) );
  if( result < 0 )
    Fail( "Failed to setsockopt(); %s.\n", strerror( errno ) );

  sox.sin_addr.s_addr = INADDR_ANY;
  sox.sin_family      = AF_INET;
  sox.sin_port        = htons( Port137 ? 137 : 0 );
  result = bind( OurSocket,
                 (struct sockaddr *)&sox, sizeof(struct sockaddr_in) );
  if( result < 0 )
    Fail( "Failed to bind() socket; %s.\n", strerror( errno ) );

  if( Verbose )
    {
    int i = sizeof( struct sockaddr_in );

    result = getsockname( OurSocket,
                          (struct sockaddr *)&sox, (socklen_t *)&i );
    if( 0 == result )
      Say( "Sending from port %d.\n", ntohs( sox.sin_port ) );
    }
  } /* OpenSocket */


static void SendQuery( uchar *msg, int msglen )
  /* ------------------------------------------------------------------------ **
   * Send the query message.
   *
   *  Input:  msg     - The message to be sent.
   *          msglen  - The length (in bytes) of the message to be sent.
   *
   *  Output: None.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  struct sockaddr_in sox;

  sox.sin_addr   = DestAddr;
  sox.sin_family = AF_INET;
  sox.sin_port   = htons( 137 );

  if( Verbose )
    Say( "Sending to %s:137.\n", inet_ntoa( sox.sin_addr ) );

  if( sendto( OurSocket,
              (void *)msg,
              msglen,
              0,
              (struct sockaddr *)&sox,
              sizeof(struct sockaddr_in) ) < 0 )
    {
    Fail( "Sendto(); %s.\n", strerror( errno ) );
    exit( 0 );
    }
  } /* SendQuery */


static int CheckReply( nbt_nsMsgBlock *msg )
  /* ------------------------------------------------------------------------ **
   * Parse the packet and report any parsing errors.
   *
   *  Input:  msg - Pointer to the message to be parsed.
   *
   *  Output: -1 on error, else an integer in the nbt_nsMsgType enum range.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int MsgType;

  MsgType = nbt_nsParseMsg( msg );
  if( MsgType < 0 )
    {
    switch( MsgType )
      {
      case cifs_errInvalidLblLen:
        Warn( "Packet Parse Failure - Invalid NBT Name Label Length [%d].\n",
               MsgType );
        break;
      case cifs_errOutOfBounds:
        Warn( "Internal error in nbt_nsParseMsg() [%d].\n", MsgType );
        break;
      case cifs_errTruncatedBufr:
        Warn( "Packet Parse Failure - Short packet, missing data [%d].\n",
              MsgType );
        break;
      case cifs_errNameTooLong:
        Warn( "Packet Parse Failure - NBT Name too long [%d].\n", MsgType );
        break;
      case cifs_errNullInput:
        Warn( "NULL buffer, Cannot parse [%d].\n", MsgType );
        break;
      case cifs_errInvalidPacket:
        Warn( "Packet Parse Failure - Syntax error [%d].\n", MsgType );
        break;
      case cifs_errUnknownCommand:
        Warn( "Packet Parse Failure - Unknown OpCode in Packet [%d].\n",
              MsgType );
        break;
      default:
        Warn( "Unknown error parsing packet [%d].\n", MsgType );
        break;
      }
    return( -1 );
    }
  return( MsgType );
  } /* CheckReply */


static void DumpReply( uchar *reply, int replylen )
  /* ------------------------------------------------------------------------ **
   * Decomposes and prints the reply in simple format.
   *
   *  Input:  reply     - Pointer to the received reply.
   *          replylen  - Number of bytes in the reply.
   *
   *  Output: none.
   *
   *  Notes:  We handle three types of packet
   *          + Positive Name Query Response
   *          + Negative Name Query Response
   *          + Node Status Response
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int            i;
  int            offset;
  char           IPstr[16];
  int            MsgType;  
  nbt_nsMsgBlock msg[1];

  msg->block.bufr = reply;
  msg->block.size = bSIZE;
  msg->block.used = replylen;
  MsgType = CheckReply( msg );
  if( MsgType < 0 )
    return;

  /* Now provide output.
   */
  switch( MsgType )
    {
    case nbt_nsNAME_QUERY_REPLY_POS:
      {
      uchar   *rdata = msg->rdata;
      uint16_t nbflags;
      int      entcount = (msg->rdata_len / 6);

      for( i = offset = 0; i < entcount; i++ )
        {
        nbflags = nbt_GetShort( rdata, offset );
        offset += 2;
        (void)sprintf( IPstr, "%d.%d.%d.%d",
                       rdata[offset],   rdata[offset+1],
                       rdata[offset+2], rdata[offset+3] );
        Say( "%-15s ", IPstr );
        offset += 4;
        Say( "[%c,", ( nbt_nsGROUP_BIT & nbflags )?'G':'U' );
        switch( nbt_nsONT_MASK & nbflags )
          {
          case nbt_nsONT_B:  Say( "B]" ); break;
          case nbt_nsONT_P:  Say( "P]" ); break;
          case nbt_nsONT_M:  Say( "M]" ); break;
          case nbt_nsONT_H:  Say( "H]" ); break;
          }
        Say( " %s\n", FormatName( msg->RR_name ) );
        }
      }
      break;

    case nbt_nsNAME_QUERY_REPLY_NEG:
      {
      int rcode = (nbt_nsRCODE_MASK & msg->flags);

      Say( "0x%.1x == ", rcode );
      switch( rcode )
        {
        case nbt_nsRCODE_FMT_ERR: Say( "Format Error: " ); break;
        case nbt_nsRCODE_SRV_ERR: Say( "Server Failure: " ); break;
        case nbt_nsRCODE_NAM_ERR: Say( "Name Not Found: " ); break;
        case nbt_nsRCODE_IMP_ERR: Say( "Unsupported request: " ); break;
        case nbt_nsRCODE_RFS_ERR: Say( "Request Refused: " ); break;
        case nbt_nsRCODE_ACT_ERR: Say( "Active Error: " ); break;
        case nbt_nsRCODE_CFT_ERR: Say( "Conflict Error: " ); break;
        default:                  Say( "Unknown error: " ); break;
        }
      Say( "%s\n", FormatName( msg->RR_name ) );
      }
      break;

    case nbt_nsNODE_STATUS_REPLY:
      {
      uint16_t name_flags;
      uchar   *rdata     = msg->rdata;
      int      num_names = *rdata;

      offset = 1;   /* We have read one byte into num_names. */

      Say( "Response to Node Status Request for: %s\n",
           FormatName( msg->RR_name ) );

      for( i = 0; i < num_names; i++ )
        {
        Say( "%s<%.2x> ", Hexify( &rdata[offset], 15 ), rdata[offset+15] );
        offset += 16;
        name_flags = nbt_GetShort( rdata, offset );
        offset += 2;
        Say( "[%c", (nbt_nsGROUP_BIT & name_flags)?'G':'U' );
        switch( nbt_nsONT_MASK & name_flags )
          {
          case nbt_nsONT_B:  Say( ",B" ); break;
          case nbt_nsONT_P:  Say( ",P" ); break;
          case nbt_nsONT_M:  Say( ",M" ); break;
          case nbt_nsONT_H:  Say( ",H" ); break;
          }
        if( nbt_nsDRG & name_flags )
          Say( ",DRG" );
        if( nbt_nsCNF & name_flags )
          Say( ",CNF" );
        if( nbt_nsACT & name_flags )
          Say( ",ACT" );
        if( nbt_nsPRM & name_flags )
          Say( ",PRM" );
        Say( "]\n" );
        }
      Say( "MAC Addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
           rdata[offset],   rdata[offset+1], rdata[offset+2],
           rdata[offset+3], rdata[offset+4], rdata[offset+5] );
      }
      break;

    default:
      Warn( "Unexpected Packet: OpCode == 0x%.1x\n",
            ((nbt_nsOPCODE_MASK & msg->flags) >> 11) & 0xF );
    }

  } /* DumpReply */


static void VDumpReply( uchar *reply, int replylen )
  /* ------------------------------------------------------------------------ **
   * This is a joyous mess.  It decomposes and prints the reply in very
   * verbose format.
   *
   *  Input:  reply     - Pointer to the received reply.
   *          replylen  - Number of bytes in the reply.
   *
   *  Output: none.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int            i, j;
  int            offset;
  int            MsgType;
  nbt_nsMsgBlock msg[1];

  msg->block.bufr = reply;
  msg->block.size = bSIZE;
  msg->block.used = replylen;
  MsgType = CheckReply( msg );
  if( MsgType < 0 )
    return;

  /* We should only be getting query responses. */
  if( nbt_nsOPCODE_QUERY != (nbt_nsOPCODE_MASK & msg->flags) )
    {
    Warn( "Unexpected Packet: OpCode == 0x%.1x\n",
          ((nbt_nsOPCODE_MASK & msg->flags) >> 11) & 0xF );
    return;
    }

  /* The RR_TYPE should be NB, NBSTAT, or NULL.
   * Samba's nmbd will use NULL for negative query responses.
   * That is correct per RFC1002, but doesn't match older nmbd behavior
   * or Windows behavior.
   */
  if( (nbt_nsRRTYPE_NB != msg->RR_type)
   && (nbt_nsRRTYPE_NBSTAT != msg->RR_type)
   && (nbt_nsRRTYPE_NULL != msg->RR_type) )
    {
    Warn( "Unexpected RR_TYPE: 0x%.4x.\n", msg->RR_type );
    return;
    }

  /* A NULL RR_TYPE should only occur if the response is negative. */
  if( (nbt_nsRRTYPE_NULL == msg->RR_type)
   && (nbt_nsRCODE_POS_RSP == (nbt_nsRCODE_MASK & msg->flags)) )
    {
    Say( "POSITIVE NAME QUERY RESPONSE (%d bytes)\n", replylen );
    Warn( "Unexpected RR_TYPE: 0x%.4x.\n", msg->RR_type );
    return;
    }

  if( (nbt_nsRRTYPE_NB == msg->RR_type) || (nbt_nsRRTYPE_NULL == msg->RR_type) )
    {
    if( nbt_nsRCODE_POS_RSP == (nbt_nsRCODE_MASK & msg->flags) )
      Say( "POSITIVE" );
    else
      Say( "NEGATIVE" );
    Say( " NAME QUERY RESPONSE (%d bytes)\n", replylen );
    }
  else
    Say( "NODE STATUS RESPONSE (%d bytes)\n", replylen );

  Say( "  {\n" );
  Say( "  HEADER\n" );
  Say( "    {\n" );
  Say( "    NAME_TRN_ID = 0x%x (%.5d)\n", msg->tid, msg->tid );
  Say( "    FLAGS\n" );
  Say( "      {\n" );
  Say( "      R      = %d\n", (nbt_nsR_BIT  & msg->flags)?1:0 );
  Say( "      OPCODE = 0x%.1x",
       ((nbt_nsOPCODE_MASK & msg->flags) >> 11) & 0xF );
  Say( " (%s)\n", OpCodeName( msg->flags ) );
  Say( "      AA     = %d\n", (nbt_nsAA_BIT & msg->flags)?1:0 );
  Say( "      TC     = %d\n", (nbt_nsTR_BIT & msg->flags)?1:0 );
  Say( "      RD     = %d\n", (nbt_nsRD_BIT & msg->flags)?1:0 );
  Say( "      RA     = %d\n", (nbt_nsRA_BIT & msg->flags)?1:0 );
  Say( "      B      = %d\n", (nbt_nsB_BIT  & msg->flags)?1:0 );
  Say( "      RCODE  = 0x%.1x", (nbt_nsRCODE_MASK & msg->flags) );
  Say( " (%s)\n", RCodeName( msg->flags ) );
  Say( "      }\n" );
  Say( "    QD_COUNT = %d\n", (nbt_nsQUERYREC & msg->rmap)?1:0 );
  Say( "    AN_COUNT = %d\n", (nbt_nsANSREC & msg->rmap)?1:0 );
  Say( "    }\n" );

  Say( "  ANSWER RECORD\n" );
  Say( "    {\n" );
  Say( "    RR_NAME  = %s\n", FormatName( msg->RR_name ) );
  Say( "    RR_TYPE  = 0x%.4x", msg->RR_type );
  Say( " (%s)\n", RecTypeName( msg->RR_type ) );
  Say( "    TTL      = 0x%.8x (%u seconds)\n", msg->ttl, msg->ttl );
  Say( "    RDLENGTH = %u\n", msg->rdata_len );
  Say( "    RDATA\n" );
  Say( "      {\n" );

  if( nbt_nsRRTYPE_NB == msg->RR_type )
    {
    uint16_t nbflags;
    uchar   *rdata = msg->rdata;

    for( i = offset = 0, j = (msg->rdata_len / 6); i < j; i++ )
      {
      nbflags = nbt_GetShort( rdata, offset );
      offset += 2;
      Say( "      ADDR_ENTRY[%d]\n", i );
      Say( "        {\n" );
      Say( "        NB_FLAGS\n" );
      Say( "          {\n" );
      Say( "          G    = %d\n", (nbt_nsGROUP_BIT & nbflags)?1:0 );
      Say( "          ONT  = " );
      switch( nbt_nsONT_MASK & nbflags )
        {
        case nbt_nsONT_B:  Say( "B (0x00)\n" ); break;
        case nbt_nsONT_P:  Say( "P (0x01)\n" ); break;
        case nbt_nsONT_M:  Say( "M (0x10)\n" ); break;
        case nbt_nsONT_H:  Say( "H (0x11)\n" ); break;
        default:  Say( "??\n" ); break;
        }
      Say( "          }\n" );
      Say( "        NB_ADDRESS = %d.%d.%d.%d\n",
           rdata[offset], rdata[offset+1], rdata[offset+2], rdata[offset+3] );
      offset += 4;
      Say( "        }\n" );
      }
    }
  else  /* Node Status Reply */
    {
    uchar   *rdata = msg->rdata;
    int      num_names = *rdata;
    uint16_t name_flags;

    offset = 1;   /* We have already eaten one byte for num_names. */
    Say( "      NUM_NAMES = %d\n", num_names );
    for( i = 0; i < num_names; i++ )
      {
      Say( "      NODE_NAME[%d]\n", i );
      Say( "        {\n" );
      Say( "        NETBIOS_NAME = %s", Hexify( &rdata[offset], 15 ) );
      Say( "<%.2x>\n", rdata[offset+15] );
      offset += 16;
      name_flags = nbt_GetShort( rdata, offset );
      offset += 2;
      Say( "        NAME_FLAGS\n" );
      Say( "          {\n" );
      Say( "          G   = %d\n", (nbt_nsGROUP_BIT & name_flags)?1:0 );
      Say( "          ONT = " );
      switch( nbt_nsONT_MASK & name_flags )
        {
        case nbt_nsONT_B:  Say( "B (0x00)\n" ); break;
        case nbt_nsONT_P:  Say( "P (0x01)\n" ); break;
        case nbt_nsONT_M:  Say( "M (0x10)\n" ); break;
        case nbt_nsONT_H:  Say( "H (0x11)\n" ); break;
        default:  Say( "??\n" ); break;
        }
      Say( "          DRG = %d\n", (nbt_nsDRG & name_flags)?1:0 );
      Say( "          CNF = %d\n", (nbt_nsCNF & name_flags)?1:0 );
      Say( "          ACT = %d\n", (nbt_nsACT & name_flags)?1:0 );
      Say( "          PRM = %d\n", (nbt_nsPRM & name_flags)?1:0 );
      Say( "          }\n" );
      Say( "        }\n" );
      }
    Say( "      STATISTICS\n" );
    Say( "        {\n" );
    Say( "        MAC = %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
          rdata[offset],   rdata[offset+1], rdata[offset+2],
          rdata[offset+3], rdata[offset+4], rdata[offset+5] );
    /* Verbose level > 2 returns a dump of the stats portion. */
    if( 3 <= Verbose )
      {
      uchar tmpbfr[80];

      offset += 6;
      for( i = 0; offset < replylen; i += 16 )
        {
        offset += util_HexDumpLn( tmpbfr, &rdata[offset], (replylen - offset) );
        Say( "        %.2x: %s\n", i, tmpbfr );
        }
      }
    Say( "        }\n" );
    }
  Say( "      }\n" );
  Say( "    }\n" );
  Say( "  }\n" );
  } /* VDumpReply */


static int MinPoll( struct pollfd *ufds,
                    unsigned int   nfds,
                    uint16_t       mintime,
                    uint16_t       maxtime )
  /* ------------------------------------------------------------------------ **
   * Based on poll(2), but forces a minimum amount of wait time before
   * returning so that multiple replies can accumulate.
   *
   *  Input:  ufds    - An array of pollfd records, as described in poll(2).
   *          nfds    - The number of entries in the <ufds> array.
   *          mintime - The minimum time to wait for messages.
   *          maxtime - The maximum time to wait for messages.
   *
   *  Output: The return value of the poll(2) function.  This function is
   *          a wrapper for poll(2):
   *
   *            "On success, a positive number is returned, where the number
   *            returned is the number of structures which have non-zero
   *            revents fields (in other words, those descriptors with events
   *            or errors reported).  A value of 0 indicates that the call
   *            timed out and no file descriptors have been selected.  On
   *            error, -1 is returned, and errno is set appropriately."
   *
   *  Notes:  <mintime> and <maxtime> are in milliseconds (1/1000 of a second).
   *          Both are unsigned short ints, which limits the range of values
   *          to 0..65.535 seconds.
   *
   *          This function is blocking and will take at least <mintime>
   *          milliseconds to complete.
   *
   *          If <maxtime> is greater than or equal to <mintime> then the
   *          function will take at most <maxtime> milliseconds to complete.
   *
   *          If <maxtime> is less than <mintime> then the function will
   *          wait an infinite amount of time for one of the requested
   *          events in <ufds> to occur.
   *
   *          See poll(2) for much more information on the <ufds> and <nfds>
   *          parameters, and the return value.
   *
   * ------------------------------------------------------------------------ **
   */
  {
  uint32_t sleeptime = (uint32_t)mintime * 1000;        /* Max 65,535,000.    */
  uint32_t minisleep = 0;
  int      polltime  = (int)maxtime - (int)mintime;     /* Could be negative. */

  if( Verbose > 1 )
    Say( "Reply timeout minimum = %dms, maximum = %dms\n", mintime, maxtime );

  /* The poll(2) function counts milliseconds, but the usleep(3) function
   * counts microseconds (1/1,000,000 second).
   * Some usleep(3) implementations return an error if the sleep time
   * is 1,000,000 (one second) or more, so we break it down into workable
   * half-second chunks.  The nanosleep(3) function might be a better choice.
   * (...and usleep(3) is listed as depricated in some manual pages.)
   */
  while( sleeptime > 0 )
    {
    minisleep = (sleeptime > 500000) ? 500000 : sleeptime;
    (void)usleep( minisleep );
    sleeptime -= minisleep;
    }
  return( poll( ufds, nfds, polltime ) );
  } /* MinPoll */


static void doQuery( uchar Qtype )
  /* ------------------------------------------------------------------------ **
   * Perform the query.
   *
   *  Input:  Qtype - Either nbt_nsQTYPE_NB or nbt_nsQTYPE_NBSTAT.
   *
   *  Output: <none>
   *
   * ------------------------------------------------------------------------ **
   */
  {
  int           i;
  int           result;
  int           msglen;
  struct pollfd pfd[1];
  uint16_t      flags = nbt_nsOPCODE_QUERY | (Bcast  ? nbt_nsB_BIT  : 0);

  /* Set or clear the RD flag. */
  switch( Qtype )
    {
    case nbt_nsQTYPE_NB:
      /* RD should normally be set in name queries. */
      flags |= ((t_false == RecDes) ? 0 : nbt_nsRD_BIT );
      break;
    case nbt_nsQTYPE_NBSTAT:
      /* RD should normally be clear in node status queries. */
      flags |= ((t_true == RecDes) ? nbt_nsRD_BIT : 0 );
      break;
    }

  /* Create the header. */
  result = nbt_nsSetHdr( SendBufr, bSIZE, flags, nbt_nsQUERYREC );
  switch( result )
    {
    case cifs_errBufrTooSmall:
      Fail( 
        "[Internal wierdness] nbt_nsSetHdr() buffer too small in doNameQuery()"
          );
      break;
    default:
      if( result < 0 )
        Fail( "Unknown error %d from nbt_nsSetHdr()" );
      break;
    }

  nbt_nsSetTID( SendBufr, TID );

  /* Fill in the Query Name. */
  result = nbt_L2Encode( &(SendBufr[nbt_nsHEADER_LEN]), NameRec );
  if( result < 0 )
    Fail( "Unknown error %d returned from L2Encode().\n", result );

  if( Verbose )
    {
    Say( "Query Name: [%s", Hexify( NameRec->name, NameRec->namelen ) );
    for( i = 0; i < 15; i++ )
      Say( "%s", (char *)Hexify( (uchar *)&(NameRec->pad), 1 ) );
    Say( "<%.2x>", NameRec->sfx );
    if( NULL != ScopeID && *ScopeID )
      Info (".%s", ScopeID );
    Say( "]\n" );

    Say( "  NBT Name: [%s]\n",
         Hexify( &(SendBufr[nbt_nsHEADER_LEN]), result ) );
    Say( "     Flags: [%s]\n", ListFlags( flags ) );
    }

  /* Fill in Qtype and Qclass. */
  msglen = result + nbt_nsHEADER_LEN;
  nbt_SetShort( SendBufr, msglen, Qtype );
  msglen += 2;
  nbt_SetShort( SendBufr, msglen, nbt_nsQCLASS_IN );
  msglen += 2;

  /* Sending once, sending twice, sending chicken soup with rice. */
  OpenSocket();
  pfd->fd     = OurSocket;
  pfd->events = POLLIN;
  for( result = i = 0; i < RetryCnt; i++ )
    {
    SendQuery( SendBufr, msglen );
    result = MinPoll( pfd, 1, RetryWait, RetryWait + (i * RetryInc) );
    if( result > 0 )
      break;
    if( result < 0 )
      Fail( "Error while waiting for reply: %s.\n", strerror( errno ) );
    }

  if( 0 == result )
    {
    Say( "No replies received.\n" );
    }
  else
    {
    /* Something received.
     * Grab it, dump it,
     * and then wait at most a fraction of a second for more.
     * Note: we are finished using SendBufr so we can recycle it.
     */
    do
      {
      struct sockaddr_in *sox = (struct sockaddr_in *)SendBufr;

      result = bSIZE;
      msglen = recvfrom( OurSocket, RecvBufr, bSIZE, 0,
                         (struct sockaddr *)sox, (socklen_t *)&result );
      if( msglen < 0 )
        Fail( "Error reading reply: %s.\n", strerror( errno ) );

      if( 0 == Verbose )
        {
        DumpReply( RecvBufr, msglen );
        }
      else
        {
        Say( "\nReply from %s:%d\n",
             inet_ntoa( sox->sin_addr ),
             ntohs( sox->sin_port ) );
        if( 1 == Verbose )
          DumpReply( RecvBufr, msglen );
        else
          VDumpReply( RecvBufr, msglen );
        }
      } while( poll( pfd, 1, 250 ) > 0 );
    }

  close( OurSocket );
  } /* doQuery */


int main( int argc, char *argv[] )
  /* ------------------------------------------------------------------------ **
   * Main.
   *
   *  Input:  argc  - Number of entries in the argv[] array.
   *          argv  - Array of strings, each representing one command line
   *                  parameter (with argv[0] being the command name itself).
   *
   *  Output: An integer return code.  This program returns either
   *          EXIT_SUCCESS or EXIT_FAILURE (typically defined in stdlib.h).
   *
   * ------------------------------------------------------------------------ **
   */
  {
  querytype qt;

  /* Read and process the command line options.
   * Return the query type.
   */
  qt = ReadOpts( argc, argv );

  /* The destination is an IP address or DNS name.
   * (The default is the limited broadcast address: 255.255.255.255).
   * Convert it to struct in_addr.
   */
  DestAddr = ResolveDestAddr( DestIP );

  /* Perform the query.
   */
  switch( qt )
    {
    case BrowserFind:
    case NameQuery:
      doQuery( nbt_nsQTYPE_NB );
      break;
    case NodeStatusIP:
    case NodeStatusName:
      doQuery( nbt_nsQTYPE_NBSTAT );
      break;
    default:
      if( Verbose > 1 )
        Say( "%s\n", Copyright );
      Say( "%s\n", Verbose ? ID : Revision );
      break;
    }
  return( EXIT_SUCCESS );
  } /* main */

/* ========================================================================== */
