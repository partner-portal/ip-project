#!/bin/bash

#
# Copyright (c) 2021-2023, ProvenRun and/or its affiliates.
# All rights reserved.
#
# @Author: Florian Lugou
# @Date:   January 28th, 2021
#

nl="
"

###############################################
# Parse parameters
###############################################

usage() {
    printf "Usage: compute the transitive closure in terms of dependencies from a set of libraries.\n"
    printf "    %s [options] <libs...>\n" "${BASH_SOURCE[0]}"
    printf "\n"
    printf "Positional arguments:\n"
    printf "    <libs>                    The set of libraries from which to start computing the\n"
    printf "                              transitive closure\n"
    printf "\n"
    printf "Options: [hV]\n"
    printf "        --create-as-needed    Create source and include directory softlinks and .dep files\n"
    printf "                              if they do not exist\n"
    printf "    -h, --help                Print this help and exit\n"
    printf "        --incpath=<dir>       Specify a path where the library headers can be found if\n"
    printf "                              --create-as-needed and --release are specified (can be\n"
    printf "                              specified multiple times)\n"
    printf "        --kernel=<dir>        Specify the path to the kernel sources\n"
    printf "        --libpath=<dir>       Specify a path where the library sources and headers can be\n"
    printf "                              found if --create-as-needed is specified (can be specified\n"
    printf "                              multiple times)\n"
    printf "        --release             Do not raise an error if a library sources directory is not\n"
    printf "                              found\n"
    printf "        --silent              Run silently\n"
    printf "        --variant=<var>       Specify the name of the release variant (optional)\n"
    printf "    -V, --version             Print the script version and exit\n"
}

version() {
    printf "%s Version 1.0\n" "${BASH_SOURCE[0]}"
}

eecho() {
    >&2 printf "%s\n" "$@"
}

CREATE_AS_NEEDED="0"
SILENT="0"
RELEASE="0"
LIBS=""
BASE_DIR="$(pwd)"
VARIANT=""
LIBPATHS=()
INCPATHS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -V|--version)
            version
            exit 0
            ;;
        --kernel=*)
            BASE_DIR="${1#*=}"
            ;;
        --kernel)
            BASE_DIR="$2"
            shift
            ;;
        --variant=*)
            VARIANT="${1#*=}"
            ;;
        --variant)
            VARIANT="$2"
            shift
            ;;
        --silent)
            SILENT="1"
            ;;
        --release)
            RELEASE="1"
            ;;
        --create-as-needed)
            CREATE_AS_NEEDED="1"
            ;;
        --libpath=*)
            LIBPATHS+=("${1#*=}")
            ;;
        --libpath)
            LIBPATHS+=("$2")
            shift
            ;;
        --incpath=*)
            INCPATHS+=("${1#*=}")
            ;;
        --incpath)
            INCPATHS+=("$2")
            shift
            ;;
        -*)
            eecho "Unknown option $1"
            exit 1
            ;;
        *)
            LIBS="${LIBS:+$LIBS$nl}$1"
            ;;
    esac
    shift
done

set -o pipefail             # trace ERR through pipes
set -o errtrace             # trace ERR through 'time command' and other functions
set -o errexit              # exit shell on error
set -o nounset              # exit the script if trying to use an uninitialised variable

export BUILD_BASE="${BUILD_BASE:-${BASE_DIR}/build/${VARIANT}}"

###############################################
# Helper functions
###############################################

excluded_from() {
    comm -13 <(printf "%s" "${1}") <(printf "%s" "${2}")
}

deduplicate_and_sort() {
    printf "%s" "${1}" | sort -u
}

create_softlinks() {
    local lib="$1"
    local srcdir="$BUILD_BASE/lib/src/$lib"
    local incdir="$BUILD_BASE/include/$lib"

    if [[ $CREATE_AS_NEEDED == 1 && ! ( -e "$srcdir" && -e "$incdir" ) ]]; then
        local target_srcdir target_incdir

        # Look for the library sources in the provided source directories
        for path in "${LIBPATHS[@]}"; do
            if [[ -d "$path/$lib" ]]; then
                target_srcdir="$path/$lib"
                if [[ -d "$path/$lib/include" ]]; then
                    target_incdir="$path/$lib/include"
                fi
                break
            fi
        done

        if [[ -z ${target_incdir:-} && $RELEASE == 1 ]]; then
            # Look for the library headers in the provided include directories
            for path in "${INCPATHS[@]}"; do
                if [[ -d "$path/$lib" ]]; then
                    target_incdir="$path/$lib"
                    break
                fi
            done
        fi

        if [[ ! -f "$BUILD_BASE/lib/lib$name.a" && ! -e "$srcdir" && -z ${target_srcdir:-} ]]; then
            eecho "error: $lib not found"
            exit 1
        fi

        local base=$(dirname "$lib")

        # Create a softlink to the source directory, if it exists
        if [[ ! ( -d "$srcdir" || -h "$srcdir" ) && ! -z ${target_srcdir:-} ]]; then
            mkdir -p "$BUILD_BASE/lib/src/$base"
            ln -snf "$target_srcdir" "$srcdir"
        fi

        # Create a softlink to the include directory, if it exists
        if [[ ! ( -d "$incdir" || -h "$incdir" ) && ! -z ${target_incdir:-} ]]; then
            mkdir -p "$BUILD_BASE/include/$base"
            ln -snf "$target_incdir" "$incdir"
        fi
    fi
}

create_dep_file() {
    local lib="$1"
    local name=$(basename "$lib")

    # Create softlinks if --create-as-needed has been specified
    create_softlinks "$lib"

    local depfile="$BUILD_BASE/lib/.${lib}.dep"

    if [[ $CREATE_AS_NEEDED == 1 && ! -f "$depfile" ]]; then
        # Try to generate the .dep file
        make -s -C "$BUILD_BASE/lib/src/$lib" \
            LIBNAME=$name \
            BASE_DIR="$BASE_DIR" \
            VARIANT="$VARIANT" \
            "$depfile" > /dev/null
    fi

    if [[ ! -f "$depfile" ]]; then
        eecho "Missing file $depfile"
        exit 1
    fi
}

###############################################
# Traverse the dependency graph
###############################################

visited=""
LIBS=$(deduplicate_and_sort "$LIBS")
while [[ ! -z $LIBS ]]; do
    # Pop the first library to visit
    lib="${LIBS%%${nl}*}"
    LIBS="${LIBS:$((${#lib}+1))}"
    visited=$(deduplicate_and_sort "${visited:+$visited$nl}$lib")

    create_dep_file "$lib"

    # Add its dependencies to the libraries to visit,
    # if they have not yet been visited
    libdep=$(cat "$BUILD_BASE/lib/.${lib}.dep")
    LIBS="${LIBS:+$LIBS$nl}$libdep"
    LIBS=$(deduplicate_and_sort "$LIBS")
    LIBS=$(excluded_from "$visited" "$LIBS")
done

if [[ $SILENT == 0 ]]; then
    for lib in $visited; do
        # In release mode, only display libraries shipped as source
        if [[ $RELEASE == 0 || -e "$BUILD_BASE/lib/src/$lib" ]]; then
            printf "%s\n" "$lib"
        fi
    done
fi
