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
ADDR_MASKED=$((BASE_ADDR & 0x0fffffff))

# Pad file to 8-byte alignment
FILESIZE=$(stat -c %s "$BINFILE")
PAD=$(( (8 - (FILESIZE % 8)) % 8 ))
if (( PAD > 0 )); then
    dd if=/dev/zero bs=1 count=$PAD >> "$BINFILE" 2>/dev/null
fi

FILESIZE=$(( FILESIZE + PAD ))
OFFSET=0
CHUNK=$(( FILESIZE ))

true > "$OUTFILE"
while [[ $((FILESIZE)) -ge $((CHUNK)) ]] && [[ $((FILESIZE)) -gt 0 ]]; do
    printf "%08X %08X\n" "$(( 16#06000000 + ADDR_MASKED + OFFSET ))" "$((CHUNK))" >> "$OUTFILE"
    hexdump -v -e '4/1 "%02X" " " 4/1 "%02X" "\n"' -n $((CHUNK)) -s $OFFSET "$BINFILE" >> "$OUTFILE"
    ((FILESIZE-=CHUNK))
    ((OFFSET+=CHUNK))
done

if [[ $((FILESIZE)) -gt 0 ]]; then 
    printf "%08X %08X\n" "$(( 16#06000000 + ADDR_MASKED + OFFSET ))" "$((FILESIZE))" >> "$OUTFILE"
    hexdump -v -e '4/1 "%02X" " " 4/1 "%02X" "\n"' -s $OFFSET "$BINFILE" >> "$OUTFILE"
fi

#SRC_ADDR=800cf698
SRC_ADDR=80007588
SRC_HEX=$((16#$SRC_ADDR))
SRC_MASKED=$(( SRC_HEX & 0x0fffffff ))

offset=$(( BASE_ADDR - SRC_HEX ))
masked_offset=$(( offset & 0x3fffffc ))

BL=$(( 16#48000001 | masked_offset ))

printf "%08X %08X\n" "$((16#04000000 + SRC_MASKED))" "$BL" >> "$OUTFILE"

FLAG_ADDR=801cc16b
FLAG_HEX=$((16#$FLAG_ADDR))
# shellcheck disable=SC2034
FLAG_MASKED=$(( FLAG_HEX & 0x0fffffff ))

#printf "%08X %08X\n" "$FLAG_MASKED" "1" >> $OUTFILE