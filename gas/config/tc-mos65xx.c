#include <as.h>
#include "tc-mos65xx.h"
#include "opcode/mos65xx.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

const char EXP_CHARS[] = "eE\0";
const char FLT_CHARS[] = "fF\0";

const char comment_chars[] = "\0";
const char line_comment_chars[] = ";\0";
const char line_separator_chars[] = "\0";

const char * md_shortopts = "\0";

static struct 
{
  int cpu_flags;
} mos65xx_ctx =
{
  .cpu_flags = MOS65XX_CPU_FLAG_EMULATION
};

struct option md_longopts[] = 
{
  {NULL, false, NULL, 0}
};
size_t md_longopts_size = (sizeof(md_longopts) / sizeof(md_longopts[0]));

int
md_parse_option(int optc ATTRIBUTE_UNUSED, const char* arg ATTRIBUTE_UNUSED)
{
  return 0;
}

void md_show_usage(FILE *f ATTRIBUTE_UNUSED)
{
}

void
md_begin()
{
  literal_prefix_dollar_hex = true;
}

static void mos65xx_lex_expr(char **s, expressionS *exp)
{
  char *suffix_delim;
  int X_md = 0;

  input_line_pointer = *s;
  suffix_delim = strchr(input_line_pointer, MOS65XX_FORCE_SIZE_SUFFIX);
  if(suffix_delim)
    *suffix_delim = 0;
  expression(exp);
  if(suffix_delim)
    *suffix_delim = MOS65XX_FORCE_SIZE_SUFFIX;

  if(exp->X_op == O_symbol)
  {
    const char *name = S_GET_NAME(exp->X_add_symbol);
    char test = toupper(*name);
    if(test && name[1] == 0)
    {
      exp->X_op = O_register;
      switch(test)
      {
        case 'A': exp->X_add_number = MOS65XX_REG_A; break;
        case 'X': exp->X_add_number = MOS65XX_REG_X; break;
        case 'Y': exp->X_add_number = MOS65XX_REG_Y; break;
        case 'S': exp->X_add_number = MOS65XX_REG_S; break;
        default: exp->X_op = O_symbol; break;
      } 
    }
  }
  else if(exp->X_op != O_constant)
  {
    if(exp->X_op == O_illegal)
      as_fatal("illegal expression");
    else if(exp->X_op == O_absent)
      as_fatal("expected expression here");
    else
      as_fatal("Complex expressions unsupported");
  }
  if(*input_line_pointer == MOS65XX_FORCE_SIZE_SUFFIX)
  {
    char test = toupper(input_line_pointer[1]);
    switch(test)
    {
      case MOS65XX_BYTE_SUFFIX: X_md |= MOS65XX_SIZEOF_BYTE; break;
      case MOS65XX_WORD_SUFFIX: X_md |= MOS65XX_SIZEOF_WORD; break;
      case MOS65XX_LONG_SUFFIX: X_md |= MOS65XX_SIZEOF_LONG; break;
      case MOS65XX_RELOC_DP_SUFFIX: X_md |= (MOS65XX_SIZEOF_WORD | MOS65XX_RELOC_DP); break;
      case MOS65XX_RELOC_BANK_SUFFIX: X_md |= (MOS65XX_SIZEOF_BYTE | MOS65XX_RELOC_BANK); break;
      default: as_fatal("Unrecognized size suffix"); break;
    }
    if(exp->X_op == O_register)
      as_fatal("Tried to force size of register");
    input_line_pointer += 2;
  }
  *s = input_line_pointer;
  exp->X_md = X_md;
}

