#ifndef MOS65XX_DIS
#define MOS65XX_DIS

#include "opcode/mos65xx.h"

struct mos65816_disas
{
  const char *nmemonic;
  uint8_t addrmode;
  uint8_t pcrel_szof;
};
#define MOS65XX_DIS_AND_PCREL(name, addrmode, pcrel) \
{ name, MOS65XX_ADDRMODE_ ## addrmode , pcrel }

#define MOS65816_DIS(name, addrmode) MOS65XX_DIS_AND_PCREL(name, addrmode, 0)

#define MOS65816_PCREL_DIS(name, addrmode) \
MOS65XX_DIS_AND_PCREL(name, addrmode, MOS65XX_SIZEOF_BYTE)

#define MOS65816_PCREL16_DIS(name, addrmode) \
MOS65XX_DIS_AND_PCREL(name, addrmode, MOS65XX_SIZEOF_WORD)
 
#define MOS65XX_ARG_SLEN 	32
struct mos65xx_arg_str 
{
  char arg[MOS65XX_ARG_SLEN];
  int width;
  int pcrel_width;
};

#endif
