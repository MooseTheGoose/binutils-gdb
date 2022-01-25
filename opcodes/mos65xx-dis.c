
#include "sysdep.h"
#include "disassemble.h"
#include "mos65xx-dis.h"
#include <stdio.h>

/*
 * Opcodes must be sorted by base opcode.
 */
#if 0
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
#endif

static int
mos65xx_dis_read_val(bfd_vma vaddr, struct disassemble_info *info, int n, uint32_t *val)
{
  bfd_byte buf[4];
  uint32_t locval = 0;
  int status;
  int cntdwn = n;
  if(n > 4)
    return -1;
  status = info->read_memory_func(vaddr, buf, n, info);
  if(status)
  {
    info->memory_error_func(status, vaddr, info);
    return -1;
  }
  while(cntdwn--)
    locval = locval << 8 | buf[cntdwn];
  *val = locval;
  return n;
}

/*
static int
mos65xx_implied_lookup_compar(struct mos65xx_implied_lookup *lhs, struct mos65xx_implied_lookup *rhs)
{
  return (int)lhs->opcode - (int)rhs->opcode;
}
*/

static int
mos65xx_dis_read_arg(struct mos65xx_arg_str *arg, bfd_vma vaddr,
			struct disassemble_info *info)
{
  int width = arg->width;
  bool pcrel = arg->pcrel;
  if(width > 0)
  {
    char *printstr = arg->arg;
    int printsz = MOS65XX_ARG_SLEN;
    uint32_t val;
    if(mos65xx_dis_read_val(vaddr, info, width, &val) < 0)
      return -1;
    if(pcrel)
    {
      val = (int32_t)(val << (32 - 8 * width)) >> (32 - 8 * width);
      val += (vaddr + width);
    }
    int snlen = 0; 
    if(width == MOS65XX_SIZEOF_BYTE)
      snlen = snprintf(printstr, printsz, "$%02X", val);
    else if(width == MOS65XX_SIZEOF_WORD)
      snlen = snprintf(printstr, printsz, "$%04X", val);
    else
      snlen = snprintf(printstr, printsz, "$%06X", val);
    if(snlen >= 0 && snlen < printsz)
    {
      printstr += snlen;
      printsz -= snlen;
      if(val < (uint32_t)((width - 1) << 8))
      {
        char suffix = MOS65XX_WORD_SUFFIX;
        if(width == MOS65XX_SIZEOF_LONG)
          suffix = MOS65XX_LONG_SUFFIX;
        snlen = snprintf(printstr, printsz, ":%c", suffix);
      }
    }
  }
  return width;
}

