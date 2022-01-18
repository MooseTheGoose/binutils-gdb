
#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"

#include "elf/mos65xx.h"

#if 0

struct mos65xx_reloc_map_entry
{
  bfd_reloc_code_real_type bfd_reloc_enum;
  unsigned short mos65xx_reloc_type;
};

static const struct mos65xx_reloc_map_entry mos65xx_reloc_map[R_MOS65XX_MAX] =
{
  {BFD_RELOC_NONE,	R_MOS65XX_NONE}
};


static reloc_howto_type mos65xx_elf_howto_table[] =
{
  HOWTO(R_MOS65XX_NONE,
  	0,
  	3,
  	0,
  	false,
  	0,
  	complain_overflow_dont,
  	bfd_elf_generic_reloc,
  	"R_MOS65xx_NONE",
  	false,
  	0,
  	0,
  	false)
};

static reloc_howto_type *
elf_mos65xx_reloc_type_lookup (bfd *abfd,
			    bfd_reloc_code_real_type code)
{
  unsigned int i;
  for(i = 0; i < R_MOS65XX_MAX; i++)
    if(code == mos65xx_reloc_map[i].bfd_reloc_enum)
      return &mos65xx_elf_howto_table[mos65xx_reloc_map[i].mos65xx_reloc_type];

  _bfd_error_handler(_("%pB: unsupported relocation type %#x"), abfd, code);
  return NULL;
}

static reloc_howto_type *
mos65xx_reloc_name_lookup(bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  int table_size = sizeof(mos65xx_elf_howto_table) / sizeof(mos65xx_elf_howto_table[0]);
  int i;
  for(i = 0; i < table_size; i++)
    if(mos65xx_elf_howto_table[i].name != 0 
	&& strcasecmp(mos65xx_elf_howto_table[i].name, r_name) == 0)
	return &mos65xx_elf_howto_table[i];

  _bfd_error_handler(_("%pB: unsupported relocation name %s"), abfd, r_name);
  return NULL;
}

static reloc_howto_type *
mos65xx_rtype_to_howto(bfd *abfd, unsigned int r_type)
{
  int table_size = sizeof(mos65xx_elf_howto_table) / sizeof(mos65xx_elf_howto_table[0]);
  int i;
  for(i = 0; i < table_size; i++)
    if(mos65xx_elf_howto_table[i].type == r_type)
      return &mos65xx_elf_howto_table[i];

  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
		      abfd, r_type);

  return NULL;
}

#endif

#define TARGET_LITTLE_SYM		mos65xx_elf32_vec
#define TARGET_LITTLE_NAME		"elf-mos65xx"
#define ELF_ARCH			bfd_arch_mos65xx
#define ELF_MACHINE_CODE		EM_MOS65XX
#define ELF_MAXPAGESIZE			0x1

