
#include "sysdep.h"
#include "disassemble.h"
#include <stdio.h>

int print_insn_mos65xx(bfd_vma vaddr ATTRIBUTE_UNUSED, struct disassemble_info *info ATTRIBUTE_UNUSED) {
  printf("Badger");
  return 1;
}
