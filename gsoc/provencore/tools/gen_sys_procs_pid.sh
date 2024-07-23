#!/bin/bash

# @Author: Henri Chataing
# @Date:   March 13th, 2018

function usage() {
    echo "Usage: generate sys_procs_pid.h header from DTB kernel config."
    echo "    ./gen_sys_procs.sh <dt file> -o <dir>"
    echo ""
    echo "Options: [o:hV]"
    echo "    -o, --output-dir <dir>    Specify the output directory"
    echo "    -h, --help                Print this help and exit"
    echo "    -V, --version             Print the script version and exit"
}

function eecho() {
    >&2 echo $@
}

INPUT=""
OUTPUT=""

while [[ $# -gt 0 ]]
do
case $1 in
    -h|--help)
        usage
        exit 0
        ;;
    -V|--version)
        version
        exit 0
        ;;
    -o)
        OUTPUT="$2"
        shift
        ;;
    --output-dir=*)
        OUTPUT="${1#*=}"
        ;;
    *)
        INPUT="$1"
        ;;
esac
shift
done

if [ -z "$INPUT" ]; then
    eecho "Missing input file"
    usage
    exit 1
fi

if [ -z "$OUTPUT" ]; then
    eecho "Missing output directory"
    usage
    exit 1
fi

if [ ! -d "$OUTPUT" ]; then
    eecho "Invalid output directory"
    usage
    exit 1
fi

HEADER=$OUTPUT/sys_procs_pid.h

echo \
"/*
 * Copyright (c) 2020-2023 ProvenRun S.A.S
 * All Rights Reserved.
 */
#ifndef _SYS_PROCS_PID_H_INCLUDED_
#define _SYS_PROCS_PID_H_INCLUDED_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
" > $HEADER

PROCS=$(fdtget -l $INPUT "/processes")

# Number of kernel tasks. Keep the variable same so it is easier to grep for
#  (it is the name used in the kernel code).
NR_TASKS=3

procnr=0

function genproc() {
    if [ "$1" == "user" ]; then
        return 0
    fi
    name="$1"
    pid=$(((${procnr} + ${NR_TASKS}) * 8))
    pid=$(printf '0x%x' $pid)
    printf "#define %-32s    UINT64_C(${pid})\n" "${name^^}_ID" >> $HEADER
    procnr=$((${procnr} + 1))
    return 0
}

for proc in ${PROCS}; do
    genproc $proc
    if [ "$?" -ne "0" ]; then
        exit 1
    fi
done

echo "" >> $HEADER
echo "#define SYS_PROCS_NUM ((size_t)UINTMAX_C(${procnr}))" >> $HEADER
echo "#define KCONFIG_SYS_PROCS_ARRAY \\" >> $HEADER
echo "    { \\" >> $HEADER

for proc in ${PROCS}; do
    if [ ! "$proc" == "user" ]; then
        echo "        ${proc^^}_ID, \\" >> $HEADER
    fi
done

echo "    }" >> $HEADER
echo "" >> $HEADER
echo "#endif /* _SYS_PROCS_PID_H_INCLUDED_ */" >> $HEADER
