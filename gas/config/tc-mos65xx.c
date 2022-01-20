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
const char line_comment_chars[] = "#;\0";
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
#define MD_REG_MASK 	0xff
#define MD_CONST_MASK 	0xff
#define MD_LEX_IND 	0x100
#define MD_LEX_IND_LNG 	0x200
#define MD_LEX_IMM 	0x400
#define MD_LEX_SYM 	0x800

#define IND_OPEN 	'('
#define IND_CLOSE 	')'
#define IND_LNG_OPEN 	'['
#define IND_LNG_CLOSE 	']'
 
struct md_lexical_operand 
{
  uint16_t typ;
  expressionS ex;
};

static struct md_lexical_operand mos65xx_construct_lex_symbol(char **s)
{
  struct md_lexical_operand op;
  char *suffix_delim;
  op.typ = MD_LEX_NONE;
  input_line_pointer = *s;
  suffix_delim = strchr(input_line_pointer, FORCE_SIZE_SUFFIX);
  if(suffix_delim)
    *suffix_delim = 0;
  expression(&op.ex);
  if(suffix_delim)
    *suffix_delim = FORCE_SIZE_SUFFIX;

  if(op.ex.X_op == O_symbol)
  {
    op.typ |= MD_LEX_SYM;
    const char *name = S_GET_NAME(op.ex.X_add_symbol);
    if((*name == MD_LEX_A || *name == MD_LEX_IDX || *name == MD_LEX_IDY || *name == MD_LEX_S)
  	&& name[1] == 0)
    {
      op.typ |= *name;
      op.typ &= ~MD_LEX_SYM;
    }
  }
  else if(op.ex.X_op != O_constant)
  {
    if(op.ex.X_op == O_illegal)
      as_fatal("illegal expression");
    else if(op.ex.X_op == O_absent)
      as_fatal("expected expression here");
    else
      as_fatal("Complex expressions unsupported");
  }
  if(*input_line_pointer == FORCE_SIZE_SUFFIX)
  {

    if(input_line_pointer[1] == MD_LEX_BYTE 
  	|| input_line_pointer[1] == MD_LEX_WORD || input_line_pointer[1] == MD_LEX_LONG)
    {
      if(op.typ & MD_REG_MASK)
    	as_fatal("Tried to force size of register");
      op.typ |= suffix_delim[1];
      input_line_pointer += 2;
    }
    else
    {
      as_fatal("Unrecognized size suffix");
    }
  }
  *s = input_line_pointer;
  return op;
}

#define SKIP_SPACES(s) while(*s && isspace(*s)) s++

static struct mos65xx_addrmode parse_addrmode(char *start)
{
  struct md_lexical_operand lex;
  uint16_t init_lextype = MD_LEX_NONE;
  struct mos65xx_addrmode mode = { MOS65XX_ADDRMODE_INVALID, 0, 0 };

  SKIP_SPACES(start);
  switch(*start++)
  {
    case IND_OPEN: init_lextype = MD_LEX_IND; break;
    case IND_LNG_OPEN: init_lextype = MD_LEX_IND_LNG; break;
    default: start--;
  }
  if(strchr(start, IND_OPEN) || strchr(start, IND_LNG_OPEN))
    as_fatal("Brackets may not be nested and must appear right after the nmemonic");
  
  lex = mos65xx_construct_lex_symbol(&start);
  SKIP_SPACES(start);
  return mode;
}

#define SKIP_SPACES(s) while(*s && isspace(*s)) s++

#define MAX_NMEMONIC_LEN 7

void
md_assemble(char *line)
{
  char nmemonic[MAX_NMEMONIC_LEN + 1];
  char *token;
  char *save_lineptr = input_line_pointer;

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

  if(MOS65XX_OPCODE_TYPE(opcode->flags) == MOS65XX_OPCODE_NOARGS)
  {
    *(uint8_t *)frag_more(1) = MOS65XX_OPCODE_BASE(opcode->flags);
    printf("No arguments. Nothing to do.\n");
  }
  else
  {
    struct mos65xx_addrmode mode ATTRIBUTE_UNUSED = parse_addrmode(line); 
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
