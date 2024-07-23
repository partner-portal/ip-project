#!/bin/bash

# Xterm configuration
# -------------------
#
# Default Xterm configuration might not look as you would like.
# For that, it is recommended to configure them the following way:
#
# 1. Use a text editor and create/open ~/.Xresources
# 2. Write the Xterm configuration, as an example:
#    ===================================================
#    !XTerm*faceName:     Inconsolata:antialias=true
#    XTerm*faceName:     DejaVuSansMono:antialias=true
#    !XTerm*faceName:     FreeMonoBold:antialias=true
#    XTerm*faceSize:     9
#    XTerm*faceSize1:    8
#    XTerm*faceSize2:    9
#    XTerm*faceSize3:    10
#    XTerm*faceSize4:    12
#    XTerm*faceSize5:    16
#    XTerm*faceSize6:    24
#
#    XTerm*background:black
#    XTerm*foreground:white
#    ===================================================
#    Consult the man page of Xterm for more information
# 3. Run "xrdb -merge ~/.Xresources" to take the modification into account

set -euo pipefail
# cd to ProvenCore root directory. Easier for the rest of the script.
cd $(dirname $0)
cd "../../"

##
# Print usage information.
##
function usage() {
    echo "Usage: $1 [-n N] [-o stdio|term|tty:name|file:name] [...]"
    echo ""
    echo "  Start a qemu session with the requested options, redirecting uart channels to"
    echo "  various outputs."
    echo "  By default, the first uart is directed to 'stdio', and any additional channel"
    echo "  is directed to a separate 'xterm'. This behavior can be modified using the -o flag."
    echo ""
    echo "       -h,   --help              Display usage information"
    echo "       -q,   --quiet             Disable debug messages"
    echo "       -b V, --build=V           Select a provencore build variant"
    echo "       -r,   --dry-run           Print the command to execute without launching qemu"
    echo "       -d,   --debug             Open a gdb connection on tcp::1234 and pause the cpu at startup"
    echo "       -S,   --semihosting       Enable -semihosting option"
    echo "       -g,   --nographic         Enable -nographic option"
    echo "       -n N, --nr-uarts=N        Select the number of uart devices to spawn (default 1)"
    echo "       -o D, --output=D          Select the serial device for the nth uart device"
    echo "                                 Supported modes are:"
    echo "                                     stdio     : pair uart with host terminal"
    echo "                                     term      : open a fresh xterm window"
    echo "                                     tty:NAME  : open a named xterm window"
    echo "                                     file:NAME : write output to file"
    echo "                                 Unconfigured uarts will be directed to a terminal (term mode)"
    echo "       -s C, --sdcard=C          Open C as an SD card storage"
    echo "       -t T, --tap=T             Use specified tap device for networking"
    echo "       --deterministic           Align the timer on the instruction count, not the host timer"
    echo "       --virtio-net-tapX-brY=P   Bind VIRTIO port P to tapX on top of brY for network"
    echo "       --virtio-net-user=P       Bind VIRTIO port P to user for network"
    echo ""
    echo "Note on terminal used to launch QEMU:"
    echo " This script uses xterm, if the default fonts are not pleasant for you,"
    echo " you can configure xterm. Read the comments at the start of this script to see how."
}

##
# Return the path to the qemu binary for the selected architecture
# $1: architecture
##
function find_qemu() {
    if type qemu-system-$1 &>/dev/null; then
        echo "qemu-system-$1"
        return 0
    elif [ ! -z ${QEMUPATH+x} ]; then
        if type $QEMUPATH/qemu-system-$1 &>/dev/null; then
            echo "$QEMUPATH/qemu-system-$1"
            return 0
        fi
    fi
    echo ""
    return 0
}

##
# Read the architecture of the current build.
# Note that armv7a is converted to arm, armv8 to aarch64
# $1: name of the variant build if specified on the command line
##
function get_arch() {
    local configs_dir=$(VARIANT="$1" MKFILE="Makefile" \
        make -s -f "scripts/Makefile.print" print-CONFIGS_DIR)
    local var=$(BASE_DIR="." CONFIGS_DIR="${configs_dir}" \
                MKFILE="build/$1/kconfig/target.mk" \
                make -s -f "scripts/Makefile.print" print-KARCH)
    case $var in
        armv7a)
            echo "arm"
            ;;
        armv8)
            echo "aarch64"
            ;;
        *)
            echo $var
            ;;
    esac
    return 0
}

