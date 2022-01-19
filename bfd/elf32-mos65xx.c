
#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"

#include "elf/mos65xx.h"

struct mos65xx_reloc_map_entry {
  bfd_reloc_code_real_type typ;
  reloc_howto_type howto;
};

static const struct mos65xx_reloc_map_entry mos65xx_reloc_map[R_MOS65XX_MAX] =
{
  { BFD_RELOC_NONE,
    HOWTO(R_MOS65XX_NONE,
  	0,
  	3,
  	0,
  	false,
  	0,
  	complain_overflow_dont,
  	bfd_elf_generic_reloc,
  	"R_MOS65XX_NONE",
  	false,
  	0,
  	0,
  	false) }
};

static reloc_howto_type *
mos65xx_elf_reloc_type_lookup (bfd *abfd,
			    bfd_reloc_code_real_type code)
{
  unsigned int i;
  for(i = 0; i < R_MOS65XX_MAX; i++)
    if(code == mos65xx_reloc_map[i].typ)
      return &mos65xx_reloc_map[i].howto;

  _bfd_error_handler(_("%pB: unsupported relocation type %#x"), abfd, code);
  return NULL;
}

static reloc_howto_type *
mos65xx_elf_reloc_name_lookup(bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  int i;
  for(i = 0; i < R_MOS65XX_MAX; i++)
    if(mos65xx_reloc_map[i].howto.name != 0 
	&& strcasecmp(mos65xx_reloc_map[i].howto.name, r_name) == 0)
	return &mos65xx_reloc_map[i].howto;

  _bfd_error_handler(_("%pB: unsupported relocation name %s"), abfd, r_name);
  return NULL;
}

/*
static reloc_howto_type *
mos65xx_elf_rtype_to_howto(bfd *abfd, unsigned int r_type)
{
  int i;
  for(i = 0; i < R_MOS65XX_MAX; i++)
    if(mos65xx_reloc_map[i].howto.type == r_type)
      return &mos65xx_reloc_map[i].howto;

  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
		      abfd, r_type);

  return NULL;
}
*/

#define TARGET_LITTLE_SYM		mos65xx_elf32_vec
#define TARGET_LITTLE_NAME		"elf32-mos65xx"
#define ELF_ARCH			bfd_arch_mos65xx
#define ELF_MACHINE_CODE		EM_MOS65XX
#define ELF_MAXPAGESIZE			0x1

#define bfd_elf32_bfd_reloc_type_lookup 	mos65xx_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup 	mos65xx_elf_reloc_name_lookup
#define bfd_elf32_bfd_rtype_to_howto		mos65xx_elf_rtype_to_howto

#include "elf32-target.h"
