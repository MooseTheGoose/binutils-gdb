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
#define MD_LEX_IND 	(IND_OPEN << 8)
#define MD_LEX_IND_LNG 	(IND_LNG_OPEN << 8)
#define MD_LEX_SYM 	(SYM_ID << 8)
#define MD_LEX_INDC 	(IND_CLOSE << 8)
#define MD_LEX_IND_LNGC (IND_LNG_CLOSE << 8)
#define MD_LEX_COMMA 	(COMMA_ID << 8)
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
    op->typ |= MD_LEX_IMM_FLAG;
}

#define SKIP_SPACES(s) while(*s && isspace(*s)) s++

static 
int parse_addrmode(char *start, 
  struct md_lexical_operand *lhs, struct md_lexical_operand *rhs)
{
  unsigned int id;
  int mode = MOS65XX_ADDRMODE_INVALID;
  
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
    mode = MOS65XX_ADDRMODE_IND;
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
      mode = MOS65XX_ADDRMODE_IND_IDX;
      SKIP_SPACES(start);
      mos65xx_construct_lex_symbol(&start, rhs);
      if(MD_DELIMS(rhs->typ) != REG_ID)
        as_fatal("Expected register for Indirect Indexing");
      id = MD_REGNUM(rhs->typ);
      if(id != MD_LEX_IDX && id != MD_LEX_S)
        as_fatal("Expected register S or X for Indexing");
      SKIP_SPACES(start);
      if(*start++ != IND_CLOSE)
        as_fatal("Expected ')' in Indexing");
      if(id == MD_LEX_S)
      {
        /* Stack relative indirect indexed */
        mode = MOS65XX_ADDRMODE_STK_IND_IDY;
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
        mode = MOS65XX_ADDRMODE_IND_IDY;
        if(*start++ != COMMA_ID)
          as_fatal("Expected ',' for DP Indirect Indexed Y");
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        if(MD_REGNUM(rhs->typ) != MD_LEX_IDY || MD_DELIMS(rhs->typ) != REG_ID)
          as_fatal("Expected 'Y' for DP Indirect Indexed Y");
        rhs->typ = MD_LEX_NONE;
      }
    }
    else
      as_fatal("Expected ',' or ')' in DP Indirect");
  }
  else if(*start == IND_LNG_OPEN)
  {
    mode = MOS65XX_ADDRMODE_IND_LNG;
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
      mode = MOS65XX_ADDRMODE_IND_LNG_IDY;
      start++;
      SKIP_SPACES(start);
      mos65xx_construct_lex_symbol(&start, rhs);
      if(MD_DELIMS(rhs->typ) != REG_ID || MD_REGNUM(rhs->typ) != MD_LEX_IDY)
        as_fatal("Expected 'Y' register for Indirect Long Indexed");
    }
  }
  else if(*start)
  {
    mode = MOS65XX_ADDRMODE_ACC;
    mos65xx_construct_lex_symbol(&start, lhs);
    id = MD_DELIMS(lhs->typ);
    if(id == REG_ID && MD_REGNUM(lhs->typ) != MD_LEX_A)
      as_fatal("Only A register can be used as first argument");
    else if(lhs->typ & MD_LEX_IMM_FLAG)
    {
      mode = MOS65XX_ADDRMODE_IMM;
      SKIP_SPACES(start);
      if(*start == COMMA_ID)
      {
        mode = MOS65XX_ADDRMODE_BLK;
        start++;
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        if((rhs->typ & MD_LEX_IMM_FLAG) == 0)
          as_fatal("Expected another immediate operand for block mode addressing");
      } 
    }
    else if(id != REG_ID)
    {
      int implied_width = MD_REGNUM(lhs->typ);
      int val = lhs->ex.X_add_number;
      if(implied_width == MD_LEX_NONE)
      {
        /* Replace magic numbers with names */
        if(MD_DELIMS(lhs->typ) == MD_LEX_SYM)
          implied_width = MD_LEX_LONG;
        else if(val <= UINT8_MAX && val >= INT8_MIN)
          implied_width = MD_LEX_BYTE;
        else if(val < UINT16_MAX && val >= INT16_MIN)
          implied_width = MD_LEX_WORD;
        else
          implied_width = MD_LEX_LONG;
      }
      switch(implied_width)
      {
        case MD_LEX_BYTE: mode = MOS65XX_ADDRMODE_PGE; break;
        case MD_LEX_WORD: mode = MOS65XX_ADDRMODE_ABS; break;
        case MD_LEX_LONG: mode = MOS65XX_ADDRMODE_LNG; break;
      }
      SKIP_SPACES(start);
      if(*start == COMMA_ID)
      {
        start++;
        SKIP_SPACES(start);
        mos65xx_construct_lex_symbol(&start, rhs);
        id = MD_REGNUM(rhs->typ);
        if(MD_DELIMS(rhs->typ) != REG_ID)
          as_fatal("Expected register for Absolute Indexing or Stack Relative");
        else if(id == MD_LEX_A)
          as_fatal("Expected 'X', 'Y', or 'S' registers for Absolute Indexing or Stack Relative");
        else if(id == MD_LEX_IDX)
        {
          switch(implied_width)
          {
            case MD_LEX_BYTE: mode = MOS65XX_ADDRMODE_IDX; break;
            case MD_LEX_WORD: mode = MOS65XX_ADDRMODE_ABS_IDX; break;
            case MD_LEX_LONG: mode = MOS65XX_ADDRMODE_ABS_LNG_IDX; break;
          }
        }
        else if(id == MD_LEX_IDY)
        {
          mode = (implied_width == MD_LEX_BYTE) ? MOS65XX_ADDRMODE_IDY : MOS65XX_ADDRMODE_ABS_IDY;
        }
        else
          mode = MOS65XX_ADDRMODE_STK_REL;
      }
    }
  }
  else
  {
    mode = MOS65XX_ADDRMODE_ACC;
  }

  SKIP_SPACES(start);
  if(*start)
    as_fatal("Encountered junk at the end of operand");
  return mode;
}

