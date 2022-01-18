
#include "sysdep.h"
#include "disassemble.h"

int print_insn_mos65xx(bfd_vma vaddr, struct disassemble_info *info) {
  return (uintptr_t)(vaddr + info) & 0xffff;
}
