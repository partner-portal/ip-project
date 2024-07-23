#!/bin/bash
#
# @Author: Henri Chataing
# @Date:   June 13th, 2018
#
# Arguments are var / header
# CFLAGS is passed to gcc

binfile=$(mktemp)

trap 'rm -f $binfile' ERR

echo "
#include <$2>
#include <stdio.h>

int main(void) {
    printf(\"%d\", $1);
    return 0;
}
" | gcc -W -Wall -Wextra $CFLAGS -o $binfile -xc -

$binfile
rm $binfile
