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

#define FORCE_SIZE_SUFFIX ':'

#define MD_LEX_NONE 	0x0
#define MD_LEX_IDX 	'X'
#define MD_LEX_IDY 	'Y'
#define MD_LEX_A 	'A'
#define MD_LEX_S 	'S'
#define MD_LEX_BYTE 	'B'
#define MD_LEX_WORD 	'W'
#define MD_LEX_LONG 	'L'
#define MD_REGNUM(x) 	((x) & 0x7f)
#define MD_LEX_SYM 	(SYM_ID << 8)
#define MD_LEX_REG 	(REG_ID << 8)
#define MD_LEX_ADDR 	(ADDR_ID << 8)
#define MD_DELIMS(x) 	((x) >> 8 & 0x7f)
#define MD_LEX_IMM_FLAG 0x8000

#define IND_OPEN 	'('
#define IND_CLOSE 	')'
#define IND_LNG_OPEN 	'['
#define IND_LNG_CLOSE 	']'
#define IMM_PRE  	'#'
#define SYM_ID 		'M'
#define COMMA_ID 	','
#define REG_ID 		'R'
#define ADDR_ID 	'0'
 
struct md_lexical_operand 
{
  uint16_t typ;
  expressionS ex;
};

static void mos65xx_construct_lex_symbol(char **s, struct md_lexical_operand *op)
{
  char *suffix_delim;
  char prefix;
  input_line_pointer = *s;
  prefix = *input_line_pointer;
  if(prefix == IMM_PRE)
    input_line_pointer++;
  suffix_delim = strchr(input_line_pointer, FORCE_SIZE_SUFFIX);
  if(suffix_delim)
    *suffix_delim = 0;
  expression(&op->ex);
  if(suffix_delim)
    *suffix_delim = FORCE_SIZE_SUFFIX;

  if(op->ex.X_op == O_symbol)
  {
    op->typ = MD_LEX_SYM;
    const char *name = S_GET_NAME(op->ex.X_add_symbol);
    char test = toupper(*name);
    if((test == MD_LEX_A || test == MD_LEX_IDX || test == MD_LEX_IDY || test == MD_LEX_S)
  	&& name[1] == 0)
    {
      op->typ = (test | MD_LEX_REG);
    }
  }
  else if(op->ex.X_op != O_constant)
  {
    if(op->ex.X_op == O_illegal)
      as_fatal("illegal expression");
    else if(op->ex.X_op == O_absent)
      as_fatal("expected expression here");
    else
      as_fatal("Complex expressions unsupported");
  }
  if(*input_line_pointer == FORCE_SIZE_SUFFIX)
  {
    char test = toupper(input_line_pointer[1]);
    if(test == MD_LEX_BYTE || test == MD_LEX_WORD || test == MD_LEX_LONG)
    {
      if(MD_DELIMS(op->typ) == REG_ID)
    	as_fatal("Tried to force size of register");
      op->typ |= test;
      input_line_pointer += 2;
    }
    else
    {
      as_fatal("Unrecognized size suffix");
    }
  }
  *s = input_line_pointer;
  if(prefix == IMM_PRE)
  {
    if(MD_DELIMS(op->typ) == REG_ID)
      as_fatal("Tried to make register an immediate");
    op->typ |= MD_LEX_IMM_FLAG;
  }
}

#define SKIP_SPACES(s) while(*s && isspace(*s)) s++

static 
int parse_operand(char *start, 
  struct md_lexical_operand *lhs, struct md_lexical_operand *rhs)
{
  unsigned int id;
  int mode = MOS65XX_OPERAND_IMPLIED;
  
  lhs->typ = MD_LEX_NONE;
  rhs->typ = MD_LEX_NONE;
  SKIP_SPACES(start);
  id = (*start == IND_OPEN || *start == IND_LNG_OPEN);
  if(id) start++;
  if(strchr(start, IND_OPEN) || strchr(start, IND_LNG_OPEN))
    as_fatal("Brackets must not be nested and must appear right after the nmemonic");
  if(id) start--;

