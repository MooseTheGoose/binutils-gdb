
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
  { 
    BFD_RELOC_NONE,
    HOWTO(R_MOS65XX_NONE, /* Type */
  	0, /* rightshift */
  	3, /* size (0 = byte, 1 = short, 3 = <none>, 5 = mos65816 long) */
  	0, /* bitsize */
  	false, /* pc_relative */
  	0, /* bitpos */
  	complain_overflow_dont, /* complain_on_overflow */
  	bfd_elf_generic_reloc, /* special_function */
  	"R_MOS65XX_NONE", /* name */
  	false, /* partial_inplace */
  	0, /* src_mask */
  	0, /* dst_mask */
  	false)  /* pcrel_offset */
  },
  { 
    BFD_RELOC_8,
    HOWTO(R_MOS65XX_8,
 	0,
 	1,
 	8,
 	false,
 	0,
 	complain_overflow_bitfield,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_8",
 	false,
 	0,
 	0xff,
 	false)
  },
  {
    BFD_RELOC_16,
    HOWTO(R_MOS65XX_16,
 	0,
 	2,
 	16,
 	false,
 	0,
 	complain_overflow_bitfield,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_16",
 	false,
 	0,
 	0xffff,
 	false)
  },
  {
    BFD_RELOC_24,
    HOWTO(R_MOS65XX_24,
 	0,
 	5,
 	24,
 	false,
 	0,
 	complain_overflow_bitfield,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_24",
 	false,
 	0,
 	0xffffff,
 	false)
  },
  {
    BFD_RELOC_8_PCREL,
    HOWTO(R_MOS65XX_8_PCREL,
 	0,
 	0,
 	8,
 	true,
 	0,
 	complain_overflow_signed,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_8_PCREL",
 	false,
 	0,
 	0xff,
 	true)
  },
  {
    BFD_RELOC_16_PCREL,
    HOWTO(R_MOS65XX_16_PCREL,
 	0,
 	1,
 	16,
 	true,
 	0,
 	complain_overflow_signed,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_16_PCREL",
 	false,
 	0,
 	0xffff,
 	true)
  },
  {
    BFD_RELOC_MOS65XX_DPAGE,
    HOWTO(R_MOS65XX_DPAGE,
 	8,
 	1,
 	16,
 	false,
 	0,
 	complain_overflow_dont,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_DPAGE",
 	false,
 	0,
 	0xffff,
 	false)
  },
  {
    BFD_RELOC_MOS65XX_BANK,
    HOWTO(R_MOS65XX_BANK,
 	16,
 	0,
 	8,
 	false,
 	0,
 	complain_overflow_dont,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_BANK",
 	false,
 	0,
 	0xff,
 	false)
  },
  {
    BFD_RELOC_8_FFnn,
    HOWTO(R_MOS65XX_PAGEOFS,
 	0,
 	0,
 	8,
 	false,
 	0,
 	complain_overflow_dont,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_PAGEOFS",
 	false,
 	0,
 	0xff,
 	false)
  },
  {
    BFD_RELOC_MOS65XX_STK_REL,
    HOWTO(R_MOS65XX_STK_REL,
 	0,
 	0,
 	8,
 	false,
 	0,
 	complain_overflow_unsigned,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_STK_REL",
 	false,
 	0,
 	0xff,
 	false)
  },
  {
    BFD_RELOC_MOS65XX_ABS,
    HOWTO(R_MOS65XX_ABS,
 	0,
 	0,
 	16,
 	false,
 	0,
 	complain_overflow_dont,
 	bfd_elf_generic_reloc,
 	"R_MOS65XX_ABS",
 	false,
 	0,
 	0xff,
 	false)
  }
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
