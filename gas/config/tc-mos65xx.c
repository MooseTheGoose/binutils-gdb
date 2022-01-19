#include <as.h>
#include "tc-mos65xx.h"
#include <stdio.h>

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
}


void
md_assemble(char *line)
{
  printf("%s\n", line);
}

void
md_apply_fix(fixS * fixP, valueT* valP, segT seg)
{
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
