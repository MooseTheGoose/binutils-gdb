
#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_mos65xx_arch;

#define N(mach, name, bits, default, next) \
{ 16, bits, 8, bfd_arch_mos65xx, mach, "mos65xx", name, 0, default, \
  bfd_default_compatible, bfd_default_scan, bfd_arch_default_fill, \
  next, 0 }
  
#define M(n) &arch_info_struct[n]

/*
static const bfd_arch_info_type arch_info_struct[] =
{
  N(bfd_mach_6502, "6502", 16, true, 0)
};
*/

/*
const bfd_arch_info_type bfd_mos65xx_arch = 
  N(bfd_mach_6502, "6502", 16, true, 0);
*/
const bfd_arch_info_type bfd_mos65xx_arch =
  N(bfd_mach_65816, "65816", 24, true, 0);
