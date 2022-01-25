
#include "sysdep.h"
#include "disassemble.h"
#include "mos65xx-dis.h"
#include <stdio.h>

#if 0
static struct mos65816_disas
default_disas[256] =
{
  /* 0x00 */
  MOS65816_DIS("BRK", IMM),
  MOS65816_DIS("ORA", IND_IDX),
  MOS65816_DIS("COP", IMM),
  MOS65816_DIS("ORA", STK_REL),
  MOS65816_DIS("TSB", PGE),
  MOS65816_DIS("ORA", PGE),
  MOS65816_DIS("ASL", PGE),
  MOS65816_DIS("ORA", IND_LNG),
  MOS65816_DIS("PHP", IMPLIED),
  MOS65816_DIS("ORA", IMM),
  MOS65816_DIS("ASL", ACC),
  MOS65816_DIS("PHD", IMM),
  MOS65816_DIS("TSB", ABS),
  MOS65816_DIS("ORA", ABS),
  MOS65816_DIS("ASL", ABS),
  MOS65816_DIS("ORA", LNG),
  MOS65816_DIS_PCREL("BPL", ABS),
  MOS65816_DIS("ORA", IND_IDY),
  MOS65816_DIS("ORA", IND),
  MOS65816_DIS("ORA", STK_IND_IDY),
  MOS65816_DIS("TRB", PGE),
  MOS65816_DIS("ORA", IDX),
  MOS65816_DIS("ASL", IDX),
  MOS65816_DIS("ORA", IND_LNG_IDY),
  MOS65816_DIS("CLC", IMPLIED),
  MOS65816_DIS("ORA", ABS_IDY),
  MOS65816_DIS("INC", ACC),
  MOS65816_DIS("TCS", IMPLIED),
  MOS65816_DIS("TRB", ABS),
  MOS65816_DIS("ORA", ABS_IDX),
  MOS65816_DIS("ASL", ABS_IDX),
  MOS65816_DIS("ORA", ABS_LNG_IDX),
  /* 0x20 */
  MOS65816_DIS("JSR", ABS),
  MOS65816_DIS("AND", IND_IDX),
  MOS65816_DIS("JSR", LNG),
  MOS65816_DIS("AND", STK_REL),
  MOS65816_DIS("BIT", PGE),
  MOS65816_DIS("AND", PGE),
  MOS65816_DIS("ROL", PGE),
  MOS65816_DIS("AND", IND_LNG),
  MOS65816_DIS("PLP", IMPLIED),
  MOS65816_DIS("AND", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PLD", IMPLIED),
  MOS65816_DIS("BIT", ABS),
  MOS65816_DIS("AND", ABS),
  MOS65816_DIS("ROL", ABS),
  MOS65816_DIS("AND", LNG),
  MOS65816_DIS_PCREL("BMI", ABS),
  MOS65816_DIS("AND", IND_IDY),
  MOS65816_DIS("AND", IND),
  MOS65816_DIS("AND", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("AND", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("AND", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("AND", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("AND", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("AND", ABS_LNG_IDX),
  /* 0x40 */
  MOS65816_DIS("RTI", IMPLIED),
  MOS65816_DIS("EOR", IND_IDX),
  MOS65816_DIS("WDM", IMM),
  MOS65816_DIS("EOR", STK_REL),
  MOS65816_DIS("MVP", BLK),
  MOS65816_DIS("EOR", PGE),
  MOS65816_DIS("LSR", PGE),
  MOS65816_DIS("EOR", IND_LNG),
  MOS65816_DIS("PHA", IMPLIED),
  MOS65816_DIS("EOR", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PHK", IMPLIED),
  MOS65816_DIS("JMP", ABS),
  MOS65816_DIS("EOR", ABS),
  MOS65816_DIS("LSR", ABS),
  MOS65816_DIS("EOR", LNG),
  MOS65816_DIS_PCREL("BVC", ABS),
  MOS65816_DIS("EOR", IND_IDY),
  MOS65816_DIS("EOR", IND),
  MOS65816_DIS("EOR", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("EOR", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("EOR", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("EOR", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("EOR", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("EOR", ABS_LNG_IDX),
  /* 0x60 */
  MOS65816_DIS("JSR", ABS),
  MOS65816_DIS("ADC", IND_IDX),
  MOS65816_DIS("JSR", LNG),
  MOS65816_DIS("ADC", STK_REL),
  MOS65816_DIS("BIT", PGE),
  MOS65816_DIS("ADC", PGE),
  MOS65816_DIS("ROL", PGE),
  MOS65816_DIS("ADC", IND_LNG),
  MOS65816_DIS("PLP", IMPLIED),
  MOS65816_DIS("ADC", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PLD", IMM),
  MOS65816_DIS("BIT", ABS),
  MOS65816_DIS("ADC", ABS),
  MOS65816_DIS("ROL", ABS),
  MOS65816_DIS("ADC", LNG),
  MOS65816_DIS_PCREL("BMI", ABS),
  MOS65816_DIS("ADC", IND_IDY),
  MOS65816_DIS("ADC", IND),
  MOS65816_DIS("ADC", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("ADC", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("ADC", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("ADC", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("ADC", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("ADC", ABS_LNG_IDX),
  /* 0x80 */
  MOS65816_DIS("JSR", ABS),
  MOS65816_DIS("STA", IND_IDX),
  MOS65816_DIS("JSR", LNG),
  MOS65816_DIS("STA", STK_REL),
  MOS65816_DIS("BIT", PGE),
  MOS65816_DIS("STA", PGE),
  MOS65816_DIS("ROL", PGE),
  MOS65816_DIS("STA", IND_LNG),
  MOS65816_DIS("PLP", IMPLIED),
  MOS65816_DIS("BIT", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PLD", IMM),
  MOS65816_DIS("BIT", ABS),
  MOS65816_DIS("STA", ABS),
  MOS65816_DIS("ROL", ABS),
  MOS65816_DIS("STA", LNG),
  MOS65816_DIS_PCREL("BMI", ABS),
  MOS65816_DIS("STA", IND_IDY),
  MOS65816_DIS("STA", IND),
  MOS65816_DIS("STA", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("STA", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("STA", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("STA", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("STA", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("STA", ABS_LNG_IDX),
  /* 0xa0 */
  MOS65816_DIS("JSR", ABS),
  MOS65816_DIS("LDA", IND_IDX),
  MOS65816_DIS("JSR", LNG),
  MOS65816_DIS("LDA", STK_REL),
  MOS65816_DIS("BIT", PGE),
  MOS65816_DIS("LDA", PGE),
  MOS65816_DIS("ROL", PGE),
  MOS65816_DIS("LDA", IND_LNG),
  MOS65816_DIS("PLP", IMPLIED),
  MOS65816_DIS("LDA", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PLD", IMM),
  MOS65816_DIS("BIT", ABS),
  MOS65816_DIS("LDA", ABS),
  MOS65816_DIS("ROL", ABS),
  MOS65816_DIS("LDA", LNG),
  MOS65816_DIS_PCREL("BMI", ABS),
  MOS65816_DIS("LDA", IND_IDY),
  MOS65816_DIS("LDA", IND),
  MOS65816_DIS("LDA", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("LDA", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("LDA", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("LDA", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("LDA", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("LDA", ABS_LNG_IDX),
  /* 0xc0 */
  MOS65816_DIS("JSR", ABS),
  MOS65816_DIS("CMP", IND_IDX),
  MOS65816_DIS("JSR", LNG),
  MOS65816_DIS("CMP", STK_REL),
  MOS65816_DIS("BIT", PGE),
  MOS65816_DIS("CMP", PGE),
  MOS65816_DIS("ROL", PGE),
  MOS65816_DIS("CMP", IND_LNG),
  MOS65816_DIS("PLP", IMPLIED),
  MOS65816_DIS("CMP", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PLD", IMM),
  MOS65816_DIS("BIT", ABS),
  MOS65816_DIS("CMP", ABS),
  MOS65816_DIS("ROL", ABS),
  MOS65816_DIS("CMP", LNG),
  MOS65816_DIS_PCREL("BMI", ABS),
  MOS65816_DIS("CMP", IND_IDY),
  MOS65816_DIS("CMP", IND),
  MOS65816_DIS("CMP", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("CMP", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("CMP", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("CMP", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("CMP", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("CMP", ABS_LNG_IDX),
  /* 0xe0 */
  MOS65816_DIS("JSR", ABS),
  MOS65816_DIS("SBC", IND_IDX),
  MOS65816_DIS("JSR", LNG),
  MOS65816_DIS("SBC", STK_REL),
  MOS65816_DIS("BIT", PGE),
  MOS65816_DIS("SBC", PGE),
  MOS65816_DIS("ROL", PGE),
  MOS65816_DIS("SBC", IND_LNG),
  MOS65816_DIS("PLP", IMPLIED),
  MOS65816_DIS("SBC", IMM),
  MOS65816_DIS("ROL", ACC),
  MOS65816_DIS("PLD", IMM),
  MOS65816_DIS("BIT", ABS),
  MOS65816_DIS("SBC", ABS),
  MOS65816_DIS("ROL", ABS),
  MOS65816_DIS("SBC", LNG),
  MOS65816_DIS_PCREL("BMI", ABS),
  MOS65816_DIS("SBC", IND_IDY),
  MOS65816_DIS("SBC", IND),
  MOS65816_DIS("SBC", STK_IND_IDY),
  MOS65816_DIS("BIT", IDX),
  MOS65816_DIS("SBC", IDX),
  MOS65816_DIS("ROL", IDX),
  MOS65816_DIS("SBC", IND_LNG_IDY),
  MOS65816_DIS("SEC", IMPLIED),
  MOS65816_DIS("SBC", ABS_IDY),
  MOS65816_DIS("DEC", ACC),
  MOS65816_DIS("TSC", IMPLIED),
  MOS65816_DIS("BIT", ABS_IDX),
  MOS65816_DIS("SBC", ABS_IDX),
  MOS65816_DIS("ROL", ABS_IDX),
  MOS65816_DIS("SBC", ABS_LNG_IDX)
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
  int pcrel_width = arg->pcrel_width;
  int read_width = (pcrel_width > 0) ? pcrel_width : width;
  if(read_width > 0)
  {
    char *printstr = arg->arg;
    int printsz = MOS65XX_ARG_SLEN;
    uint32_t val;
    if(mos65xx_dis_read_val(vaddr, info, read_width, &val) < 0)
      return -1;
    if(pcrel_width > 0)
    {
      val = (int32_t)(val << (32 - 8 * pcrel_width)) >> (32 - 8 * pcrel_width);
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
		const char *nmemonic, int addrmode, int pcrel_width)
{
  int len = 1;

  struct mos65xx_arg_widths widths;
  struct mos65xx_arg_str arg1, arg2;

  mos65xx_addrmode_widths(addrmode, &widths);
  arg2.width = widths.width2;
  arg2.pcrel_width = 0;
  arg1.width = widths.width1;
  arg1.pcrel_width = pcrel_width;

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
  int pcrel_width = 0;

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
    return print_insn(vaddr, info, nmemonic, addrmode, pcrel_width);
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