  if(*start == IND_OPEN)
  {
    mode = MOS65XX_OPERAND_IND;
    start++;
    SKIP_SPACES(start);
    mos65xx_construct_lex_symbol(&start, lhs);
    if(MD_DELIMS(lhs->typ) == REG_ID)
      as_fatal("Didn't expect register for first operand of indirect");
    SKIP_SPACES(start);
    id = *start++;
    if(id == COMMA_ID)
    {
      /* Expect either SR indexed or indirect X. Assume indirect X first. */
      mode = MOS65XX_OPERAND_IND_IDX;
      SKIP_SPACES(start);
      mos65xx_construct_lex_symbol(&start, rhs);
      id = MD_REGNUM(rhs->typ);
      if((lhs->typ & MD_LEX_IMM_FLAG) && MD_DELIMS(rhs->typ) != REG_ID && id != MD_LEX_S)
        as_fatal("Expected S register after immediate for Stack Relative Indexing");
      else if(MD_DELIMS(rhs->typ) != REG_ID && id != MD_LEX_IDX && id != MD_LEX_S)
        as_fatal("Expected X register after address for Indirect Indexing");
      SKIP_SPACES(start);
      if(*start++ != IND_CLOSE)
        as_fatal("Expected ')' in Indexing");
      if(id == MD_LEX_S)
      {
        /* Stack relative indirect indexed */
        mode = MOS65XX_OPERAND_STK_IND_IDY;
        SKIP_SPACES(start);
        if(*start++ != COMMA_ID)
          as_fatal("Expected ',' in Stack Relative Indirect Indexed");
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        if(MD_REGNUM(rhs->typ) != MD_LEX_IDY || MD_DELIMS(rhs->typ) != REG_ID)
          as_fatal("Expected 'Y' for Stack Relative Indirect Indexed"); 
      } 
    }
    else if(id == IND_CLOSE)
    {
      /* Expect indirect Y or just indirect */
      SKIP_SPACES(start);
      if(*start)
      {
        mode = MOS65XX_OPERAND_IND_IDY;
        if(*start++ != COMMA_ID)
          as_fatal("Expected ',' for DP Indirect Indexed Y");
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        if(MD_REGNUM(rhs->typ) != MD_LEX_IDY || MD_DELIMS(rhs->typ) != REG_ID)
          as_fatal("Expected 'Y' for DP Indirect Indexed Y");
      }
    }
    else
      as_fatal("Expected ',' or ')' in DP Indirect");
  }
  else if(*start == IND_LNG_OPEN)
  {
    mode = MOS65XX_OPERAND_IND_LNG;
    start++;
    SKIP_SPACES(start);
    mos65xx_construct_lex_symbol(&start, lhs);

    if(MD_DELIMS(lhs->typ) == REG_ID)
      as_fatal("Didn't expect register for first operand of Indirect Long");

    SKIP_SPACES(start);
    if(*start++ != IND_LNG_CLOSE)
      as_fatal("Expected ']' to close Indirect Long address");
    SKIP_SPACES(start);
    if(*start == COMMA_ID)
    {
      mode = MOS65XX_OPERAND_IND_LNG_IDY;
      start++;
      SKIP_SPACES(start);
      mos65xx_construct_lex_symbol(&start, rhs);
      if(MD_DELIMS(rhs->typ) != REG_ID || MD_REGNUM(rhs->typ) != MD_LEX_IDY)
        as_fatal("Expected 'Y' register for Indirect Long Indexed");
    }
  }
  else if(*start)
  {
    mode = MOS65XX_OPERAND_ACC;
    mos65xx_construct_lex_symbol(&start, lhs);
    id = MD_DELIMS(lhs->typ);
    if(id == REG_ID && MD_REGNUM(lhs->typ) != MD_LEX_A)
      as_fatal("Only A register can be used as first argument");

    /* Consider making stack relative use an immediate instead of an address */
    else if(lhs->typ & MD_LEX_IMM_FLAG)
    {
      mode = MOS65XX_OPERAND_IMM;
      SKIP_SPACES(start);
      if(*start == COMMA_ID)
      {
        start++;
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        if(rhs->typ & MD_LEX_IMM_FLAG)
          mode = MOS65XX_OPERAND_BLK;
        else if(MD_DELIMS(rhs->typ) == REG_ID && MD_REGNUM(rhs->typ) == MD_LEX_S)
          mode = MOS65XX_OPERAND_STK_REL;
        else
          as_fatal("Expected block or stack relative addressing mode");
      } 
    }
    else if(id != REG_ID)
    {
      mode = MOS65XX_OPERAND_PGE;
      SKIP_SPACES(start);
      if(*start == COMMA_ID)
      {
        start++;
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        id = MD_REGNUM(rhs->typ);
        if(MD_DELIMS(rhs->typ) != REG_ID)
          as_fatal("Expected register for Absolute Indexing");
        else if(id == MD_LEX_A || id == MD_LEX_S)
          as_fatal("Expected 'X' or 'Y' register for Absolute Indexing");
        else if(id == MD_LEX_IDX)
          mode = MOS65XX_OPERAND_IDX;
        else
          mode = MOS65XX_OPERAND_IDY;
      }
    }
  }

