//
// Article.C -- Manage newsgroup articles
//
// Copyright 2003 Michael Sweet
// Copyright 2002 Greg Ercolano
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public Licensse as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "Article.H"

// RETURN STRING VERSION OF UNSIGNED LONG
static string ultos(unsigned long num)
{
    ostringstream buffer;
    buffer << num;
    return(buffer.str());
}

// TRUNCATE STRING AT FIRST CR|LF
static void TruncateCrlf(char *s)
{
    char *ss = strpbrk(s, "\r\n");
    if ( ss ) *ss = '\0';
}

// SPLIT RFC822 3.1.2 HEADER INTO KEY/VALUE PAIRS
//    If header malformed, key == "".
//
static void SplitKeyValue(char *s, string& key, string& val)
{
    key = ""; val = "";		// empty strings first
    char *sep = strchr(s, ':');
    if ( ! sep ) return;	// malformed?

    // Parse key
    ++sep;			// just past ':'
    char save = *sep;
    *sep = '\0';		// "To: fred" -> "To:\0fred"
    key = s;			// key="To:"
    *sep = save;

    // Parse value, skip all leading white (if any)
    //    eg. if s="Subject: xyz", val will be "xyz", not " xyz"
    //
    val = sep + strspn(sep, " \t");
}

// CREATE ABSOLUTE PATHNAME TO THE GROUP
//     e.g. "/var/spool/news/rush/general/1234"
//
string Article::_ArticlePath(unsigned long num)
{
    // rush.general -> rush/general
    string pathgroup = group; 
    replace(pathgroup.begin(), pathgroup.end(), '.', '/');
    return(string(G_conf.SpoolDir()) + string("/") +
	   pathgroup + string("/") + ultos(number));
}

// PARSE HEADER INTO CLASS
//     Returns -1 if header unknown.
//
int Article::_ParseHeader(string& key, string& val)
{
    // fprintf(stderr, "PARSING KEY='%s' VAL='%s'\n", key.c_str(), val.c_str());
    
    // HEADER NAMES ARE CASE INSENSITIVE: INTERNET DRAFT (Son of RFC1036)
    const char *s = key.c_str();
    if ( strcasecmp(s, "Subject:" ) == 0 ) 
	{ subject = val; }
    else if ( strcasecmp(s, "From:" ) == 0 )
	{ from = val; }
    else if ( strcasecmp(s, "Date:" ) == 0 )
	{ date = val; }
    else if ( strcasecmp(s, "Xref:" ) == 0 )
	{ xref = val; }
    else if ( strcasecmp(s, "Message-ID:") == 0 ) 
	{ messageid = val; }
    else if ( strcasecmp(s, "References:") == 0 ) 
	{ references = val; }
    else if ( strcasecmp(s, "Lines:") == 0 ) 
	{ lines = atoi(val.c_str()); }
    else
        return(-1);
    return(0);
}

// LOAD ARTICLE FROM SPECIFIED GROUP
int Article::Load(const char *groupname, unsigned long num)
{
    // ZERO OUT FIELDS
    //group       = "";		// don't clear; parent may call us with this->group
    filename      = "";
    number        = num;
    valid         = 0;		// assume invalid until successful
    from          = "";
    date          = "";
    messageid     = "";
    subject       = "";
    references    = "";
    xref          = "";
    lines         = 0;
    errmsg        = "";

    if ( strlen(groupname) >= GROUP_MAX )
         { errmsg = "Group name too long"; return(-1); }

    group = groupname;

    filename = _ArticlePath(number);

    FILE *fp = fopen(filename.c_str(), "r");
    if ( fp == NULL )
    {
        errmsg = string("article ") + ultos(number) + 
	         string(" no longer exists: '") + filename +
		 string("': ") + string(strerror(errno));
	return(-1);
    }

    // Folding/unfolding of multiline headers
    //     RFC822 3.1.1 (LONG HEADER FIELDS)
    //     RFC822 3.4.8 (FOLDING LONG HEADER FIELDS)
    //

    // LOAD KEY/VALUE PAIRS
    int done = 0;
    string key, val;
    char s[LINE_LEN];

    while ( !done && fgets(s, sizeof(s)-1, fp) != NULL )
    {
        TruncateCrlf(s);

	switch ( s[0] )
	{
	    // CONTINUING TO UNFOLD MULTILINE HEADER? (RFC822 3.1.1)
	    case '\t':
	    case ' ':
		val += (s+1);
		if ( val.length() >= FIELD_MAX )		// prevent ram DoS
		    { val.erase(FIELD_MAX-1, val.length()); }	// truncate
		continue;

	    // END OF HEADERS?
	    case '\0':
		if ( key != "" )		// parse previous header, if any
		    _ParseHeader(key, val);
		done = 1;
		break;

	    // NEW HEADER?
	    default:
		if ( key != "" )		// parse previous header, if any
		   _ParseHeader(key, val);
		SplitKeyValue(s, key, val);
		break;
	}
    }
    fclose(fp);

    if ( messageid == "" ) { errmsg = "No 'Message-ID' field"; return(-1); }
    if ( from == ""      ) { errmsg = "No 'From' field";       return(-1); }

    valid = 1;
    return(0);
}

