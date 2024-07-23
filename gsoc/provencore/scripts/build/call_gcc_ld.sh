#!/bin/bash

# Convert the arguments of a call to 'ld' in order to call gcc for linking.
# Some of the parameters need to be prefixed with [-Wl,] while others can be
# ignored.
#
# This script is used in an instance where either gcc and ld will be called
# for linking a binary, and the options are mingled.
#
# e.g. the path to libgcc is known only to gcc ; gcc must be used in order
# to create a binary using that library.
#
# Usage:
#   ./call_gcc_ld.sh GCC [options..]
# where GCC is the path to / name of the gcc binary to execute, and
# the additional options are all ld call parameters.

binary="$1"
newargs=""
shift

while [[ $# -gt 0 ]]
do
case $1 in
    "--gc-sections"|"--print-gc-sections"|-Map=*|-b*|--build-id=*)
        arg="-Wl,$1"
        ;;

    "-("|"-)"|"--start-group"|"--end-group"|"--whole-archive"|"--no-whole-archive")
        arg="-Wl,$1"
        ;;

    "--script")
        arg="-T $2"
        shift
        ;;

    "-L"|"-l"|"-o"|"-z"|"-T")
        arg="$1 $2"
        shift
        ;;

    "-nostdlib"|-L*|-l*|*.o|"--target=*")
        arg="$1"
        ;;

    *)
        >&2 echo "warning: unrecognized ld option [$1]"
        arg="$1"
        ;;
esac
newargs="$newargs $arg"
shift
done

"${binary}" -Wl,--build-id=none $newargs
