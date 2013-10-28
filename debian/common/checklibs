#! /bin/sh 
#                               -*- Mode: Sh -*- 
# checklibs.sh --- 
# Author           : Manoj Srivastava ( srivasta@glaurung.internal.golden-gryphon.com ) 
# Created On       : Fri Sep 29 15:36:22 2006
# Created On Node  : glaurung.internal.golden-gryphon.com
# Last Modified By : Manoj Srivastava
# Last Modified On : Fri Sep 29 22:53:27 2006
# Last Machine Used: glaurung.internal.golden-gryphon.com
# Update Count     : 43
# Status           : Unknown, Use with caution!
# HISTORY          : 
# Description      : 
# 
# arch-tag: 8ba11489-77fa-45a0-92c4-9c5b162ee119
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 

# Make sure we abort on error
set -e
progname="$(basename \"$0\")"

trap 'rm -f search_patterns.txt;' ALRM HUP INT PIPE TERM ABRT FPE BUS QUIT SEGV ILL EXIT

# Find all undefined symbols in all ELF objects in this tree
readelf -s -D -W $(find . -type f -print0 | xargs -0r  file | grep " ELF" | \
 awk '{print $1}' | sed -e 's/:$//') | grep UND | grep -v LOCAL |
  perl -ple 's/.*\s(\S+)\s*$/\^$1\$/g' | sort -u > search_patterns.txt;

# Find all the libraries needed in this tree 
objdump -T --private-headers $(find . -type f -print0  | xargs -0r file | grep " ELF" | \
  awk '{print $1}' | sed -e 's/:$//') | grep NEEDED | sort -u | awk '{print $2}' | 
    while read lib; do
        # For each library, see where it lives o the file system
        LIB=
        for library_dir in "/lib" "/usr/lib" $EXTRA_LIBRARY_PATHS; do
            if [ -e "$library_dir/$lib" ]; then
                 LIB="$library_dir/$lib";
                 break
            fi
        done
        if [ -z "$LIB" ]; then
            echo >&2 "Can't find $lib"
            continue
        fi
        # If we fond the library, find what symbols it defines, and if these symbols
        # are some that we need
        if readelf -s -D -W $LIB | grep -v UND | perl -ple 's/.*\s(\S+)\s*$/$1/g' | \
            sort -u | grep -q -f search_patterns.txt ; then
            # Library provides at least some symbols we need
            if [ -n "$DEBUG" ]; then echo "Found $LIB"; fi
        else
            # Library does not provide any symbols we need
            echo "$LIB" ;
        fi
done

# Get rid of the intermediate file
rm -f search_patterns.txt;
exit 0

