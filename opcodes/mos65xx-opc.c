
#include <opcode/mos65xx.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

static const struct mos65xx_op mos65xx_opcodes[] = 
{
  { "adc", MOS65XX_OPCODE_ALUMEM | 0x61 },
  { "and", MOS65XX_OPCODE_ALUMEM | 0x21 },
  { "asl", MOS65XX_OPCODE_BITOPS | 0x6 },
  { "bcc", MOS65XX_OPCODE_BRANCH | 0x90 },
  { "bcs", MOS65XX_OPCODE_BRANCH | 0xb0 },
  { "beq", MOS65XX_OPCODE_BRANCH | 0xf0 },
  { "bit", MOS65XX_OPCODE_LDY | 0x24 },
  { "bmi", MOS65XX_OPCODE_BRANCH | 0x30 },
  { "bne", MOS65XX_OPCODE_BRANCH | 0xd0 },
  { "bpl", MOS65XX_OPCODE_BRANCH | 0x10 },
  { "bra", MOS65XX_OPCODE_BRANCH | 0x80 },
  { "brk", MOS65XX_OPCODE_NOARGS | 0x0 },
  { "brl", MOS65XX_OPCODE_BRANCH | 0x82 },
  { "bvc", MOS65XX_OPCODE_BRANCH | 0x50 },
  { "bvs", MOS65XX_OPCODE_BRANCH | 0x70 },
  { "clc", MOS65XX_OPCODE_NOARGS | 0x18 },
  { "cld", MOS65XX_OPCODE_NOARGS | 0xd8 },
  { "cli", MOS65XX_OPCODE_NOARGS | 0x58 },
  { "clv", MOS65XX_OPCODE_NOARGS | 0xb8 },
  { "cmp", MOS65XX_OPCODE_ALUMEM | 0xc1 },
  { "cop", MOS65XX_OPCODE_SPECIAL_COP | 0x2 },
  { "cpx", MOS65XX_OPCODE_LDX | 0xe0 },
  { "cpy", MOS65XX_OPCODE_LDY | 0xc0 },
  { "dec", MOS65XX_OPCODE_BITOPS | 0xc6 },
  { "dex", MOS65XX_OPCODE_NOARGS | 0xca },
  { "dey", MOS65XX_OPCODE_NOARGS | 0x88 },
  { "eor", MOS65XX_OPCODE_ALUMEM | 0x41 },
  { "inc", MOS65XX_OPCODE_BITOPS | 0xe6 },
  { "inx", MOS65XX_OPCODE_NOARGS | 0xe8 },
  { "iny", MOS65XX_OPCODE_NOARGS | 0xc8 },
  { "jmp", MOS65XX_OPCODE_JUMP | 0x4c },
  { "jsr", MOS65XX_OPCODE_JUMP | 0x20 },
  { "lda", MOS65XX_OPCODE_ALUMEM | 0xa1 },
  { "ldx", MOS65XX_OPCODE_LDX | 0xa2 },
  { "ldy", MOS65XX_OPCODE_LDY | 0xa0 },
  { "lsr", MOS65XX_OPCODE_BITOPS | 0x46 },
  { "mvn", MOS65XX_OPCODE_SPECIAL_MVNP | 0x54 },
  { "mvp", MOS65XX_OPCODE_SPECIAL_MVNP | 0x44 },
  { "nop", MOS65XX_OPCODE_NOARGS | 0xea },
  { "ora", MOS65XX_OPCODE_ALUMEM | 0x01 },
  { "pea", MOS65XX_OPCODE_SPECIAL_PEA | 0xf4 },
  { "pei", MOS65XX_OPCODE_SPECIAL_PEI | 0xd4 },
  { "per", MOS65XX_OPCODE_SPECIAL_PER | 0x62 },
  { "pha", MOS65XX_OPCODE_NOARGS | 0x48 },
  { "phb", MOS65XX_OPCODE_NOARGS | 0x8b },
  { "phd", MOS65XX_OPCODE_NOARGS | 0x0b },
  { "phk", MOS65XX_OPCODE_NOARGS | 0x4b },
  { "php", MOS65XX_OPCODE_NOARGS | 0x8 },
  { "phx", MOS65XX_OPCODE_NOARGS | 0xda },
  { "phy", MOS65XX_OPCODE_NOARGS | 0x5a },
  { "pla", MOS65XX_OPCODE_NOARGS | 0x68 },
  { "plb", MOS65XX_OPCODE_NOARGS | 0xab },
  { "pld", MOS65XX_OPCODE_NOARGS | 0x2b },
  { "plp", MOS65XX_OPCODE_NOARGS | 0x28 },
  { "plx", MOS65XX_OPCODE_NOARGS | 0xfa },
  { "ply", MOS65XX_OPCODE_NOARGS | 0x7a },
  { "rep", MOS65XX_OPCODE_SPECIAL_REP | 0xc2 },
  { "rol", MOS65XX_OPCODE_BITOPS | 0x26 },
  { "ror", MOS65XX_OPCODE_BITOPS | 0x66 },
  { "rti", MOS65XX_OPCODE_NOARGS | 0x40 },
  { "rtl", MOS65XX_OPCODE_NOARGS | 0x6b },
  { "rts", MOS65XX_OPCODE_NOARGS | 0x60 },
  { "sbc", MOS65XX_OPCODE_ALUMEM | 0xe1 },
  { "sec", MOS65XX_OPCODE_NOARGS | 0x38 },
  { "sed", MOS65XX_OPCODE_NOARGS | 0xf8 },
  { "sei", MOS65XX_OPCODE_NOARGS | 0x78 },
  { "sep", MOS65XX_OPCODE_NOARGS | 0xe2 },
  { "sta", MOS65XX_OPCODE_ALUMEM | 0x81 },
  { "stp", MOS65XX_OPCODE_NOARGS | 0xdb },
  { "stx", MOS65XX_OPCODE_LDX | 0x82 },
  { "sty", MOS65XX_OPCODE_LDY | 0x80 },
  { "stz", MOS65XX_OPCODE_LDY | 0x60 },
  { "tax", MOS65XX_OPCODE_NOARGS | 0xaa },
  { "tay", MOS65XX_OPCODE_NOARGS | 0xa8 },
  { "tcd", MOS65XX_OPCODE_NOARGS | 0x5b },
  { "tcs", MOS65XX_OPCODE_NOARGS | 0x1b },
  { "tdc", MOS65XX_OPCODE_NOARGS | 0x7b },
  { "trb", MOS65XX_OPCODE_SPECIAL_TRSB | 0x14 },
  { "tsb", MOS65XX_OPCODE_SPECIAL_TRSB | 0x04 },
  { "tsc", MOS65XX_OPCODE_NOARGS | 0x3b },
  { "tsx", MOS65XX_OPCODE_NOARGS | 0xba },
  { "txa", MOS65XX_OPCODE_NOARGS | 0x8a },
  { "txs", MOS65XX_OPCODE_NOARGS | 0x9a },
  { "txy", MOS65XX_OPCODE_NOARGS | 0x9b },
  { "tya", MOS65XX_OPCODE_NOARGS | 0x98 },
  { "tyx", MOS65XX_OPCODE_NOARGS | 0xbb },
  { "wai", MOS65XX_OPCODE_NOARGS | 0xcb },
  { "wdm", MOS65XX_OPCODE_NOARGS | 0x42 },
  { "xba", MOS65XX_OPCODE_NOARGS | 0xeb },
  { "xce", MOS65XX_OPCODE_NOARGS | 0xfb } 
};
#define MOS65XX_OPCODES_SIZE (sizeof(mos65xx_opcodes) / sizeof(mos65xx_opcodes[0]))

static int mos65xx_op_compare(const struct mos65xx_op *left, const struct mos65xx_op *right)
{
  return strcasecmp(left->name, right->name); 
}

struct mos65xx_opcode *mos65xx_opcode_lookup(const char *nmemonic)
{
  struct mos65xx_op dummy = {nmemonic, 0};
  return bsearch(&dummy, mos65xx_opcodes, 
    sizeof(mos65xx_opcodes) / sizeof(mos65xx_opcodes[0]),
    sizeof(mos65xx_opcodes[0]),
    (int(*)(const void *, const void *))mos65xx_op_compare);  
}

