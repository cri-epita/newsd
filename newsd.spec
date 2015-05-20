#
# "$Id: newsd.spec 122 2005-10-19 17:50:17Z mike $"
#
# RPM "spec" file for newsd.
#
# Copyright 2003-2005 Michael Sweet
# Copyright 2002 Greg Ercolano
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public Licensse as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

Summary: newsd usenet news server
Name: newsd
Version: 1.44
Release: 1
Copyright: GPL
Group: System Environment/Daemons
Source: http://ftp.easysw.com/pub/newsd/newsd-%{version}-source.tar.gz
Url: http://www.easysw.com/~mike/newsd/
Packager: Anonymous <anonymous@foo.com>
Vendor: Greg Ercolano and Michael Sweet

# Use buildroot so as not to disturb the version already installed
BuildRoot: /var/tmp/%{name}-root

%description
Newsd is a standalone local NNTP news server for private
newsgroup serving on a single server.  It is useful for serving
private newsgroup(s) to an intranet or the Internet and can act
as a simple mail gateway, however it does not interface with
other news servers and cannot manage distributed news feeds,
i.e. Usenet news.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" LDFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr

# If we got this far, all prerequisite libraries must be here.
make

%install
# Make sure the RPM_BUILD_ROOT directory exists.
rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

%preun
if test -f /sbin/init.d/newsd; then
	/sbin/init.d/newsd stop
fi
if test -f /etc/rc.d/init.d/newsd; then
	/etc/rc.d/init.d/newsd stop
fi
if test -f /etc/init.d/newsd; then
	/etc/init.d/newsd stop
fi

if test -x /sbin/chkconfig; then
	/sbin/chkconfig --del newsd
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%config(noreplace) /etc/newsd.conf

# RC dirs are a pain under Linux...  Uncomment the appropriate ones if you
# don't use Red Hat or Mandrake...

# OLD RedHat/Mandrake
/etc/rc.d/init.d/*
/etc/rc.d/rc0.d/*
/etc/rc.d/rc2.d/*
/etc/rc.d/rc3.d/*
/etc/rc.d/rc5.d/*

#/sbin/rc.d/*
#/sbin/rc.d/rc0.d/*
#/sbin/rc.d/rc2.d/*
#/sbin/rc.d/rc3.d/*
#/sbin/rc.d/rc5.d/*

# NEW RedHat/Mandrake
#/etc/init.d/*
#/etc/rc0.d/*
#/etc/rc2.d/*
#/etc/rc3.d/*
#/etc/rc5.d/*

/usr/sbin/*
%dir /usr/share/doc/newsd
/usr/share/doc/newsd/*
/usr/share/man/*

%dir /var/spool/newsd


#
# End of "$Id: newsd.spec 122 2005-10-19 17:50:17Z mike $".
#
