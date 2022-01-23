
#include "sysdep.h"
#include "disassemble.h"
#include "mos65xx-dis.h"
#include <stdio.h>

/*
 * Opcodes must be sorted by base opcode.
 */
static const struct mos65xx_implied_lookup
mos65xx_implied_table[] =
{
  MOS65XX_IMPCREATE(BRK),
  MOS65XX_IMPCREATE(PHP),
  MOS65XX_IMPCREATE(PHD),
  MOS65XX_IMPCREATE(CLC),
  MOS65XX_IMPCREATE(TCS),
  MOS65XX_IMPCREATE(PLP),
  MOS65XX_IMPCREATE(PLD),
  MOS65XX_IMPCREATE(SEC),
  MOS65XX_IMPCREATE(TSC),
  MOS65XX_IMPCREATE(RTI),
  MOS65XX_IMPCREATE(WDM),
  MOS65XX_IMPCREATE(PHA),
  MOS65XX_IMPCREATE(PHK),
  MOS65XX_IMPCREATE(CLI),
  MOS65XX_IMPCREATE(PHY),
  MOS65XX_IMPCREATE(TCD),
  MOS65XX_IMPCREATE(RTS),
  MOS65XX_IMPCREATE(PLA),
  MOS65XX_IMPCREATE(RTL),
  MOS65XX_IMPCREATE(SEI),
  MOS65XX_IMPCREATE(PLY),
  MOS65XX_IMPCREATE(TDC),
  MOS65XX_IMPCREATE(DEY),
  MOS65XX_IMPCREATE(TXA),
  MOS65XX_IMPCREATE(PHB),
  MOS65XX_IMPCREATE(TYA),
  MOS65XX_IMPCREATE(TXS),
  MOS65XX_IMPCREATE(TXY),
  MOS65XX_IMPCREATE(TAY),
  MOS65XX_IMPCREATE(TAX),
  MOS65XX_IMPCREATE(PLB),
  MOS65XX_IMPCREATE(CLV),
  MOS65XX_IMPCREATE(TSX),
  MOS65XX_IMPCREATE(TYX),
  MOS65XX_IMPCREATE(INY),
  MOS65XX_IMPCREATE(DEX),
  MOS65XX_IMPCREATE(WAI),
  MOS65XX_IMPCREATE(CLD),
  MOS65XX_IMPCREATE(PHX),
  MOS65XX_IMPCREATE(STP),
  MOS65XX_IMPCREATE(SEP),
  MOS65XX_IMPCREATE(INX),
  MOS65XX_IMPCREATE(NOP),
  MOS65XX_IMPCREATE(XBA),
  MOS65XX_IMPCREATE(SED),
  MOS65XX_IMPCREATE(PLX),
  MOS65XX_IMPCREATE(XCE)
};

static int
mos65xx_implied_lookup_compar(struct mos65xx_implied_lookup *lhs, struct mos65xx_implied_lookup *rhs)
{
  return (int)lhs->opcode - (int)rhs->opcode;
}

int print_insn_mos65xx(bfd_vma vaddr, struct disassemble_info *info) {
  bfd_byte opcode;
  int len = 1;
  struct mos65xx_implied_lookup key, *lookup;

  int status = info->read_memory_func(vaddr, &opcode, 1, info);
  if(status)
  {
    info->memory_error_func(status, vaddr, info);
    return -1;
  }
  vaddr += 1;

  key.opcode = opcode;
  lookup = bsearch(
    &key,
    mos65xx_implied_table,
    sizeof(mos65xx_implied_table) / sizeof(mos65xx_implied_table[0]),
    sizeof(mos65xx_implied_table[0]),
    (int (*)(const void *, const void *))mos65xx_implied_lookup_compar
  );
  if(lookup != NULL) {
    printf("%s ", lookup->name); 
  } else {
    printf("Badger ");
  }
  return len;
}