# $1 is path/to/file
# $2 is the offset
function get_uint64_from_file() {
# xxd doesn't seem widely available with the -e option
#    DWORD="0x$(xxd -g8 -e -l $2 $1 | awk '{print $2}')"
    local lower=$(hexdump -s $2            -n 4 -e '"%08x"' $1)
    local upper=$(hexdump -s `expr $2 + 4` -n 4 -e '"%08x"' $1)
    echo "0x$upper$lower"
}

# $1 is path/to/kernel.bin
function get_kernel_version() {
    echo $(get_uint64_from_file $1 8)
}

# $1 is path/to/codes.bin
function get_codes_version() {
    echo $(get_uint64_from_file $1 0)
}

variant=""
quiet="0"
debug="0"
semihosting="0"
nographic="0"
dryrun="0"
nruarts="1"
output=()
sdcard=""
tap=""
deterministic="0"
script="$0"
virtios=("" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "")
badargs="false"

while [[ $# -gt 0 ]]
do
case $1 in
    -h|--help)
        usage "$script"
        exit 0
        ;;
    -q|--quiet)
        quiet="1"
        ;;
    -d|--debug)
        debug="1"
        ;;
    -S|--semihosting)
        semihosting="1"
        ;;
    -g|--nographic)
        nographic="1"
        ;;
    -r|--dry-run)
        dryrun="1"
        ;;
    -b)
        variant="$2"
        shift
        ;;
    --build=*)
        variant="${1#*=}"
        ;;
    -m)
        memory="$2"
        shift
        ;;
    -n)
        nruarts="$2"
        shift
        ;;
    --nr-uarts=*)
        nruarts="${1#*=}"
        ;;
    -o)
        output+=("$2")
        shift
        ;;
    --output=*)
        output+=("${1#*=}")
        ;;
    -s)
        sdcard="$2"
        shift
        ;;
    --sdcard=*)
        sdcard="${1#*=}"
        ;;
    -t)
        tap="$2"
        shift
        ;;
    --tap=*)
        tap="${1#*=}"
        ;;
    --deterministic)
        deterministic="1"
        ;;
    --virtio-*=*)
        index="${1#*=}"
        if [ -z "${index}" ] || [ "${index}" -lt 0 ] || [ "${index}" -gt 31 ]
        then
            badargs="true"
            printf "Missing VIRTIO device number (0-31) in \"%s\"\n" "${1}"
        else
            if ! [ -z "${virtios[${index}]}" ]
            then
                badargs="true"
                printf "Collision on VIRTIO%s (\"%s\")\n" "${index}" "${1}"
            else
                virtiodev="${1%%=*}"
                virtios[${index}]="${virtiodev#--virtio-}"
            fi
        fi
        ;;
    *)
        badargs="true"
        echo "Unexpected option: $1"
        ;;
esac
shift
done

if [ "${badargs}" = "true" ]
then
    printf "Bad parameters, aborting\n"
    exit 1
fi

# Option checks
if [ "$nruarts" -le "0" ]; then
    echo "Invalid number of uarts: $nruarts"
    exit 1
fi

if [ "${#output[*]}" -le "0" ]; then
    output+=("stdio")
fi

if [ "$nruarts" -lt "${#output[*]}" ]; then
    echo "Too many specified output entries: ${#output[*]}"
    exit 1
fi

