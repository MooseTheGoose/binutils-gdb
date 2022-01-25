
#include <opcode/mos65xx.h>
#include <mos65xx-opc.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/* Opcodes must be in alphabetical order for bsearch */
static const struct mos65xx_oplookup
mos65xx_opcodes[] = 
{
  MOS65XX_OPCREATE(ADC) ,
  MOS65XX_OPCREATE(AND) ,
  MOS65XX_OPCREATE(ASL) ,
  MOS65XX_OPCREATE(BCC) ,
  MOS65XX_OPCREATE(BCS) ,
  MOS65XX_OPCREATE(BEQ) ,
  MOS65XX_OPCREATE(BIT) ,
  MOS65XX_OPCREATE(BMI) ,
  MOS65XX_OPCREATE(BNE) ,
  MOS65XX_OPCREATE(BPL) ,
  MOS65XX_OPCREATE(BRA) ,
  MOS65XX_OPCREATE(BRK) ,
  MOS65XX_OPCREATE(BRL) ,
  MOS65XX_OPCREATE(BVC) ,
  MOS65XX_OPCREATE(BVS) ,
  MOS65XX_OPCREATE(CLC) ,
  MOS65XX_OPCREATE(CLD) ,
  MOS65XX_OPCREATE(CLI) ,
  MOS65XX_OPCREATE(CLV) ,
  MOS65XX_OPCREATE(CMP) ,
  MOS65XX_OPCREATE(COP) ,
  MOS65XX_OPCREATE(CPX) ,
  MOS65XX_OPCREATE(CPY) ,
  MOS65XX_OPCREATE(DEC) ,
  MOS65XX_OPCREATE(DEX) ,
  MOS65XX_OPCREATE(DEY) ,
  MOS65XX_OPCREATE(EOR) ,
  MOS65XX_OPCREATE(INC) ,
  MOS65XX_OPCREATE(INX) ,
  MOS65XX_OPCREATE(INY) ,
  MOS65XX_OPCREATE(JMP) ,
  MOS65XX_OPCREATE(JSR) ,
  MOS65XX_OPCREATE(LDA) ,
  MOS65XX_OPCREATE(LDX) ,
  MOS65XX_OPCREATE(LDY) ,
  MOS65XX_OPCREATE(LSR) ,
  MOS65XX_OPCREATE(MVN) ,
  MOS65XX_OPCREATE(MVP) ,
  MOS65XX_OPCREATE(NOP) ,
  MOS65XX_OPCREATE(ORA) ,
  MOS65XX_OPCREATE(PEA) ,
  MOS65XX_OPCREATE(PEI) ,
  MOS65XX_OPCREATE(PER) ,
  MOS65XX_OPCREATE(PHA) ,
  MOS65XX_OPCREATE(PHB) ,
  MOS65XX_OPCREATE(PHD) ,
  MOS65XX_OPCREATE(PHK) ,
  MOS65XX_OPCREATE(PHP) ,
  MOS65XX_OPCREATE(PHX) ,
  MOS65XX_OPCREATE(PHY) ,
  MOS65XX_OPCREATE(PLA) ,
  MOS65XX_OPCREATE(PLB) ,
  MOS65XX_OPCREATE(PLD) ,
  MOS65XX_OPCREATE(PLP) ,
  MOS65XX_OPCREATE(PLX) ,
  MOS65XX_OPCREATE(PLY) ,
  MOS65XX_OPCREATE(REP) ,
  MOS65XX_OPCREATE(ROL) ,
  MOS65XX_OPCREATE(ROR) ,
  MOS65XX_OPCREATE(RTI) ,
  MOS65XX_OPCREATE(RTL) ,
  MOS65XX_OPCREATE(RTS) ,
  MOS65XX_OPCREATE(SBC) ,
  MOS65XX_OPCREATE(SEC) ,
  MOS65XX_OPCREATE(SED) ,
  MOS65XX_OPCREATE(SEI) ,
  MOS65XX_OPCREATE(SEP) ,
  MOS65XX_OPCREATE(STA) ,
  MOS65XX_OPCREATE(STP) ,
  MOS65XX_OPCREATE(STX) ,
  MOS65XX_OPCREATE(STY) ,
  MOS65XX_OPCREATE(STZ) ,
  MOS65XX_OPCREATE(TAX) ,
  MOS65XX_OPCREATE(TAY) ,
  MOS65XX_OPCREATE(TCD) ,
  MOS65XX_OPCREATE(TCS) ,
  MOS65XX_OPCREATE(TDC) ,
  MOS65XX_OPCREATE(TRB) ,
  MOS65XX_OPCREATE(TSB) ,
  MOS65XX_OPCREATE(TSC) ,
  MOS65XX_OPCREATE(TSX) ,
  MOS65XX_OPCREATE(TXA) ,
  MOS65XX_OPCREATE(TSX) ,
  MOS65XX_OPCREATE(TXY) ,
  MOS65XX_OPCREATE(TYA) ,
  MOS65XX_OPCREATE(TYX) ,
  MOS65XX_OPCREATE(WAI) ,
  MOS65XX_OPCREATE(WDM),
  MOS65XX_OPCREATE(XBA),
  MOS65XX_OPCREATE(XCE) 
};

