#!/usr/bin/env bash

# Usage: ./bin_to_gecko06.sh input.bin address output.txt
# Example: ./bin_to_gecko06.sh InjectedFunc.bin 81700000 InjectedFunc.gecko.txt

BINFILE=$1
ADDR_HEX=$2
OUTFILE=$3

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <input.bin> <hex address> <output.txt>"
    exit 1
fi

# Ensure file exists
if [[ ! -f "$BINFILE" ]]; then
    echo "Error: file '$BINFILE' not found"
    exit 1
fi

# Strip 0x if present
ADDR_HEX=${ADDR_HEX#0x}
BASE_ADDR=$((16#$ADDR_HEX))
ADDR_MASKED=$(($BASE_ADDR & 0x0fffffff))

# Pad file to 4-byte alignment
FILESIZE=$(stat -c %s "$BINFILE")

printf "%08X %08X\n" "$((16#06000000 + $ADDR_MASKED))" "$(($FILESIZE + 4))"> "$OUTFILE"

hexdump -v -e '4/1 "%02X" " " 4/1 "%02X" "\n"' "$BINFILE" >> "$OUTFILE"

SRC_ADDR=800cf698
SRC_HEX=$((16#$SRC_ADDR))
SRC_MASKED=$(( SRC_HEX & 0x0fffffff ))

offset=$(( BASE_ADDR - SRC_HEX ))
masked_offset=$(( offset & 0x3fffffc ))

BL=$(( 16#48000001 | masked_offset ))

printf "%08X %08X\n" "$((16#04000000 + $SRC_MASKED))" "$BL" >> "$OUTFILE"

FLAG_ADDR=801cc16b
FLAG_HEX=$((16#$FLAG_ADDR))
FLAG_MASKED=$(( FLAG_HEX & 0x0fffffff ))

printf "%08X %08X\n" "$FLAG_MASKED" "1" >> $OUTFILE