  SKIP_SPACES(start);
  if(*start)
    as_fatal("Encountered junk at the end of operand");
  return mode;
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
coerce_operand_to_addrmode(int operand, uint32_t modeflags, int width)
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

  /* TODO: Replace magic numbers wtih MOS65XX_BYTE_WIDTH, MOS65XX_WORD_WIDTH, MOS65XX_LONG_WIDTH */
  if(width != 0 && width != 8 && width != 16 && width != 24)
    as_fatal("Expected width to be 0, 8, 16, or 24 bits");
 
  for(i = 0; i < (int)(sizeof(ambiguous_modes) / sizeof(ambiguous_modes[0])); i++)
  {
    const struct ambiguous_mode_data *data = ambiguous_modes + i;
    if(operand == data->operand)
    {
      uint8_t preferred_mode = (width == 8) ? data->mode_8 : ((width == 16) ? data->mode_16 : data->mode_24);
      if(preferred_mode != MOS65XX_ADDRMODE_INVALID && (modeflags & MOS65XX_MODEFLAG(preferred_mode)))
        mode = preferred_mode;
      else if(data->mode_8 != MOS65XX_ADDRMODE_INVALID && (modeflags & MOS65XX_MODEFLAG(data->mode_8)))
        mode = data->mode_8;
      else if(data->mode_16 != MOS65XX_ADDRMODE_INVALID && (modeflags & MOS65XX_MODEFLAG(data->mode_16)))
        mode = data->mode_16;
      else
        mode = data->mode_24;
      break;
    }
  }
  return mode;
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
    struct md_lexical_operand lhs, rhs;
    int opcode_prefix;
    int operand = parse_operand(line, &lhs, &rhs);
    if(MD_DELIMS(lhs.typ) == SYM_ID || MD_DELIMS(rhs.typ) == SYM_ID)
      as_fatal("Ummm... We don't support symbols yet. *blush*");
    print_operand(operand);
    int addrmode = coerce_operand_to_addrmode(operand, opcode.modeflags, 0);

    printf("modeflags: %x\n", opcode.modeflags);
    printf("mode: %x\n", MOS65XX_MODEFLAG(addrmode));
    if((MOS65XX_MODEFLAG(addrmode) & opcode.modeflags) == 0)
    {
      if(addrmode == MOS65XX_ADDRMODE_IMPLIED)
      {
        if(opcode.opclass == MOS65XX_OPCLASS_BITOPS)
          addrmode = MOS65XX_ADDRMODE_ACC;
      }
      if((MOS65XX_MODEFLAG(addrmode) & opcode.modeflags) == 0)
        as_fatal("Invalid addressing mode for instruction");
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
    opcode_prefix &= 0xff;

    if(opcode.modeflags & (MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_PCREL) | MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_PCREL16)))
      as_fatal("PC Relative addressing is just so much nope right now...");
    else if(addrmode == MOS65XX_ADDRMODE_BLK)
      as_fatal("Block addressing is also just so much nope right now...");
    else
    {
      /*
       *  This very clearly does not work.
       *  This is just a placeholder for now
       *  while I think of how to refactor this
       *  assembler...
       */
      int frag_bytes = 1;
      char *frag = frag_more(frag_bytes);
      frag[0] = opcode_prefix;
    }
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