static uint32_t mos65xx_opclass_modeflags[MOS65XX_OPCLASS_MAX] =
{
  MOS65XX_ALUMEM_MODEFLAGS,
  MOS65XX_CPXY_MODEFLAGS,
  MOS65XX_BITOPS_MODEFLAGS,
  MOS65XX_BRANCH_MODEFLAGS,
  MOS65XX_IDX_MODEFLAGS,
  MOS65XX_IDY_MODEFLAGS,
  MOS65XX_IMPLIED_MODEFLAGS,
  MOS65XX_JUMP_MODEFLAGS,
  MOS65XX_JSR_MODEFLAGS,
  MOS65XX_BLK_MODEFLAGS,
  MOS65XX_PEA_MODEFLAGS,
  MOS65XX_PEI_MODEFLAGS,
  MOS65XX_PER_MODEFLAGS,
  MOS65XX_IMM_MODEFLAGS,
  MOS65XX_STA_MODEFLAGS,
  MOS65XX_STX_MODEFLAGS,
  MOS65XX_STY_MODEFLAGS,
  MOS65XX_STZ_MODEFLAGS,
  MOS65XX_TB_MODEFLAGS,
  MOS65XX_BRANCHLONG_MODEFLAGS 
};

/*
 *  The assembler can define these in any
 *  way most convenient, but these do have
 *  to exist for every address mode.
 *
 *  Do not take changing this lightly.
 *  These are an integral part into encoding
 *  the right opcode given an addressing mode.
 *  Change at your own risk.
 */
const uint8_t
mos65xx_addrmode_addends[MOS65XX_ADDRMODE_MAX] =
{
  /* IND_IDX, STK_REL, PGE, IND_LNG, IMM, ABS, LNG */
  0x0, 0x2, 0x4, 0x6, 0x0, 0xc, 0xe, 0x10,
  /* IND_IDY, IND, STK_IND_IDY, IDX, IND_LNG_IDY, ABS_IDY, ABS_IDX, ABS_LNG_IDX */
  0x11, 0x12, 0x14, 0x16, 0x18, 0x1c, 0x1e,
  /* IDY, ACC, BLK, IMPLIED */
  0x14, 0x8, 0x0, 0x0, 
  /* ABS_IND, ABS_IND_IDX, ABS_IND_LNG */
  0x10, 0x12, 0x1e,
  /* PCREL, PCREL16 */
  0x0, 0x0
}; 

static int
mos65xx_oplookup_compar(
  struct mos65xx_oplookup *lhs,
  struct mos65xx_oplookup *rhs)
{
  return strcasecmp(lhs->name, rhs->name);
}

struct mos65xx_op
*mos65xx_opcode_lookup(const char *nmemonic, struct mos65xx_op *out_opcode)
{
  struct mos65xx_oplookup dummy = {nmemonic, 0, 0};
  struct mos65xx_oplookup *id = bsearch(&dummy, mos65xx_opcodes, 
    sizeof(mos65xx_opcodes) / sizeof(mos65xx_opcodes[0]),
    sizeof(mos65xx_opcodes[0]),
    (int(*)(const void *, const void *))mos65xx_oplookup_compar);
  struct mos65xx_op *fill_in = (id == NULL) ? NULL : out_opcode;
  if(fill_in != NULL)
  {
    fill_in->name = id->name;
    fill_in->modeflags = mos65xx_opclass_modeflags[id->opclass];
    fill_in->opclass = id->opclass;
    fill_in->base = id->base;
  }
  return fill_in;
}

void
mos65xx_addrmode_widths(int addrmode, struct mos65xx_arg_widths *widths)
{
  int width1 = 0;
  int width2 = 0;

  switch(addrmode)
  {
    case MOS65XX_ADDRMODE_BLK:
      width1 = MOS65XX_SIZEOF_BYTE;
      width2 = MOS65XX_SIZEOF_BYTE;
      break;
    case MOS65XX_ADDRMODE_PGE:
    case MOS65XX_ADDRMODE_IND_LNG:
    case MOS65XX_ADDRMODE_IMM:
    case MOS65XX_ADDRMODE_IND_IDY:
    case MOS65XX_ADDRMODE_IND_IDX:
    case MOS65XX_ADDRMODE_IND:
    case MOS65XX_ADDRMODE_STK_IND_IDY:
    case MOS65XX_ADDRMODE_IDX:
    case MOS65XX_ADDRMODE_IND_LNG_IDY:
    case MOS65XX_ADDRMODE_IDY:
      width1 = MOS65XX_SIZEOF_BYTE;
      break;
    case MOS65XX_ADDRMODE_ABS:
    case MOS65XX_ADDRMODE_ABS_IDY:
    case MOS65XX_ADDRMODE_ABS_IDX:
    case MOS65XX_ADDRMODE_ABS_IND:
    case MOS65XX_ADDRMODE_ABS_IND_IDX:
    case MOS65XX_ADDRMODE_ABS_IND_LNG:
      width1 = MOS65XX_SIZEOF_WORD;
      break;
    case MOS65XX_ADDRMODE_LNG:
    case MOS65XX_ADDRMODE_ABS_LNG_IDX:
      width1 = MOS65XX_SIZEOF_LONG;
      break;
  }
  widths->width1 = width1;
  widths->width2 = width2;
}
