#ifndef TC_MOS65XX
#define TC_MOS65XX

#define TARGET_ARCH	bfd_arch_mos65xx
#define BFD_ARCH	TARGET_ARCH
#define TARGET_FORMAT	"elf32-mos65xx"

#define TARGET_BYTES_BIG_ENDIAN 0

#define md_number_to_chars number_to_chars_littleendian

extern const char EXP_CHARS[];
extern const char FLT_CHARS[];

extern struct option md_longopts[];
extern const char * md_shortopts;

int md_parse_option(int optc, const char *optarg);
void md_show_usage(FILE * f);

void md_assemble(char *input);

int md_parse_option(int, const char*);
void md_show_usage(FILE *f);

#define WORKING_DOT_WORD 	1

#define md_convert_frag(b,s,f) as_fatal("convert_frag called")
#define md_operand(s) as_fatal("Unrecognized expression")
#define md_estimate_size_before_relax(f, s) \
  (as_fatal("estimate_size_before_relax called"), 1)

#endif
