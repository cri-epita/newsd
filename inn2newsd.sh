#!/bin/sh
#
# Simple script to convert an INND spool directory into a Newsd spool
# directory.
#
# Usage:
#
#    inn2newsd.sh inn-spooldir newsd-spooldir
#

if test $# != 2; then
	echo Usage: $0 inn-spooldir newsd-spooldir
	exit 1
fi

if test ! -d "$1"; then
	echo $0: INND directory $1 does not exist!
	exit 1
fi

if test ! -d "$1/articles"; then
	echo $0: INND directory $1 does not contain an articles subdirectory!
	exit 1
fi

if test ! -d "$2"; then
	echo $0: Newsd directory $2 does not exist!
	exit 1
fi

inndspool="$1/articles"
newsdspool="$2"

cd "$inndspool"

if test "$inndspool" != "$newsdspool"; then
	echo Copying INND articles to Newsd...
	cp -r * "$newsdspool"
	chown -R news:news "$newsdspool"
	chmod -R ugo+r,u+w,go-w "$newsdspool"
	echo Copy complete!
fi

cd "$newsdspool"

echo Creating Newsd groups...
for group in `find . -type d -print`; do
	case "$group" in
		*/announce | */commit | */cvs)
		echo postok 0 >$group/.config
		;;

		*)
		echo postok 1 >$group/.config
		;;
	esac
done