#define SKIP_SPACES(s) 		while(*s && isspace(*s)) s++
#define MOS65XX_IND_OPEN  	'('
#define MOS65XX_IND_CLOSE 	')'
#define MOS65XX_IND_LNG_OPEN 	'['
#define MOS65XX_IND_LNG_CLOSE 	']'
#define MOS65XX_COMMA 		','
#define MOS65XX_IMM_PREFIX 	'#'
static 
void parse_operand(char *start, struct mos65xx_operand *op)
{
  unsigned int id;

  op->typ = MOS65XX_OPERAND_IMPLIED; 
  SKIP_SPACES(start);
  id = (*start == MOS65XX_IND_OPEN || *start == MOS65XX_IND_LNG_OPEN);
  if(id) start++;
  if(strchr(start, MOS65XX_IND_OPEN) || strchr(start, MOS65XX_IND_LNG_OPEN))
    as_fatal("Brackets must not be nested and must appear right after the nmemonic");
  if(id) start--;

  if(*start == MOS65XX_IND_OPEN)
  {
    op->typ = MOS65XX_OPERAND_IND;
    start++;
    SKIP_SPACES(start);
    mos65xx_lex_expr(&start, &op->lhs);
    if(op->lhs.X_op == O_register)
      as_fatal("Didn't expect register for first operand of indirect");
    SKIP_SPACES(start);
    id = *start++;
    if(id == MOS65XX_COMMA)
    {
      /* Expect either SR indexed or indirect X. Assume indirect X first. */
      op->typ = MOS65XX_OPERAND_IND_IDX;
      SKIP_SPACES(start);
      mos65xx_lex_expr(&start, &op->rhs);
      if(op->rhs.X_op != O_register || (op->rhs.X_add_number != MOS65XX_REG_S && op->rhs.X_add_number != MOS65XX_REG_X))
        as_fatal("Expected S,X register after address for Indirect Indexing");
      SKIP_SPACES(start);
      if(*start++ != MOS65XX_IND_CLOSE)
        as_fatal("Expected ')' in Indexing");
      if(op->rhs.X_add_number == MOS65XX_REG_S)
      {
        /* Stack relative indirect indexed */
        op->typ = MOS65XX_OPERAND_STK_IND_IDY;
        SKIP_SPACES(start);
        if(*start++ != MOS65XX_COMMA)
          as_fatal("Expected ',' in Stack Relative Indirect Indexed");
        SKIP_SPACES(start);
        mos65xx_lex_expr(&start, &op->rhs);
        if(op->rhs.X_op != O_register || op->rhs.X_add_number != MOS65XX_REG_Y)
          as_fatal("Expected 'Y' for Stack Relative Indirect Indexed"); 
      } 
    }
    else if(id == MOS65XX_IND_CLOSE)
    {
      /* Expect indirect Y or just indirect */
      SKIP_SPACES(start);
      if(*start)
      {
        op->typ = MOS65XX_OPERAND_IND_IDY;
        if(*start++ != MOS65XX_COMMA)
          as_fatal("Expected ',' for DP Indirect Indexed Y");
        SKIP_SPACES(start);
        mos65xx_lex_expr(&start, &op->rhs);
        if(op->rhs.X_op != O_register || op->rhs.X_add_number != MOS65XX_REG_Y)
          as_fatal("Expected 'Y' for DP Indirect Indexed Y");
      }
    }
    else
      as_fatal("Expected ',' or ')' in DP Indirect");
  }
  else if(*start == MOS65XX_IND_LNG_OPEN)
  {
    op->typ = MOS65XX_OPERAND_IND_LNG;
    start++;
    SKIP_SPACES(start);
    mos65xx_lex_expr(&start, &op->lhs);

    if(op->lhs.X_op == O_register)
      as_fatal("Didn't expect register for first operand of Indirect Long");

    SKIP_SPACES(start);
    if(*start++ != MOS65XX_IND_LNG_CLOSE)
      as_fatal("Expected ']' to close Indirect Long address");
    SKIP_SPACES(start);
    if(*start == MOS65XX_COMMA)
    {
      op->typ = MOS65XX_OPERAND_IND_LNG_IDY;
      start++;
      SKIP_SPACES(start);
      mos65xx_lex_expr(&start, &op->rhs);
      if(op->rhs.X_op != O_register || op->rhs.X_add_number != MOS65XX_REG_Y)
        as_fatal("Expected 'Y' register for Indirect Long Indexed");
    }
  }
  else if(*start == MOS65XX_IMM_PREFIX)
  {
    op->typ = MOS65XX_OPERAND_IMM;
    start++;
    mos65xx_lex_expr(&start, &op->lhs);
    if(op->lhs.X_op == O_register)
      as_fatal("Can't make immediate out of register");
  }
  else if(*start)
  {
    op->typ = MOS65XX_OPERAND_ACC;
    mos65xx_lex_expr(&start, &op->lhs);
    if(op->lhs.X_op == O_register && op->lhs.X_add_number != MOS65XX_REG_A)
      as_fatal("Only A register can be used as first argument");
    else if(op->lhs.X_op != O_register)
    {
      op->typ = MOS65XX_OPERAND_PGE;
      SKIP_SPACES(start);
      if(*start == MOS65XX_COMMA)
      {
        start++;
        SKIP_SPACES(start);
        mos65xx_lex_expr(&start, &op->rhs);
        if(op->rhs.X_op != O_register)
          op->typ = MOS65XX_OPERAND_BLK;
        else if(op->rhs.X_add_number == MOS65XX_REG_A || op->rhs.X_add_number == MOS65XX_REG_S)
          as_fatal("Expected 'X' or 'Y' register for Absolute Indexing");
        else if(op->rhs.X_add_number == MOS65XX_REG_X)
          op->typ = MOS65XX_OPERAND_IDX;
        else
          op->typ = MOS65XX_OPERAND_IDY;
      }
    }
  }

  SKIP_SPACES(start);
  if(*start)
    as_fatal("Encountered junk at the end of operand");
}

