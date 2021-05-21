#!/bin/sh

TOP_DIR=$(dirname $0)
LAST_DIR=$PWD

if test ! -f $TOP_DIR/configure.ac ; then
    echo "You must execute this script from the top level directory."
    exit 1
fi

AUTOCONF=${AUTOCONF:-autoconf}
ACLOCAL=${ACLOCAL:-aclocal}
AUTOHEADER=${AUTOHEADER:-autoheader}

run_or_die ()
{
    COMMAND=$1

    # check for empty commands
    if test -z "$COMMAND" ; then
        echo "*warning* no command specified"
        return 1
    fi

    shift;

    OPTIONS="$@"

    # print a message
    printf "*info* running %s" "$COMMAND"
    if test -n "$OPTIONS" ; then
        echo " ($OPTIONS)"
    else
        echo
    fi

    # run or die
    $COMMAND $OPTIONS ; RESULT=$?
    if test $RESULT -ne 0 ; then
        echo "*error* $COMMAND failed. (exit code = $RESULT)"
        exit 1
    fi

    return 0
}

if test X`uname` = XOpenBSD ; then
    # OpenBSD's autoconf and automake require the AUTOCONF_VERSION and
    # AUTOMAKE_VERSION environment variables be set.  For autoconf,
    # there's some documentation for that here:
    # https://www.openbsd.org/faq/ports/specialtopics.html#Autoconf
    # Grab the version from pkg_info's output using the last version it
    # reports about if there are multiple versions installed.
    if test -z "$AUTOCONF_VERSION" ; then
        version=`pkg_info autoconf | grep '^Information for inst:autoconf-' | tail -n 1 | cut -d - -f 2`
        version_major=`echo " $version" | cut -d . -f 1 | tr -d ' \n\r\t'`
        version_minor=`echo " $version" | cut -d . -f 2 | cut -d p -f 1 | tr -d ' \n\r\t'`
        if test -n "$version_major" -a -n "$version_minor" ; then
            AUTOCONF_VERSION="$version_major"."$version_minor"
            export AUTOCONF_VERSION
        fi
    fi
    if test -z "$AUTOMAKE_VERSION" ; then
        version=`pkg_info automake | grep 'Information for inst:automake-' | tail -n 1 | cut -d - -f 2`
        version_major=`echo " $version" | cut -d . -f 1 | tr -d ' \n\r\t'`
        version_minor=`echo " $version" | cut -d . -f 2 | cut -d p -f 1 | tr -d ' \n\r\t'`
        if test -n "$version_major" -a -n "$version_minor" ; then
            AUTOMAKE_VERSION="$version_major"."$version_minor"
            export AUTOMAKE_VERSION
        fi
    fi
    echo "OpenBSD; using AUTOCONF_VERSION=$AUTOCONF_VERSION AUTOMAKE_VERSION=$AUTOMAKE_VERSION"
fi

cd $TOP_DIR

run_or_die $ACLOCAL -I m4
run_or_die $AUTOHEADER
run_or_die $AUTOCONF

cd $LAST_DIR
