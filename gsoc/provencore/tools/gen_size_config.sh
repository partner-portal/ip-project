#!/bin/bash

# @Author: Henri Chataing
# @Date:   March 13th, 2018

function usage() {
    echo "Usage: generate size_config.h header from DTB kernel config."
    echo "    ./gen_size_config.sh <dt file> -o <dir>"
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

HEADER=$OUTPUT/size_config.h

function getconfig()
{
    prop=$(fdtget $INPUT "/" $1 2> /dev/null)
    if [ "$?" -ne "0" ]; then
        eecho "Missing root property $1"
        exit 1
    fi
    echo "#define KCONFIG_$2 (${prop})" >> $HEADER
}

function getconfig_pages()
{
    prop=$(fdtget $INPUT "/" $1 2> /dev/null)
    if [ "$?" -ne "0" ]; then
        eecho "Missing root property $1"
        exit 1
    fi
    prop=$((${prop} * 4096))
    prop=$(printf "0x%x" ${prop})
    echo "#define KCONFIG_$2 (${prop})" >> $HEADER
}

echo \
"/*
 * Copyright (c) 2020-2023 ProvenRun S.A.S
 * All Rights Reserved.
 */
#ifndef _SIZE_CONFIG_H_INCLUDED_
#define _SIZE_CONFIG_H_INCLUDED_
" > $HEADER

getconfig "nr-irq-hooks" "NR_IRQ_HOOKS"
getconfig "nr-procs" "NR_PROCS"

procs=$(fdtget $INPUT -l "/processes" 2> /dev/null)
if [ "$?" -ne "0" ]; then
    eecho "Missing /processes child node"
    exit 1
fi
procs=(${procs})
nrprocs=$((${#procs[@]} + 4))
echo "#define KCONFIG_NR_SYS_PROCS (${nrprocs})" >> $HEADER

getconfig "nr-regions" "REGION_POOL_SIZE"
getconfig "nr-proc-stack-pages" "NR_STACK_PAGES"
getconfig "nr-proc-auths" "NR_AUTHS"
getconfig "nr-proc-shms" "NR_SHM"
getconfig "nr-shms" "NR_SHM_POOL_SIZE"
getconfig "nr-sched-queues" "NR_SCHED_QUEUES"
getconfig "nr-dyn-elfs" "NR_DYNAMIC_PROGRAMS"

getconfig_pages "nr-dyn-elf-pages" "DYNAMIC_ARCHIVE_SPACE"
getconfig_pages "nr-dma-pages" "PBUFF_SIZE"

echo "" >> $HEADER
echo "#define SAFETY_PAGES (4U)" >> $HEADER

echo \
"
#endif /* _SIZE_CONFIG_H_INCLUDED_ */" >> $HEADER