virtioargs=""
virtio_index="32"
virtio_hole="false"
while [ "${virtio_index}" -gt "0" ]
do
    virtio_index=$((virtio_index-1))
    case "${virtios[${virtio_index}]}" in
    "")
        virtio_hole="true"
        ;;
    net-user)
        if [ "${virtio_hole}" = "true" ]
        then
            printf "VIRTIO devices are not contiguous, aborting\n"
            exit 1
        fi
        virtioargs="${virtioargs} -netdev user,id=virtio${virtio_index}"
        virtioargs="${virtioargs} -device virtio-net-device,netdev=virtio${virtio_index}"
        ;;
    net-tap*-br*)
        if [ "${virtio_hole}" = "true" ]
        then
            printf "VIRTIO devices are not contiguous, aborting\n"
            exit 1
        fi
        tapbrdev="${virtios[${virtio_index}]#net-tap}"
        tapdev="${tapbrdev%%-br*}"
        brdev="${tapbrdev#*-br}"
        virtioargs="${virtioargs} -netdev tap,id=virtio${virtio_index},script=no,downscript=no,br=br${brdev},ifname=tap${tapdev}"
        virtioargs="${virtioargs} -device virtio-net-device,netdev=virtio${virtio_index}"
        ;;
    net-br*-tap*)
        if [ "${virtio_hole}" = "true" ]
        then
            printf "VIRTIO devices are not contiguous, aborting\n"
            exit 1
        fi
        brtapdev="${virtios[${virtio_index}]#net-br}"
        brdev="${brtapdev%%-tap*}"
        tapdev="${brtapdev#*-tap}"
        virtioargs="${virtioargs} -netdev tap,id=virtio${virtio_index},script=no,downscript=no,br=br${brdev},ifname=tap${tapdev}"
        virtioargs="${virtioargs} -device virtio-net-device,netdev=virtio${virtio_index}"
        ;;
    *)
        printf "Unsupported VIRTIO: \"%s\"\n" "${virtios[${virtio_index}]}"
        exit 1
        ;;
    esac
done

arch=$(get_arch "$variant")
qemu=$(find_qemu "$arch")
opts=""

if [ -z $qemu ]; then
    echo "qemu-system-$arch not found"
    echo "Please update your PATH or QEMUPATH variables"
    exit 1
fi

qemu_version=$("${qemu}" --version | head -n 1 | \
               cut -d' ' -f 4 | cut -d'(' -f 1)

# QEMU 2.11 in Ubuntu18.04 has bug https://patchwork.kernel.org/patch/10200681/
# We try a workaround for AArch32 and AArch64 platforms.
# To be removed once Ubuntu 20.04 comes out...
has_cpu_num_bug=0
if [[ "${qemu_version}" =~ ^2.11.*$ ]];
then
    has_cpu_num_bug=1
fi

if [ "$quiet" -ne "1" ]; then
    echo "arch: $arch"
    echo "qemu: '$qemu' version '$qemu_version'"
fi

# Source kernel limits.
# Defines the following constants:
#   KERNEL_LOAD_ADDR
#   KERNEL_START_ADDR
#   CODES_LOAD_ADDR
. "build/$variant/kernel_limits.mk"

monitor_bin=""
kernel_bin="build/$variant/kernel.bin"
codes_bin="build/$variant/codes.bin"

# Version check
kernel_version=$(get_kernel_version build/$variant/kernel.bin)
codes_version=$(get_codes_version ${codes_bin})

if [ "$kernel_version" != "$codes_version" ]; then
    echo "kernel/codes version mismatch"
    exit 1
fi

# Architecture specific options
case $arch in
    arm)
        start_addr="${KERNEL_START_ADDR}"
        opts="-cpu cortex-a15 -M vexpress-a15"
        memory=${memory:-1024}
        ;;

    aarch64)
        . "build/$variant/secmonv8_limits.mk"
        start_addr="${SECMONV8_START_ADDR}"
        monitor_load_addr="${SECMONV8_LOAD_ADDR}"
        monitor_bin="build/$variant/secmonv8.bin"
        opts="-machine virt,secure=on,gic-version=3,virtualization=true"
        opts+=" -cpu cortex-a57 -smp 1"
        opts+=" -global virtio-mmio.force-legacy=false"
        memory=${memory:-4096}
        ;;

    riscv64)
        . "build/$variant/mmode_limits.mk"
        start_addr="${MMODE_START_ADDR}"
        monitor_load_addr="${MMODE_LOAD_ADDR}"
        monitor_bin="build/$variant/mmode.bin"
        opts="-machine sifive_u -bios none"
        memory=${memory:-1024}
        ;;

    *)
        echo "Unsupported architecture $arch"
        exit 1
esac

opts="$opts -m $memory"

# Load monitor, kernel and applications
opts="$opts -device loader,addr=${start_addr}"
if [ "$has_cpu_num_bug" -eq "0" ]; then
   opts="$opts,cpu-num=0" # Workaround: do not add it when the bug is present
