#!/bin/bash

set -euo pipefail

function usage() {
    echo "Usage: Get the last modification time of a tracked ProvenCore file."
    echo "       If the file modified or not tracked, fall-back to current date."
    echo "       The output format in an UNIX timestamp"
    echo ""
    echo "    $0 <file>"
    echo ""
    echo "Options: [--round-date]"
    echo "    -h, --help                Print this help and exit"
    echo "    --round-date              Truncate the modification date to"
    echo "                               the date day"
}

function eecho() {
    >&2 echo $@
}

FILE=""
ROUND_TO_DAY="0"

while [[ $# -gt 0 ]]
do
case $1 in
    -h|--help)
        usage
        exit 0
        ;;
    --round-date)
        ROUND_TO_DAY="1"
        ;;
    *)
        if [ -f "$1" ]; then
            if [ "$FILE" == "" ]; then
                FILE=$(realpath "$1")
            else
                eecho "$0: Multiples files are not supported ('$1')"
                exit 1
            fi
        else
            eecho "$0: File '$1' not found"
            exit 1
        fi
        ;;
esac
shift
done

# Move into file folder
cd $(dirname "${FILE}")

if hg identify &> /dev/null; then
    TRACKED_STATE=$(hg status -A ${FILE})
    TRACKED_STATE=${TRACKED_STATE:0:1}
else
    # No hg repository here. Let us consider we have an untracked file.
    TRACKED_STATE="?"
fi

case ${TRACKED_STATE} in
    "C")
        FILE_MOD_DATE=$(hg log -r "last(::. and filelog(\"${FILE}\"))" --template '{date}\n' | cut -f 1 -d.)
        ;;
    ?|A|I|M)
        # Untracked or modified file, use current date
        FILE_MOD_DATE=$(date +%s)
        ;;
    *)
        eecho "Unknown tracking state '${TRACKED_STATE}', falling back to current date"
        FILE_MOD_DATE=$(date +%s)
esac

if [ ${ROUND_TO_DAY} -ne 0 ]; then
    DAY_OFFSET=$((${FILE_MOD_DATE} % 86400))
    FILE_MOD_DATE=$((${FILE_MOD_DATE} - ${DAY_OFFSET}))
fi

echo ${FILE_MOD_DATE}
