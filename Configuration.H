//
// Configuration.H -- News Configuration Class
//
// Copyright 2003-2004 Michael Sweet
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <sys/types.h>

// Log levels...
enum
{
    L_ERROR,				// Show error messages
    L_INFO,				// Show error + info messages
    L_DEBUG				// Show error + info + debug messages
};

// This class holds all of the global configuration information...
class Configuration
{
    int		hostnamelookups;	// do hostname lookups?
    struct sockaddr_in listen;		// Listen address
    string	errorlog;		// Log file
    int		loglevel;		// Log level
    FILE	*log;			// Stream for logging
    ino_t	log_ino;		// Inode # for log (detects rotation)
    long	maxlogsize;		// maximum size of log in bytes (0=unlimited)
    unsigned	maxclients;		// maximum number of child processes
    string	sendmail;		// sendmail command
    string	servername;		// news server hostname
    string	spamfilter;		// spam filter command
    string	spooldir;		// spool directory
    unsigned	timeout;		// #secs timeout after inactivity
    string	user;			// user to run as

    uid_t	uid;			// user ID
    gid_t	gid;			// group ID
    bool	bad_user;		// true means username was not found

    int		lookup_user(const char *name);
    int		OpenLogAppend();
    bool	WasLogRotated();
    void	FixNewsLogDir(const char* newslogdir);

public:

    Configuration();

    // Load config data from a file...
    void Load(const char *conffile);

    // Log a message to the current log file...
    void LogMessage(int level, const char *message, ...);

    // Log settings to current log file...
    void LogSelf(int loglevel);

    // Get/set the current LogFile option...
    void ErrorLog(const char *f) { errorlog = f; }
    const char *ErrorLog() { return (errorlog.c_str()); }

    // Get/set the current HostnameLookups option...
    void HostnameLookups(int h) { hostnamelookups = h; }
    int HostnameLookups() { return (hostnamelookups); }

    // Get/set the current Listen/Port option...
    void Listen(const char *l);
    void Listen(int p);
    struct sockaddr_in *Listen() { return (&listen); }

    // Get/set the current LogLevel option...
    void LogLevel(int l) { loglevel = l; }
    int LogLevel() { return(loglevel); }

    // Get/set the current MaxClients option...
    void MaxClients(unsigned val) { maxclients = val; }
    unsigned MaxClients() { return (maxclients); }

    // Get/set the current MaxLogSize option...
    void MaxLogSize(long val) { maxlogsize = val; }
    void MaxLogSize(const char *val);
    long MaxLogSize() { return (maxlogsize); }

    // Get/set the current ServerName option...
    void ServerName(const char *h) { servername = h; };
    const char *ServerName() { return (servername.c_str()); }

    // Get/set the current Sendmail option...
    void SendMail(const char *c) { sendmail = c; };
    const char *SendMail() { return (sendmail.c_str()); }

    // Get/set the current SpamFilter option...
    void SpamFilter(const char *d) { spamfilter = d; };
    const char *SpamFilter() { return (spamfilter.c_str()); }

    // Get/set the current SpoolDir option...
    void SpoolDir(const char *d) { spooldir = d; };
    const char *SpoolDir() { return (spooldir.c_str()); }

    // Get/set the current Timeout option...
    void Timeout(unsigned val) { timeout = val; }
    unsigned Timeout() { return (timeout); }

    // Get/set the current User option...
    void User(const char *u) { user = u; lookup_user(u); };
    const char *User() { return (user.c_str()); }

    // Get the current user/group ID
    uid_t UID() const { return (uid); }
    gid_t GID() const { return (gid); }
    bool BadUser() const { return (bad_user); }

    // Log related methods
    string OldLogFilename();
    int LogUnlock();
    int LogLock();
    int Rotate(bool force);
    void DateStampedMessage(FILE *fp, const char *msg);
    void InitLog();
};

extern Configuration G_conf;

#endif /*!CONFIGURATION_H*/
