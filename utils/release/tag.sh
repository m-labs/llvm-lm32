#!/bin/sh
#===-- tag.sh - Tag the LLVM release candidates ----------------------------===#
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License.
#
#===------------------------------------------------------------------------===#
#
# Create branches and release candidates for the LLVM release.
#
#===------------------------------------------------------------------------===#

set -e

release=""
rc=""

base_url="https://llvm.org/svn/llvm-project"

function usage() {
    echo "usage: `basename $0` -release <num>"
    echo "usage: `basename $0` -release <num> -rc <num>"
    echo " "
    echo "  -release <num>  The version number of the release"
    echo "  -rc <num>       The release candidate number"
}

function tag_version() {
    set -x
    for proj in llvm cfe dragonegg test-suite compiler-rt libcxx libcxxabi ; do
        if ! svn ls $base_url/$proj/branches/release_$release > /dev/null 2>&1 ; then
            svn copy -m "Creating release_$release branch" \
                $base_url/$proj/trunk \
                $base_url/$proj/branches/release_$release
        fi
    done
    set +x
}

function tag_release_candidate() {
    set -x
    for proj in llvm cfe dragonegg test-suite compiler-rt libcxx libcxxabi ; do
        if ! svn ls $base_url/$proj/tags/RELEASE_$release > /dev/null 2>&1 ; then
            svn mkdir -m "Creating release directory for release_$release." $base_url/$proj/tags/RELEASE_$release
        fi
        if ! svn ls $base_url/$proj/tags/RELEASE_$release/rc$rc > /dev/null 2>&1 ; then
            svn copy -m "Creating release candidate $rc from release_$release branch" \
                $base_url/$proj/branches/release_$release \
                $base_url/$proj/tags/RELEASE_$release/rc$rc
        fi
    done
    set +x
}

while [ $# -gt 0 ]; do
    case $1 in
        -release | --release )
            shift
            release=$1
            ;;
        -rc | --rc )
            shift
            rc=$1
            ;;
        -h | --help | -help )
            usage
            exit 0
            ;;
        * )
            echo "unknown option: $1"
            usage
            exit 1
            ;;
    esac
    shift
done

if [ "x$release" = "x" ]; then
    echo "error: need to specify a release version"
    echo
    usage
    exit 1
fi

release=`echo $release | sed -e 's,\.,,g'`

if [ "x$rc" = "x" ]; then
    tag_version
else
    tag_release_candidate
fi

exit 1