#define SKIP_SPACES(s) while(*s && isspace(*s)) s++

#define MAX_NMEMONIC_LEN 7

static const char *addrmode_to_string(int mode)
{
    const char *addrmode = "INVALID";
    switch(mode)
    {
      case MOS65XX_ADDRMODE_IND_IDX: addrmode = "Indirect Direct Page Indexed"; break;
      case MOS65XX_ADDRMODE_STK_REL: addrmode = "Stack Relative"; break;
      case MOS65XX_ADDRMODE_PGE: addrmode = "Direct Page"; break;
      case MOS65XX_ADDRMODE_IND_LNG: addrmode = "Indirect Long"; break;
      case MOS65XX_ADDRMODE_IMM: addrmode = "Immediate"; break;
      case MOS65XX_ADDRMODE_ABS: addrmode = "Absolute"; break;
      case MOS65XX_ADDRMODE_LNG: addrmode = "Absolute Long"; break;
      case MOS65XX_ADDRMODE_IND_IDY: addrmode = "Indirect Direct Page Indexed Y"; break;
      case MOS65XX_ADDRMODE_IDY: addrmode = "Direct Page Indexed Y"; break;
      case MOS65XX_ADDRMODE_IND: addrmode = "Indirect Direct Page"; break;
      case MOS65XX_ADDRMODE_STK_IND_IDY: addrmode = "Indirect Stack Relative Indexed Y"; break;
      case MOS65XX_ADDRMODE_IDX: addrmode = "Direct Page Indexed X"; break;
      case MOS65XX_ADDRMODE_IND_LNG_IDY: addrmode = "Indirect Long Indexed Y"; break;
      case MOS65XX_ADDRMODE_ABS_IDY: addrmode = "Absolute Indexed Y"; break;
      case MOS65XX_ADDRMODE_ABS_IDX: addrmode = "Absolute Indexed X"; break;
      case MOS65XX_ADDRMODE_ABS_LNG_IDX: addrmode = "Absolute Long Indexed X"; break;
      case MOS65XX_ADDRMODE_ACC: addrmode = "Accumulator"; break;
      case MOS65XX_ADDRMODE_BLK: addrmode = "Block"; break;
    }
    return addrmode;
}

void
md_assemble(char *line)
{
  char nmemonic[MAX_NMEMONIC_LEN + 1];
  char *token;
  char *save_lineptr = input_line_pointer;
  int opclass, opbase;

  printf("%s\n", line);

  /* Find the nmemonic */
  strncpy(nmemonic, line, MAX_NMEMONIC_LEN);
  nmemonic[MAX_NMEMONIC_LEN] = 0;
  token = nmemonic;
  while(*token && !isspace(*token))
    token++;
  *token = 0;
  token = nmemonic;
  line += strlen(token);
  struct mos65xx_op *opcode = mos65xx_opcode_lookup(token);
  if(!opcode)
    as_fatal("Unrecognized nmemonic!");

  opclass = MOS65XX_OPCODE_CLASS(opcode->flags);
  opbase = MOS65XX_OPCODE_BASE(opcode->flags);
  if(opclass & MOS65XX_MODEFLAG(MOS65XX_ADDRMODE_IMPLIED))
  {
    if(opbase == MOS65XX_WDM_BASE)
    {
      char *frag = frag_more(2);
      frag[0] = MOS65XX_WDM_BASE;
      frag[1] = 0;
    }
    else
      *(char *)frag_more(1) = MOS65XX_OPCODE_BASE(opcode->flags);
  }
  else
  {
    struct md_lexical_operand lhs, rhs;
    int mode = parse_addrmode(line, &lhs, &rhs);
    int opcode_prefix = 0;
    printf("Address Mode: %s\n", addrmode_to_string(mode));
    if(MD_DELIMS(lhs.typ) == SYM_ID || MD_DELIMS(rhs.typ) == SYM_ID)
      as_fatal("Ummm... We don't support symbols yet. *blush*");
    if((opclass & MOS65XX_MODEFLAG(mode)) == 0)
      as_fatal("Invalid addressing mode for operand");

    /* 
     *  This actually takes care of most instructions. 
     *  Thanks to CISC, it's not exactly that easy for
     *  all of them...
     */
    opcode_prefix = mode + opbase;
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
