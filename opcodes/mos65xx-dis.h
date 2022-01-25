#ifndef MOS65XX_DIS
#define MOS65XX_DIS

#include "opcode/mos65xx.h"

/*
 *  Implied instructions don't have a
 *  pattern so we have to do a table
 *  lookup for disassembly.
 */
struct mos65xx_implied_lookup
{
  const char *name;
  uint8_t opcode;
};
#define MOS65XX_IMPCREATE(name) { #name, MOS65XX_ ## name ## _BASE }

#define MOS65XX_ARG_SLEN 	32
struct mos65xx_arg_str 
{
  char arg[MOS65XX_ARG_SLEN];
  int width;
  bool pcrel;
};

#endif
