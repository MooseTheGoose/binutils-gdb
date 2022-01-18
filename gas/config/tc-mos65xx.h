#ifndef TC_Z80
#define TC_Z80

#define TARGET_ARCH	bfd_arch_z80
#define BFD_ARCH	TARGET_ARCH
#define TARGET_FORMAT	"elf32-mos65xx"

#define TARGET_BYTES_BIG_ENDIAN 0

#define ONLY_STANDARD_ESCAPES

#endif
