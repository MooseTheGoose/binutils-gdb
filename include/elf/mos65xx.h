#ifndef _ELF_MOS65XX_H
#define _ELF_MOS655X_H

#include "elf/reloc-macros.h"

#define EF_MOS65XX_MACH_6502	0x1

START_RELOC_NUMBERS(elf_mos65xx_reloc_type)
	RELOC_NUMBER(R_MOS65XX_NONE,	0)
END_RELOC_NUMBERS(R_MOS65XX_MAX)

#endif
