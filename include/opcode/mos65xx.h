#ifndef OPCODE_MOS65XX_H
#define OPCODE_MOS65XX_H

#include <stdint.h>

/*
 *  Addressing modes specifically defined so that
 *  a lot of opcodes can be found via (base + mode),
 *  i.e., for LSR, base = 0x42 and, for acceptable modes,
 *  besides Accumulator, opcode = (base + mode).
 */
#define MOS65XX_ADDRMODE_IND_IDX 		0x0
#define MOS65XX_ADDRMODE_STK_REL 		0x2
#define MOS65XX_ADDRMODE_PGE 			0x4
#define MOS65XX_ADDRMODE_IND_LNG 		0x6
#define MOS65XX_ADDRMODE_IMM 			0x8
#define MOS65XX_ADDRMODE_ABS 			0xc
#define MOS65XX_ADDRMODE_LNG 			0xe
#define MOS65XX_ADDRMODE_IND_IDY 		0x10
#define MOS65XX_ADDRMODE_IND 			0x11
#define MOS65XX_ADDRMODE_STK_IND_IDY 		0x12
#define MOS65XX_ADDRMODE_IDX 			0x14
#define MOS65XX_ADDRMODE_IND_LNG_IDY 		0x16
#define MOS65XX_ADDRMODE_ABS_IDY 		0x18
#define MOS65XX_ADDRMODE_ABS_IDX 		0x1c
#define MOS65XX_ADDRMODE_ABS_LNG_IDX 		0x1e
#define MOS65XX_ADDRMODE_ACC 			0x20
#define MOS65XX_ADDRMODE_BLK 			0x21
#define MOS65XX_ADDRMODE_INVALID 		0xff
#define MOS65XX_ADDRMODE_MAX_VALUE 		(+0xffffff)
#define MOS65XX_ADDRMODE_MIN_VALUE 		(-0x800000)

#define MOS65XX_OPCODE_BASE(fl) 		((fl) & 0xff)
#define MOS65XX_OPCODE_TYPE(fl) 		((fl) >> 8 & 0xffffff)
#define MOS65XX_OPCODE_ALUMEM 			0x0100
#define MOS65XX_OPCODE_BITOPS 			0x0200
#define MOS65XX_OPCODE_LDX 			0x0300
#define MOS65XX_OPCODE_LDY 			0x0400
#define MOS65XX_OPCODE_NOARGS			0x0500

#define MOS65XX_OPCODE_SPECIAL_BEGIN 		0x0600
	#define MOS65XX_OPCODE_SPECIAL_REP 	0x0600
	#define MOS65XX_OPCODE_SPECIAL_PEA 	0x0700
	#define MOS65XX_OPCODE_SPECIAL_PEI 	0x0800
	#define MOS65XX_OPCODE_SPECIAL_PER 	0x0900
	#define MOS65XX_OPCODE_SPECIAL_COP 	0x0a00
	#define MOS65XX_OPCODE_SPECIAL_MVNP 	0x0b00
	#define MOS65XX_OPCODE_SPECIAL_TRSB 	0x0c00
#define MOS65XX_OPCODE_SPECIAL_END 		0x0c00

#define MOS65XX_OPCODE_BRANCH 			0x0d00
#define MOS65XX_OPCODE_JUMP 			0x0e00

#define MOS65XX_BANK_MAX 			0xff

