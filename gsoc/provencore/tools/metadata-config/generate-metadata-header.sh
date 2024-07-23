#!/bin/bash

set -e
set -u
set -o pipefail

function usage() {
    echo "Usage: generate the file metadata index file."
    echo "    ${BASH_SOURCE[0]} -o <file> [-I <dir>]+ <binaries>"
    echo ""
    echo "Options: [o:I:h]"
    echo "    -o, --output=<dir>        Specify the output file"
    echo "    -I, --include<dir>        Specify an additional path where to look"
    echo "                              for metadata header files"
    echo "    -h, --help                Print this help and exit"
}

header_file=""
include_paths=""
binaries=""

while [[ $# -gt 0 ]]
do
case $1 in
    -h|--help)
        usage
        exit 0
        ;;
    -o)
        header_file="$2"
        shift
        ;;
    --output=*)
        header_file="${1#*=}"
        ;;
    -I)
        include_paths="${include_paths} $2"
        shift
        ;;
    --include=*)
        include_paths="${include_paths} ${1#*=}"
        ;;
    *)
        binaries="${binaries} $1"
        ;;
esac
shift
done

rm -f ${header_file}

header="\
#ifndef METADATA_H_INCLUDED
#define METADATA_H_INCLUDED

#include <stddef.h>

"

binaries_paths=${binaries//build\//}
binaries_names=${binaries//\//_}
binaries_names=${binaries_names//-/_}
binaries_upper=${binaries_names^^}

binaries_orig=($binaries)
binaries_paths=($binaries_paths)
binaries_names=($binaries_names)
binaries_upper=($binaries_upper)
binaries_count=${#binaries_paths[@]}
final_binaries_count=0

##
# $1 is the name of the metadata header which we are looking for
##
function metadata_header_exists() {
    for f in $include_paths; do
        if [ -f "${f}/${1}" ]; then
            return 0
        fi
    done
    return 1
}

for ((i=0; i<${binaries_count}; i++))
do
    if metadata_header_exists ${binaries_paths[$i]}_metadata.h; then
        header="${header}#include <metadata/${binaries_paths[$i]}_metadata.h>\n"
        ((final_binaries_count++)) || true
    fi
done


header="${header}\nstatic const char *application_path[] = {\n"

for ((i=0; i<${binaries_count}; i++))
do
    if metadata_header_exists ${binaries_paths[$i]}_metadata.h; then
        header="${header}    \"${binaries_orig[$i]}\",\n"
    fi
done


header="${header}\
};

typedef struct {
    char const **keys;
    char const **values;
    size_t nr;
} metadata_dict;

static const metadata_dict application_metadata[] = {\n"

for ((i=0; i<${binaries_count}; i++))
do
    if metadata_header_exists ${binaries_paths[$i]}_metadata.h; then
        header="${header}\
    {
        .keys = ${binaries_names[$i]}_metadata_keys,
        .values = ${binaries_names[$i]}_metadata_values,
        .nr = ${binaries_upper[$i]}_METADATA_NR
    },\n"
    fi
done


header="${header}\
};

#define APPLICATION_METADATA_NR ((size_t)${final_binaries_count})

#endif /* METADATA_H_INCLUDED */"

echo -e "${header}" > "${header_file}"
