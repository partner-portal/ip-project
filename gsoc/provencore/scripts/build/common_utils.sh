# $1 is path/to/file
# $2 is the offset
function get_uint64_from_file {
# xxd doesn't seem widely available with the -e option
#    DWORD="0x$(xxd -g8 -e -l $2 $1 | awk '{print $2}')"
    local LOWER=$(hexdump -s $2            -n 4 -e '"%08x"' $1)
    local UPPER=$(hexdump -s `expr $2 + 4` -n 4 -e '"%08x"' $1)
    local DWORD="0x$UPPER$LOWER"
    echo "$DWORD"
}


# $1 is path/to/kernel.bin
function get_kernel_version {
    echo $(get_uint64_from_file $1 8)
}

# $1 is path/to/kernel.bin
function get_kernel_size {
    echo $(get_uint64_from_file $1 16)
}

# $1 is path/to/codes.bin
function get_codes_version {
    echo $(get_uint64_from_file $1 0)
}