// LOAD ARTICLE NUMBER FROM CURRENT GROUP
//     Returns -1 on error, errmsg has reason.
//
int Article::Load(unsigned long num)
{
    if ( group == "" )
	{ errmsg = "No group selected"; return(-1); }

    return(Load(group.c_str(), num));
}

// SEND ARTICLE TO REMOTE VIA FD
//    Returns -1 on error, errmsg has reason.
//    head: 1=send header
//    body: 1=send body
//    If both head and body are 1, separator (blank line)
//    is also sent.
//
int Article::SendArticle(int fd, int head, int body)
{
    FILE *fp = fopen(filename.c_str(), "r");
    if ( fp == NULL )
    {
        errmsg = "article ";
	errmsg.append(ultos(number));
	errmsg.append(" no longer exists: ");
	errmsg.append(strerror(errno));
	return(-1);
    }

    char s[LINE_LEN];
    enum Mode { MODE_HEAD, MODE_SEP, MODE_BODY };
    Mode mode = MODE_HEAD;

    while ( fgets(s, sizeof(s)-2, fp) )
    {
        if ( mode == MODE_SEP )
	{
	    // MOVED OFF SEPARATOR INTO BODY
	    mode = MODE_BODY;
	}
	else if ( s[0] == '\n' && mode == MODE_HEAD )
	{
	    // END OF HEADER, AND NOT SENDING BODY? DONE
	    mode = MODE_SEP;			// end of header
	    if ( body == 0 ) { break; }		// not sending body? done
	}

	// LINE TOO LONG? -- TRUNCATE
	s[LINE_LEN-4] = '\n';
	s[LINE_LEN-3] = 0;

	char *ss = strpbrk(s, "\n\r");          // truncate at first occurance of \n or \r

	if ( ss )
	{
	    // TERMINATE WITH CRLF
	    *ss++ = '\r';
	    *ss++ = '\n';
	    *ss   = '\0';
	}

	if ( ( mode == MODE_HEAD && head ) ||
	     ( mode == MODE_BODY && body ) ||
	     ( mode == MODE_SEP && head && body ) )
	{
	    write(fd, s, strlen(s));
	    G_conf.LogMessage(L_DEBUG, "SEND: %s", s);
	}
    }
    fclose(fp);
    return(0);
}

int Article::SendHead(int fd)
{
    return(SendArticle(fd, 1, 0));
}

int Article::SendBody(int fd)
{
    return(SendArticle(fd, 0, 1));
}

// SANITIZE OVERVIEW FIELDS
//    RFC 2980 2.8 says tabs in fields must fold to a single space,
//    since tabs are field delimiters in XOVER's output.
//
static string SanitizeOverview(string& val)
{
    string s = val;
    replace(s.begin(), s.end(), '\t', ' ');
    return(s);
}

// RETURN NNTP OVERVIEW (RFC2980 2.8 "XOVER")
string Article::Overview(const char *overview[])
{
    string reply = ultos(Number());
    for ( int r=0; overview[r]; r++ )
    {
	// HEADER NAMES ARE CASE INSENSITIVE: INTERNET DRAFT (Son of RFC1036)
	if (!strcasecmp(overview[r], "Subject:")) 
	    { reply += string("\t") + SanitizeOverview(subject); }
	else if (!strcasecmp(overview[r], "From:"))
	    { reply += string("\t") + SanitizeOverview(from); }
	else if (!strcasecmp(overview[r], "Date:"))
	    { reply += string("\t") + SanitizeOverview(date); }
	else if (!strcasecmp(overview[r], "Message-ID:"))
	    { reply += string("\t") + SanitizeOverview(messageid); }
	else if (!strcasecmp(overview[r], "References:"))
	    { reply += string("\t") + SanitizeOverview(references); }
	else if (!strcasecmp(overview[r], "Lines:"))
	{
	    reply += "\t"; 
	    if ( lines > 0 )
		{ reply += ultos(lines); } 
	}
	else if (!strcasecmp(overview[r], "Bytes:"))
	{
	    reply += "\t";	// WE DONT KEEP TRACK OF BYTES -- LEAVE FIELD EMPTY
	}
	else if (!strcasecmp(overview[r], "Xref:full"))
	{
	    reply += "\t"; 
	    if ( xref != "" )
		{ reply += string("Xref: ") + SanitizeOverview(xref); }
	}
    }
    return(reply);
}