#define SKIP_SPACES(s) while(*s && isspace(*s)) s++

ATTRIBUTE_UNUSED static void 
print_operand(int operand)
{
  printf("Operand: ");
  switch(operand)
  {
    case MOS65XX_OPERAND_IND_IDX: printf("(addr,X)\n"); break;
    case MOS65XX_OPERAND_STK_REL: printf("#imm,S\n"); break;
    case MOS65XX_OPERAND_PGE: printf("addr\n"); break;
    case MOS65XX_OPERAND_IND_LNG_IDY: printf("[addr],Y\n"); break;
    case MOS65XX_OPERAND_IMM: printf("#imm\n"); break;
    case MOS65XX_OPERAND_IND_IDY: printf("(addr),Y\n"); break;
    case MOS65XX_OPERAND_IND: printf("(addr)\n"); break;
    case MOS65XX_OPERAND_STK_IND_IDY: printf("(ofs,S),Y\n"); break;
    case MOS65XX_OPERAND_IDX: printf("addr,X\n"); break;
    case MOS65XX_OPERAND_IND_LNG: printf("[addr]\n"); break;
    case MOS65XX_OPERAND_IDY: printf("addr,Y\n"); break;
    case MOS65XX_OPERAND_BLK: printf("#imm,#imm\n"); break;
    case MOS65XX_OPERAND_IMPLIED: printf("<implied>\n"); break;
    case MOS65XX_OPERAND_ACC: printf("A\n"); break;
    default: printf("INVALID\n"); break;
  }
}

struct ambiguous_mode_data
{
  uint8_t operand;
  uint8_t mode_8;
  uint8_t mode_16;
  uint8_t mode_24;
};

