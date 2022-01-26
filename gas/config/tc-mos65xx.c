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

void md_show_usage(FILE *f)
{
  fprintf(f, "HELLO WORLD\n");
}

void
md_begin()
{
  literal_prefix_dollar_hex = true;
}

static void mos65xx_lex_expr(char **s, expressionS *exp)
{
  char *suffix_delim;
  char prefix;
  int X_md = 0;

  input_line_pointer = *s;
  prefix = *input_line_pointer;
  if(prefix == MOS65XX_IMM_PREFIX)
  {
    X_md |= MOS65XX_IMMFLAG;
    input_line_pointer++;
  }
  
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
  if(X_md & MOS65XX_IMMFLAG)
    if(exp->X_op == O_register)
      as_fatal("Tried to make register an immediate");
  exp->X_md = X_md;
}

#define SKIP_SPACES(s) 		while(*s && isspace(*s)) s++
#define MOS65XX_IND_OPEN  	'('
#define MOS65XX_IND_CLOSE 	')'
#define MOS65XX_IND_LNG_OPEN 	'['
#define MOS65XX_IND_LNG_CLOSE 	']'
#define MOS65XX_COMMA 		','
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
      if(MOS65XX_IMM(op->lhs.X_md) && (op->rhs.X_op != O_register || op->rhs.X_add_number != MOS65XX_REG_S))
        as_fatal("Expected S register after immediate for Stack Relative Indexing");
      else if(!MOS65XX_IMM(op->lhs.X_md) && (op->rhs.X_op != O_register || op->rhs.X_add_number != MOS65XX_REG_X))
        as_fatal("Expected X register after address for Indirect Indexing");
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
      if(MOS65XX_IMM(op->lhs.X_md))
        as_fatal("Unexpected immediate in Indirect operand");
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
    else if(MOS65XX_IMM(op->lhs.X_md))
      as_fatal("Didn't expect immediate for first operand of Indirect Long");

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
  else if(*start)
  {
    op->typ = MOS65XX_OPERAND_ACC;
    mos65xx_lex_expr(&start, &op->lhs);
    if(op->lhs.X_op == O_register && op->lhs.X_add_number != MOS65XX_REG_A)
      as_fatal("Only A register can be used as first argument");
    else if(MOS65XX_IMM(op->lhs.X_md))
    {
      op->typ = MOS65XX_OPERAND_IMM;
      SKIP_SPACES(start);
      if(*start == MOS65XX_COMMA)
      {
        start++;
        SKIP_SPACES(start);
        mos65xx_lex_expr(&start, &op->rhs);
        if(MOS65XX_IMM(op->rhs.X_md))
          op->typ = MOS65XX_OPERAND_BLK;
        else if(op->rhs.X_op == O_register  && op->rhs.X_add_number == MOS65XX_REG_S)
          op->typ = MOS65XX_OPERAND_STK_REL;
        else
          as_fatal("Expected block or stack relative addressing mode");
      } 
    }
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
          as_fatal("Expected register for Absolute Indexing");
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

static void print_operand(int operand)
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
    case MOS65XX_OPERAND_STK_IND_IDY: printf("(#imm,S),Y\n"); break;
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
    { MOS65XX_OPERAND_IND_LNG, MOS65XX_ADDRMODE_IND_LNG, MOS65XX_ADDRMODE_ABS_IND_LNG, MOS65XX_ADDRMODE_INVALID }
  };

  if(szof != 0 && szof != MOS65XX_SIZEOF_BYTE  && szof != MOS65XX_SIZEOF_WORD && szof != MOS65XX_SIZEOF_LONG)
    as_fatal("Expected size of operand to be 0, 1, 2, or 3 bytes");
 
  for(i = 0; i < (int)(sizeof(ambiguous_modes) / sizeof(ambiguous_modes[0])); i++)
  {
    const struct ambiguous_mode_data *data = ambiguous_modes + i;
    if(operand == data->operand)
    {
      uint8_t preferred_mode = (szof == MOS65XX_SIZEOF_BYTE) ? data->mode_8 : ((szof == MOS65XX_SIZEOF_WORD) ? data->mode_16 : data->mode_24);
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
        operand->lhs.X_md = (MOS65XX_SIZEOF_BYTE | MOS65XX_IMMFLAG);
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

static void
emit_insn(int addrmode, uint8_t opcode_prefix, struct mos65xx_operand operand)
{
  char *frag;
  int32_t lhs_val = operand.lhs.X_add_number;
  int32_t rhs_val = operand.rhs.X_add_number;
  struct mos65xx_arg_widths widths;
  mos65xx_addrmode_widths(addrmode, &widths);
  frag = frag_more(1 + widths.width1 + widths.width2);
  frag[0] = opcode_prefix;
  if(widths.width1 != 0)
    md_number_to_chars(frag + 1, lhs_val, widths.width1); 
  if(widths.width2 != 0)
    md_number_to_chars(frag + 1 + widths.width1, rhs_val, widths.width2);
}

void
md_assemble(char *line)
{
  char *token;
  char *save_lineptr = input_line_pointer;
  struct mos65xx_op opcode;

  printf("%s\n", line);

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
  
  if(opcode.modeflags & MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IMPLIED))
  {
    if(opcode.base == MOS65XX_WDM_BASE)
    {
      char *frag = frag_more(2);
      frag[0] = MOS65XX_WDM_BASE;
      frag[1] = 0;
    }
    else
      *(char *)frag_more(1) = opcode.base;
  }
  else
  {
    int opcode_prefix;
    struct mos65xx_operand operand;
    parse_operand(line, &operand);
    if(operand.rhs.X_op == O_symbol || operand.lhs.X_op == O_symbol)
      as_fatal("Ummm... We don't support symbols yet. *blush*");
    print_operand(operand.typ);
    printf("Preferred Size: %d\n", MOS65XX_SIZEOF(operand.lhs.X_md));
    int addrmode = coerce_operand_to_addrmode(operand.typ, opcode.modeflags, MOS65XX_SIZEOF(operand.lhs.X_md));

    printf("modeflags: %x\n", opcode.modeflags);
    printf("mode: %x\n", MOS65XX_MODEFLAG(addrmode));
    opcode_prefix = generate_opcode_prefix(&addrmode, &operand, opcode);

    if(opcode.pcrel_szof > 0)
      as_fatal("PC Relative addressing is just so much nope right now...");
    emit_insn(addrmode, opcode_prefix, operand);
  }

  input_line_pointer = save_lineptr; 
}

void
md_apply_fix(fixS * fixP ATTRIBUTE_UNUSED, valueT* valP ATTRIBUTE_UNUSED, segT seg ATTRIBUTE_UNUSED)
{
  printf("Me? Do a fixup? Whaaaaaaaaaa\n");
  seg = (segT)((uintptr_t)fixP + valP);
}

const char *
md_atof(int type, char *litP, int *sizeP)
{
  return ieee_md_atof(type, litP, sizeP, false);
}

long
md_pcrel_from(fixS * fixp)
{
  return fixp->fx_where + fixp->fx_frag->fr_address;
}

const pseudo_typeS md_pseudo_table[] =
{
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
  return NULL;
}