static int 
print_insn(bfd_vma vaddr, struct disassemble_info *info, 
		const char *nmemonic, int addrmode, bool pcrel)
{
  int len = 1;

  struct mos65xx_arg_widths widths;
  struct mos65xx_arg_str arg1, arg2;

  mos65xx_addrmode_widths(addrmode, &widths);
  arg2.width = widths.width2;
  arg2.pcrel = false;
  arg1.width = widths.width1;
  arg1.pcrel = pcrel;

  if(arg1.width > 0)
  {
    int len1 = mos65xx_dis_read_arg(&arg1, vaddr, info);
    if(len1 < 0)
      return -1;
    len += len1; 
    vaddr += len1;
  }
  if(arg2.width > 0)
  {
    int len2 = mos65xx_dis_read_arg(&arg2, vaddr, info);
    if(len2 < 0)
      return -1;
    len += len2;
    vaddr += len2;
  }
  printf("%s", nmemonic);
  switch(addrmode)
  {
    case MOS65XX_ADDRMODE_IND_IDX:
    case MOS65XX_ADDRMODE_ABS_IND_IDX:
      printf(" (%s,X)", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_STK_REL:
      printf(" #%s,S", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_PGE:
    case MOS65XX_ADDRMODE_ABS:
    case MOS65XX_ADDRMODE_LNG:
      printf(" %s", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IND_LNG:
    case MOS65XX_ADDRMODE_ABS_IND_LNG:
      printf(" [%s]", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IND_LNG_IDY:
      printf(" [%s],Y", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IMM:
      printf(" #%s", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IND_IDY:
      printf(" (%s),Y", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IND:
    case MOS65XX_ADDRMODE_ABS_IND:
      printf(" (%s)", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_STK_IND_IDY:
      printf(" (#%s,S),Y", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IDY:
    case MOS65XX_ADDRMODE_ABS_IDY:
      printf(" %s,Y", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_IDX:
    case MOS65XX_ADDRMODE_ABS_IDX:
    case MOS65XX_ADDRMODE_ABS_LNG_IDX:
      printf(" %s,X", arg1.arg);
      break;
    case MOS65XX_ADDRMODE_ACC:
      printf(" A");
      break;
    case MOS65XX_ADDRMODE_BLK:
      printf(" #%s,#%s", arg1.arg, arg2.arg);
      break;
  }
  printf(" ");
  return len;
}

int print_insn_mos65xx(bfd_vma vaddr, struct disassemble_info *info) {
  uint32_t opcode;
  const char *nmemonic;
  int addrmode = MOS65XX_ADDRMODE_INVALID;
  bool pcrel = false;

  if(mos65xx_dis_read_val(vaddr, info, 1, &opcode) < 0)
    return -1;
  vaddr += 1;

  /* Disassemble arithmetic instructions */
  if((opcode & 0x10) == 0)
  {
    const char *const arith_nmemonics[8] = 
    { 
      "ORA", "AND", "EOR", "ADC", 
      "STA", "LDA", "CMP", "SBC"
    };
    nmemonic = arith_nmemonics[(opcode >> 5) & 7];
    switch(opcode & 0x1f)
    {
      case 0x1: addrmode = MOS65XX_ADDRMODE_IND_IDX; break;
      case 0x3: addrmode = MOS65XX_ADDRMODE_STK_REL; break;
      case 0x5: addrmode = MOS65XX_ADDRMODE_PGE; break;
      case 0x7: addrmode = MOS65XX_ADDRMODE_IND_LNG; break;
      case 0x9:
        if(opcode == 0x89)
          nmemonic = "BIT";
        addrmode = MOS65XX_ADDRMODE_IMM;
        break;
      case 0xd: addrmode = MOS65XX_ADDRMODE_ABS; break;
      case 0xf: addrmode = MOS65XX_ADDRMODE_LNG; break;
      case 0x11: addrmode = MOS65XX_ADDRMODE_IND_IDY; break;
      case 0x12: addrmode = MOS65XX_ADDRMODE_IND; break;
      case 0x13: addrmode = MOS65XX_ADDRMODE_STK_IND_IDY; break;
      case 0x15: addrmode = MOS65XX_ADDRMODE_PGE; break;
      case 0x17: addrmode = MOS65XX_ADDRMODE_IND_LNG_IDY; break;
      case 0x19: addrmode = MOS65XX_ADDRMODE_ABS_IDY; break;
      case 0x1d: addrmode = MOS65XX_ADDRMODE_ABS_IDX; break;
      case 0x1f: addrmode = MOS65XX_ADDRMODE_ABS_LNG_IDX; break;
    }
  }
  if(addrmode != MOS65XX_ADDRMODE_INVALID)
    return print_insn(vaddr, info, nmemonic, addrmode, pcrel);
  /*
  if((opcode & 0x10) == 0)
  {
    const char *const bitops_nmemonics[8] =
    {
      "ASL", "ROL", "LSR", "ROR",
      "BRL", "LDX", "DEC", "INC"
    };
    nmemonic = bitops_nmemonics[(opcode >> 5) & 7];
    switch(opcode & 0x1f)
    {
      case 0x2: break;
    } 
  }
  */
  printf("??? %lu ", vaddr);  
  return 1;
}