#define MOS65XX_ADC_BASE 			0x61
#define MOS65XX_AND_BASE 			0x21
#define MOS65XX_ASL_BASE 			0x02
#define MOS65XX_BCC_BASE 			0x90
#define MOS65XX_BCS_BASE 			0xb0
#define MOS65XX_BEQ_BASE 			0xf0
#define MOS65XX_BIT_BASE 			0x20
#define MOS65XX_BMI_BASE 			0x30
#define MOS65XX_BNE_BASE 			0xd0
#define MOS65XX_BPL_BASE 			0x10
#define MOS65XX_BRA_BASE 			0x80
#define MOS65XX_BRK_BASE 			0x00
#define MOS65XX_BRL_BASE 			0x82
#define MOS65XX_BVC_BASE 			0x50
#define MOS65XX_BVS_BASE 			0x70
#define MOS65XX_CLC_BASE 			0x18
#define MOS65XX_CLD_BASE 			0xd8
#define MOS65XX_CLI_BASE 			0x58
#define MOS65XX_CLV_BASE 			0xb8
#define MOS65XX_CMP_BASE 			0xc1
#define MOS65XX_COP_BASE 			0x02
#define MOS65XX_CPX_BASE 			0xe0
#define MOS65XX_CPY_BASE 			0xc0
#define MOS65XX_DEC_BASE 			0xc2
#define MOS65XX_DEX_BASE 			0xca
#define MOS65XX_DEY_BASE 			0x88
#define MOS65XX_EOR_BASE 			0x41
#define MOS65XX_INC_BASE 			0xe2
#define MOS65XX_INX_BASE 			0xe8
#define MOS65XX_INY_BASE 			0xc8
#define MOS65XX_JMP_BASE 			0x4c
#define MOS65XX_JSR_BASE 			0x20
#define MOS65XX_LDA_BASE 			0xa1
#define MOS65XX_LDX_BASE 			0xa2
#define MOS65XX_LDY_BASE 			0xa0
#define MOS65XX_LSR_BASE 			0x42
#define MOS65XX_MVN_BASE 			0x54
#define MOS65XX_MVP_BASE 			0x44
#define MOS65XX_NOP_BASE 			0xea
#define MOS65XX_ORA_BASE 			0x01
#define MOS65XX_PEA_BASE 			0xf4
#define MOS65XX_PEI_BASE 			0xd4
#define MOS65XX_PER_BASE 			0x62
#define MOS65XX_PHA_BASE 			0x48
#define MOS65XX_PHB_BASE 			0x8b
#define MOS65XX_PHD_BASE 			0x0b
#define MOS65XX_PHK_BASE 			0x4b
#define MOS65XX_PHP_BASE 			0x08
#define MOS65XX_PHX_BASE 			0xda
#define MOS65XX_PHY_BASE 			0x5a
#define MOS65XX_PLA_BASE 			0x68
#define MOS65XX_PLB_BASE 			0xab
#define MOS65XX_PLD_BASE 			0x2b
#define MOS65XX_PLP_BASE 			0x28
#define MOS65XX_PLX_BASE 			0xfa
#define MOS65XX_PLY_BASE 			0x7a
#define MOS65XX_REP_BASE 			0xc2
#define MOS65XX_ROL_BASE 			0x22
#define MOS65XX_ROR_BASE 			0x62
#define MOS65XX_RTI_BASE 			0x40
#define MOS65XX_RTL_BASE 			0x6b
#define MOS65XX_RTS_BASE 			0x60
#define MOS65XX_SBC_BASE 			0xe1
#define MOS65XX_SEC_BASE 			0x38
#define MOS65XX_SED_BASE 			0xf8
#define MOS65XX_SEI_BASE 			0x78
#define MOS65XX_SEP_BASE 			0xe2
#define MOS65XX_STA_BASE 			0x81
#define MOS65XX_STP_BASE 			0xdb
#define MOS65XX_STX_BASE 			0x82
#define MOS65XX_STY_BASE 			0x80
#define MOS65XX_STZ_BASE 			0x60
#define MOS65XX_TAX_BASE 			0xaa
#define MOS65XX_TAY_BASE 			0xa8
#define MOS65XX_TCD_BASE 			0x5b
#define MOS65XX_TCS_BASE 			0x1b
#define MOS65XX_TDC_BASE 			0x7b
#define MOS65XX_TRB_BASE 			0x10
#define MOS65XX_TSB_BASE 			0x00
#define MOS65XX_TSC_BASE 			0x3b
#define MOS65XX_TSX_BASE 			0xba
#define MOS65XX_TXA_BASE 			0x8a
#define MOS65XX_TXS_BASE 			0x9a
#define MOS65XX_TXY_BASE 			0x9b
#define MOS65XX_TYA_BASE 			0x98
#define MOS65XX_TYX_BASE 			0xbb
#define MOS65XX_WAI_BASE 			0xcb
#define MOS65XX_WDM_BASE 			0x42
#define MOS65XX_XBA_BASE 			0xeb
#define MOS65XX_XCE_BASE 			0xfb

struct mos65xx_op {
  const char *name;
  uint32_t flags;
};

struct mos65xx_addrmode {
  uint32_t typ;
  uint16_t val1;
  uint16_t val2;
};

struct mos65xx_op *mos65xx_opcode_lookup(const char *nmemonic);

#endif
