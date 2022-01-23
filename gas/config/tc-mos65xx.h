#ifndef TC_MOS65XX
#define TC_MOS65XX

#include <opcode/mos65xx.h>

#define TARGET_ARCH	bfd_arch_mos65xx
#define BFD_ARCH	TARGET_ARCH
#define TARGET_FORMAT	"elf32-mos65xx"

#define TARGET_BYTES_BIG_ENDIAN 0

#define md_number_to_chars number_to_chars_littleendian

extern const char EXP_CHARS[];
extern const char FLT_CHARS[];

#define LITERAL_PREFIXPERCENT_BIN
#define LITERAL_PREFIXDOLLAR_HEX
#define LEX_DOLLAR 0

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

/*
 *  The grammar of typical 65816 assembly is actually
 *  ambiguous. This is because some addressing modes
 *  look the same but differ in operand size. Operand
 *  types defined so that parser can first determine form,
 *  then size after more context is gathered.
 */
#define MOS65XX_OPERAND_IND_IDX 	MOS65XX_ADDRMODE_IND_IDX
#define MOS65XX_OPERAND_STK_REL 	MOS65XX_ADDRMODE_STK_REL
#define MOS65XX_OPERAND_PGE 		MOS65XX_ADDRMODE_PGE
#define MOS65XX_OPERAND_IND_LNG_IDY 	MOS65XX_ADDRMODE_IND_LNG_IDY
#define MOS65XX_OPERAND_IMM 		MOS65XX_ADDRMODE_IMM
#define MOS65XX_OPERAND_IND_IDY 	MOS65XX_ADDRMODE_IND_IDY
#define MOS65XX_OPERAND_IND 		MOS65XX_ADDRMODE_IND
#define MOS65XX_OPERAND_STK_IND_IDY 	MOS65XX_ADDRMODE_STK_IND_IDY
#define MOS65XX_OPERAND_IDX 		MOS65XX_ADDRMODE_IDX
#define MOS65XX_OPERAND_IND_LNG 	MOS65XX_ADDRMODE_IND_LNG
#define MOS65XX_OPERAND_IDY 		MOS65XX_ADDRMODE_IDY
#define MOS65XX_OPERAND_BLK 		MOS65XX_ADDRMODE_BLK
#define MOS65XX_OPERAND_IMPLIED 	MOS65XX_ADDRMODE_IMPLIED
#define MOS65XX_OPERAND_ACC 		MOS65XX_ADDRMODE_ACC
#define MOS65XX_OPERAND_INVALID 	MOS65XX_ADDRMODE_INVALID

#endif
