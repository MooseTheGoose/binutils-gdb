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

#endif
