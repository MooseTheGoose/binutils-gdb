
if test "${OUTPUT_FORMAT}" = "elf32-mos65xx"; then
  NO_REL_RELOCS=1
  NO_RELA_RELOCS=1
  NO_SMALL_DATA=1
  EMBEDDED=1
  ALIGNMENT=1
  . $srcdir/scripttempl/elf.sc
  return 0
fi


cat <<EOF
SECTIONS {
  .text : {
    *(.text)
    *(text)
  }
  .data : {
    *(.data)
    *(data)
  }
  .bss : {
    *(.bss)
    *(bss)
  }
}
EOF