fi
opts="$opts -device loader,addr=${KERNEL_LOAD_ADDR},file=${kernel_bin}"
opts="$opts -device loader,addr=${CODES_LOAD_ADDR},file=${codes_bin}"
if [ ! -z ${monitor_bin} ]; then
    opts="$opts -device loader,addr=${monitor_load_addr},file=${monitor_bin}"
fi

opts="$opts ${virtioargs} -display none"

if [ $nographic -eq "1" ]; then
    opts="$opts -nographic"
    stdiopre="mon:"
else
    stdiopre=""
fi

if [ $semihosting -eq "1" ]; then
    opts="$opts -semihosting-config enable=on,target=native"
fi

if [ $debug -eq "1" ]; then
    opts="$opts -S -s"
fi

if [ ! -z $sdcard ]; then
    opts="$opts -drive format=raw,if=sd,file=$sdcard"
fi

if [ ! -z $tap ]; then
    opts="$opts -net nic -net tap,ifname=$tap,script=no,downscript=no"
fi

if [ "$deterministic" -eq "1" ]; then
    opts="$opts -icount shift=0,align=off,sleep=off"
fi

maxuart=$(($nruarts - 1))
nrstdio="0"
uartpids=""

for out in ${!output[*]}; do
    if [ "${output[$out]}" ==  "stdio" ]; then
        nrstdio=$(($nrstdio + 1))
    fi
done

if [ $nrstdio -gt 1 ]; then
    opts="$opts -chardev stdio,mux=on,id=multiplex"
    stdiodev="chardev:multiplex"
else
    stdiodev="stdio"
fi

for nr in $(seq 0 1 $maxuart); do
    if [ $nr -lt ${#output[@]} ]; then
        kind=${output[$nr]}
    else
        kind="term"
    fi
    case $kind in
        stdio)
            opts="$opts -serial ${stdiopre}${stdiodev}"
            if [[ "$quiet" == "0" ]]; then
                echo "uart${nr}: stdio"
            fi
            ;;

        file:*)
            name="${kind#file:}"
            if [ -z $name ]; then
                echo "Missing filename for uart file output"
                exit 1
            fi
            opts="$opts -serial file:$name"
            if [[ "$quiet" == "0" ]]; then
                echo "uart${nr}: file:$name"
            fi
            ;;

        tty:*)
            name="${kind#tty:}"
            if [ -z $name ]; then
                echo "Missing filename for uart tty output"
                exit 1
            fi
            # Clears the TTY (first command is for xterm to remove scrollback)
            printf "\x1b[3J" > $name
            printf "\x1b[2J" > $name
            printf "\x1b[H" > $name
            opts="$opts -chardev tty,id=serial$nr,path=$name \
                -serial chardev:serial$nr"

            if [[ "$quiet" == "0" ]]; then
                echo "uart${nr}: pts=$name"
            fi
            ;;

        pty)
            opts="$opts -chardev pty,id=serial$nr \
                -serial chardev:serial$nr"
            if [[ "$quiet" == "0" ]]; then
                echo "uart${nr}: pty"
            fi
            ;;

        term|*)
            uart=$(mktemp)
            export SHELL="/bin/bash"
            xterm -T "uart$nr" -e "tty > $uart && sleep 10d" &
            pid=$!
            pts=""
            while [ -z $pts ]; do
                pts=$(cat $uart)
            done

            opts="$opts -chardev tty,id=serial$nr,path=$pts \
                -serial chardev:serial$nr"
            uartpids="$uartpids $pid"

            if [[ "$quiet" == "0" ]]; then
                echo "uart${nr}: pts=$pts pid=$pid"
            fi
            ;;
    esac
done

if [[ "$quiet" != "1" ]]; then
    echo "$qemu $opts"
fi

if [[ "$dryrun" != "1" ]]; then
    $qemu $opts
fi

if [ -n "$uartpids" ]; then
    echo "qemu exited, kill remaining uarts"
fi
for pid in $uartpids; do
    kill -15 $pid
done

if [[ "$dryrun" == "1" ]] && [[ ! -z "$uartpids" ]]; then
    # Running the command is not recommended as is depends on
    #  script-defined uarts
    exit 1
fi