static int
coerce_operand_to_addrmode(int operand, uint32_t modeflags, int szof)
{
  int mode = operand;
  int i;

  static const struct ambiguous_mode_data ambiguous_modes[] = 
  {
    { MOS65XX_OPERAND_IND_IDX, MOS65XX_ADDRMODE_IND_IDX, MOS65XX_ADDRMODE_ABS_IND_IDX, MOS65XX_ADDRMODE_INVALID },
    { MOS65XX_OPERAND_PGE, MOS65XX_ADDRMODE_PGE, MOS65XX_ADDRMODE_ABS, MOS65XX_ADDRMODE_INVALID },
    { MOS65XX_OPERAND_IND, MOS65XX_ADDRMODE_IND, MOS65XX_ADDRMODE_ABS_IND, MOS65XX_ADDRMODE_INVALID },
    { MOS65XX_OPERAND_IDX, MOS65XX_ADDRMODE_IDX, MOS65XX_ADDRMODE_ABS_IDX, MOS65XX_ADDRMODE_ABS_LNG_IDX },
    { MOS65XX_OPERAND_IDY, MOS65XX_ADDRMODE_IDY, MOS65XX_ADDRMODE_ABS_IDY, MOS65XX_ADDRMODE_INVALID },
    { MOS65XX_OPERAND_IND_LNG, MOS65XX_ADDRMODE_IND_LNG, MOS65XX_ADDRMODE_ABS_IND_LNG, MOS65XX_ADDRMODE_INVALID },
    { MOS65XX_OPERAND_IMM, MOS65XX_ADDRMODE_IMM8, MOS65XX_ADDRMODE_IMM, MOS65XX_ADDRMODE_INVALID }
  };

  if(szof != 0 && szof != MOS65XX_SIZEOF_BYTE  && szof != MOS65XX_SIZEOF_WORD && szof != MOS65XX_SIZEOF_LONG)
    as_fatal("Expected size of operand to be 0, 1, 2, or 3 bytes");
 
  for(i = 0; i < (int)(sizeof(ambiguous_modes) / sizeof(ambiguous_modes[0])); i++)
  {
    const struct ambiguous_mode_data *data = ambiguous_modes + i;
    if(operand == data->operand)
    {
      uint8_t preferred_mode = (szof == MOS65XX_SIZEOF_LONG) ? data->mode_24 : ((szof == MOS65XX_SIZEOF_WORD) ? data->mode_16 : data->mode_8);
      if(modeflags & MOS65XX_MODEFLAG(preferred_mode))
        mode = preferred_mode;
      else if(modeflags & MOS65XX_MODEFLAG(data->mode_8))
        mode = data->mode_8;
      else if(modeflags & MOS65XX_MODEFLAG(data->mode_16))
        mode = data->mode_16;
      else
        mode = data->mode_24;
      break;
    }
  }
  return mode;
}

static uint8_t
generate_opcode_prefix(int *inout_addrmode, struct mos65xx_operand *operand, struct mos65xx_op opcode)
{
  int opcode_prefix;
  int addrmode = *inout_addrmode;
  if((MOS65XX_MODEFLAG(addrmode) & opcode.modeflags) == 0)
  {
    if(addrmode == MOS65XX_ADDRMODE_IMPLIED)
    {
      if(MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ACC) & opcode.modeflags)
        addrmode = MOS65XX_ADDRMODE_ACC;
      else if(MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IMM) & opcode.modeflags)
      {
        addrmode = MOS65XX_ADDRMODE_IMM;
        operand->lhs.X_op = O_constant;
        operand->lhs.X_add_number = 0;
        operand->lhs.X_md = MOS65XX_SIZEOF_BYTE;
      } 
    }
    if((MOS65XX_MODEFLAG(addrmode) & opcode.modeflags) == 0)
      as_fatal("Invalid addressing mode for instruction");
    *inout_addrmode = addrmode;
  }

  /* Prepare yourself for some magic... */ 
  opcode_prefix = opcode.base + mos65xx_addrmode_addends[addrmode];
  if(addrmode == MOS65XX_ADDRMODE_IMM)
  {
    if(opcode.opclass == MOS65XX_OPCLASS_IDX && opcode.base == MOS65XX_BIT_BASE)
      opcode_prefix = 0x89;
    else if(opcode.opclass == MOS65XX_OPCLASS_ALUMEM)
      opcode_prefix += 0x8;
  }
  else if(addrmode == MOS65XX_ADDRMODE_ACC)
  {
    if(opcode.opclass == MOS65XX_OPCLASS_BITOPS)
    {
      if(opcode.base == MOS65XX_INC_BASE)
        opcode_prefix = 0x1a;
      else if(opcode.base == MOS65XX_DEC_BASE)
        opcode_prefix = 0x3a;
    }
  }
  else if(opcode.opclass == MOS65XX_OPCLASS_STZ)
  {
    if(addrmode == MOS65XX_ADDRMODE_ABS)
      opcode_prefix = 0x9c;
    else if(addrmode == MOS65XX_ADDRMODE_ABS_IDX)
      opcode_prefix = 0x9e;
  }
  else if(opcode.opclass == MOS65XX_OPCLASS_JUMP)
  {
    opcode_prefix += (opcode_prefix - 0x4c) * 0x8;
  }
  else if(opcode.opclass == MOS65XX_OPCLASS_JSR)
  {
    if(addrmode == MOS65XX_ADDRMODE_ABS_IND_IDX)
      opcode_prefix = 0xfc;
  }
  return opcode_prefix & 0xff;
}

