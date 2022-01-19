

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