#define MOS65XX_RELOCFLAGS_PAGEOFS \
  (MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_PGE) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IND_LNG) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IND_IDX) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IND_IDY) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IND) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IDX) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IND_LNG_IDY) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IDY)) 

#define MOS65XX_RELOCFLAGS_ABS \
  (MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ABS) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ABS_IDY) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ABS_IDX) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ABS_IND) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ABS_IND_IDX) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_ABS_IND_LNG))

#define MOS65XX_RELOCFLAGS_STK_REL \
  (MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_STK_REL) \
  | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_STK_IND_IDY))

static void
emit_or_reloc_nonpcrel(char *frag, struct mos65xx_operand operand, 
  struct mos65xx_arg_widths widths, int addrmode)
{
  int r_type = BFD_RELOC_8;
  int modeflag = MOS65XX_MODEFLAG(addrmode);
  expressionS *exps[2] = { &operand.lhs, &operand.rhs };
  int widths_arr[2] = { widths.width1, widths.width2 };
  long unsigned int i;

  for(i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
  {
    expressionS *curr_exp = exps[i];
    int width = widths_arr[i];
    int X_md = curr_exp->X_md;
    if(width <= 0)
      continue;
      
    r_type = BFD_RELOC_NONE;
    
    switch(width)
    {
      case MOS65XX_SIZEOF_BYTE: r_type = BFD_RELOC_8; break;
      case MOS65XX_SIZEOF_WORD: r_type = BFD_RELOC_16; break;
      case MOS65XX_SIZEOF_LONG: r_type = BFD_RELOC_24; break;
      default:
        break;
    }
    
    if(modeflag & MOS65XX_RELOCFLAGS_PAGEOFS)
      r_type = BFD_RELOC_8_FFnn;
    else if(modeflag & MOS65XX_RELOCFLAGS_ABS)
      r_type = BFD_RELOC_MOS65XX_ABS;
    else if(modeflag & MOS65XX_RELOCFLAGS_STK_REL)
      r_type = BFD_RELOC_MOS65XX_STK_REL;  
    
    switch(MOS65XX_SIZEOF(X_md))
    {
      case MOS65XX_SIZEOF_BYTE: r_type = BFD_RELOC_8_FFnn; break;
      case MOS65XX_SIZEOF_WORD: r_type = BFD_RELOC_MOS65XX_ABS; break;
      case MOS65XX_SIZEOF_LONG: r_type = BFD_RELOC_24; break;
      default:
        break;
    }
    
    switch(MOS65XX_RELOC(X_md))
    {
      case MOS65XX_RELOC_DP: r_type = BFD_RELOC_MOS65XX_DPAGE; break;
      case MOS65XX_RELOC_BANK: r_type = BFD_RELOC_MOS65XX_BANK; break;
      default:
        break;
    }
    
    if(curr_exp->X_op == O_constant)
    {
      switch(r_type)
      {
        case BFD_RELOC_8_FFnn: curr_exp->X_add_number &= 0xff; break;
        case BFD_RELOC_MOS65XX_ABS: curr_exp->X_add_number &= 0xffff; break;
        case BFD_RELOC_MOS65XX_DPAGE: 
          curr_exp->X_add_number = (curr_exp->X_add_number) >> 8 & 0xffff; 
          break;
        case BFD_RELOC_MOS65XX_BANK:
          curr_exp->X_add_number = (curr_exp->X_add_number) >> 16 & 0xff;
          break;
        default:
        {
          int bound = (1 << (8 * width));
          if(curr_exp->X_add_number < -(bound >> 1) || curr_exp->X_add_number >= bound)
            as_warn("Overflow in constant expression");
          break;
        }
      }
      md_number_to_chars(frag, curr_exp->X_add_number, width);
    }
    else
    {
      fix_new_exp(frag_now, frag - frag_now->fr_literal, width, curr_exp, false, r_type);
    }
    frag += width;
  }
}

static void
emit_insn(int addrmode, uint8_t opcode_prefix, struct mos65xx_op opcode, struct mos65xx_operand operand)
{
  char *frag;
  struct mos65xx_arg_widths widths;
  mos65xx_addrmode_widths(addrmode, &widths, mos65xx_ctx.cpu_flags);
  frag = frag_more(1 + widths.width1 + widths.width2);
  *frag++ = opcode_prefix;

  if(opcode.pcrel_szof > 0)
    fix_new_exp(frag_now, frag - frag_now->fr_literal, opcode.pcrel_szof, &operand.lhs, true,
    	(opcode.pcrel_szof == MOS65XX_SIZEOF_BYTE) ? BFD_RELOC_8_PCREL : BFD_RELOC_16_PCREL);
  else
    emit_or_reloc_nonpcrel(frag, operand, widths, addrmode);
}

void
md_assemble(char *line)
{
  char *token;
  char *save_lineptr = input_line_pointer;
  struct mos65xx_op opcode;
  int default_width = MOS65XX_SIZEOF_BYTE;

  /* printf("%s\n", line); */

  /* Find the nmemonic */
  token = strchr(line, ' ');
  if(token) *token = 0;
  if(mos65xx_opcode_lookup(line, &opcode) == NULL)
    as_fatal("Unrecognized nmemonic!");
  if(token)
  {
    line = token;
    *token = ' ';
  }
  else
    line += strlen(line);
  
  int opcode_prefix;
  struct mos65xx_operand operand;
  parse_operand(line, &operand);
  if(MOS65XX_SIZEOF(operand.lhs.X_md) == 0 && operand.lhs.X_op == O_constant)
  {
    default_width = MOS65XX_SIZEOF_LONG;
    if(operand.lhs.X_add_number < 0x10000 && operand.lhs.X_add_number >= -0x8000)
    {
      if(operand.lhs.X_add_number < 0x100 && operand.lhs.X_add_number >= -0x80)
        default_width = MOS65XX_SIZEOF_BYTE;
      else
        default_width = MOS65XX_SIZEOF_WORD;
    }
  }
  int addrmode = coerce_operand_to_addrmode(operand.typ, opcode.modeflags, default_width);
  opcode_prefix = generate_opcode_prefix(&addrmode, &operand, opcode);
  emit_insn(addrmode, opcode_prefix, opcode, operand);

  input_line_pointer = save_lineptr; 
}

void
md_apply_fix(fixS * fixP ATTRIBUTE_UNUSED, valueT* valP ATTRIBUTE_UNUSED, segT seg ATTRIBUTE_UNUSED)
{
  long val = *valP;
  char *rel = fixP->fx_where + fixP->fx_frag->fr_literal;

  if(fixP->fx_addsy == NULL)
    fixP->fx_done = 1;
  if(fixP->fx_pcrel)
  {
    segT fix_seg = S_GET_SEGMENT(fixP->fx_addsy);
    if(fix_seg == seg || fix_seg == absolute_section)
    {
      val += S_GET_VALUE(fixP->fx_addsy);
      fixP->fx_done = 1;
    }
  }
  switch(fixP->fx_r_type)
  {
    case BFD_RELOC_8_PCREL:
    case BFD_RELOC_16_PCREL:
      fixP->fx_no_overflow = 0;
      break;
    default:
      fixP->fx_no_overflow = 1;
      break;
  }

  switch(fixP->fx_r_type)
  {
    case BFD_RELOC_8_PCREL:
      if(fixP->fx_done && (val < -0x80 || val >= 0x80))
        as_bad_where(fixP->fx_file, fixP->fx_line, "8-bit signed offset out of range!");
      *rel++ = val & 0xff;
      break;
    case BFD_RELOC_16_PCREL:
      if(fixP->fx_done && (val < -0x8000 || val >= 0x8000))
        as_bad_where(fixP->fx_file, fixP->fx_line, "16-bit signed offset out of range!");
      *rel++ = val & 0xff;
      *rel++ = val >> 8 & 0xff;
      break;
    case BFD_RELOC_8:
      if(fixP->fx_done && (val < -0x80 || val >= 0x100))
        as_warn_where(fixP->fx_file, fixP->fx_line, "8-bit relocation out of range");
      *rel++ = val & 0xff; 
      break;
    case BFD_RELOC_16:
      if(fixP->fx_done && (val < -0x8000 || val >= 0x1000))
        as_warn_where(fixP->fx_file, fixP->fx_line, "16-bit relocation out of range");
      *rel++ = val & 0xff;
      *rel++ = val >> 8 & 0xff;
      break;
    case BFD_RELOC_24:
      if(fixP->fx_done && (val < -0x800000 || val >= 0x1000000))
        as_warn_where(fixP->fx_file, fixP->fx_line, "24-bit relocation out of range");
      *rel++ = val & 0xff;
      *rel++ = val >> 8 & 0xff;
      *rel++ = val >> 16 & 0xff;
      break;
    case BFD_RELOC_8_FFnn:
      *rel++ = val & 0xff;
      break;
    case BFD_RELOC_MOS65XX_DPAGE:
      *rel++ = val >> 8 & 0xffff;
      break;
    case BFD_RELOC_MOS65XX_BANK:
      *rel++ = val >> 16 & 0xff;
      break; 
    case BFD_RELOC_MOS65XX_STK_REL:
      if(fixP->fx_done && (val < 0 || val >= 0x100))
        as_warn_where(fixP->fx_file, fixP->fx_line, "Stack-relative offset out of range");
      *rel++ = val & 0xff;
      break;
    default:
      break;
  }
}

const char *
md_atof(int type, char *litP, int *sizeP)
{
  return ieee_md_atof(type, litP, sizeP, false);
}

long
md_pcrel_from(fixS * fixp)
{
  return fixp->fx_where + fixp->fx_frag->fr_address + fixp->fx_size;
}

const pseudo_typeS md_pseudo_table[] =
{
  { "int", cons, 3 },
  { "long", cons, 3 },
  { "int32", cons, 4 },
  { NULL, 0, 0 }
};

valueT md_section_align(segT seg ATTRIBUTE_UNUSED, valueT size)
{
  return size;
}

symbolS *
md_undefined_symbol(char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

arelent *
tc_gen_reloc(asection *seg ATTRIBUTE_UNUSED, fixS *fixp ATTRIBUTE_UNUSED)
{
  arelent *reloc;

  reloc = XNEW(arelent);
  reloc->sym_ptr_ptr = XNEW(asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym(fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->addend = fixp->fx_offset;
  reloc->howto = bfd_reloc_type_lookup(stdoutput, fixp->fx_r_type);
  if(reloc->howto == NULL)
  {
    as_bad_where(fixp->fx_file, fixp->fx_line, 
    	"Reloc not supported by object file format");
    return NULL;
  }
  return reloc;
}
