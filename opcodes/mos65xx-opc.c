
#include <opcode/mos65xx.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

static const struct mos65xx_op mos65xx_opcodes[] = 
{
  { "adc", MOS65XX_OPCREATE(ADC) },
  { "and", MOS65XX_OPCREATE(AND) },
  { "asl", MOS65XX_OPCREATE(ASL) },
  { "bcc", MOS65XX_OPCREATE(BCC) },
  { "bcs", MOS65XX_OPCREATE(BCS) },
  { "beq", MOS65XX_OPCREATE(BEQ) },
  { "bit", MOS65XX_OPCREATE(BIT) },
  { "bmi", MOS65XX_OPCREATE(BMI) },
  { "bne", MOS65XX_OPCREATE(BNE) },
  { "bpl", MOS65XX_OPCREATE(BPL) },
  { "bra", MOS65XX_OPCREATE(BRA) },
  { "brk", MOS65XX_OPCREATE(BRK) },
  { "brl", MOS65XX_OPCREATE(BRL) },
  { "bvc", MOS65XX_OPCREATE(BVC) },
  { "bvs", MOS65XX_OPCREATE(BVS) },
  { "clc", MOS65XX_OPCREATE(CLC) },
  { "cld", MOS65XX_OPCREATE(CLD) },
  { "cli", MOS65XX_OPCREATE(CLI) },
  { "clv", MOS65XX_OPCREATE(CLV) },
  { "cmp", MOS65XX_OPCREATE(CMP) },
  { "cop", MOS65XX_OPCREATE(COP) },
  { "cpx", MOS65XX_OPCREATE(CPX) },
  { "cpy", MOS65XX_OPCREATE(CPY) },
  { "dec", MOS65XX_OPCREATE(DEC) },
  { "dex", MOS65XX_OPCREATE(DEX) },
  { "dey", MOS65XX_OPCREATE(DEY) },
  { "eor", MOS65XX_OPCREATE(EOR) },
  { "inc", MOS65XX_OPCREATE(INC) },
  { "inx", MOS65XX_OPCREATE(INX) },
  { "iny", MOS65XX_OPCREATE(INY) },
  { "jmp", MOS65XX_OPCREATE(JMP) },
  { "jsr", MOS65XX_OPCREATE(JSR) },
  { "lda", MOS65XX_OPCREATE(LDA) },
  { "ldx", MOS65XX_OPCREATE(LDX) },
  { "ldy", MOS65XX_OPCREATE(LDY) },
  { "lsr", MOS65XX_OPCREATE(LSR) },
  { "mvn", MOS65XX_OPCREATE(MVN) },
  { "mvp", MOS65XX_OPCREATE(MVP) },
  { "nop", MOS65XX_OPCREATE(NOP) },
  { "ora", MOS65XX_OPCREATE(ORA) },
  { "pea", MOS65XX_OPCREATE(PEA) },
  { "pei", MOS65XX_OPCREATE(PEI) },
  { "per", MOS65XX_OPCREATE(PER) },
  { "pha", MOS65XX_OPCREATE(PHA) },
  { "phb", MOS65XX_OPCREATE(PHB) },
  { "phd", MOS65XX_OPCREATE(PHD) },
  { "phk", MOS65XX_OPCREATE(PHK) },
  { "php", MOS65XX_OPCREATE(PHP) },
  { "phx", MOS65XX_OPCREATE(PHX) },
  { "phy", MOS65XX_OPCREATE(PHY) },
  { "pla", MOS65XX_OPCREATE(PLA) },
  { "plb", MOS65XX_OPCREATE(PLB) },
  { "pld", MOS65XX_OPCREATE(PLD) },
  { "plp", MOS65XX_OPCREATE(PLP) },
  { "plx", MOS65XX_OPCREATE(PLX) },
  { "ply", MOS65XX_OPCREATE(PLY) },
  { "rep", MOS65XX_OPCREATE(REP) },
  { "rol", MOS65XX_OPCREATE(ROL) },
  { "ror", MOS65XX_OPCREATE(ROR) },
  { "rti", MOS65XX_OPCREATE(RTI) },
  { "rtl", MOS65XX_OPCREATE(RTL) },
  { "rts", MOS65XX_OPCREATE(RTS) },
  { "sbc", MOS65XX_OPCREATE(SBC) },
  { "sec", MOS65XX_OPCREATE(SEC) },
  { "sed", MOS65XX_OPCREATE(SED) },
  { "sei", MOS65XX_OPCREATE(SEI) },
  { "sep", MOS65XX_OPCREATE(SEP) },
  { "sta", MOS65XX_OPCREATE(STA) },
  { "stp", MOS65XX_OPCREATE(STP) },
  { "stx", MOS65XX_OPCREATE(STX) },
  { "sty", MOS65XX_OPCREATE(STY) },
  { "stz", MOS65XX_OPCREATE(STZ) },
  { "tax", MOS65XX_OPCREATE(TAX) },
  { "tay", MOS65XX_OPCREATE(TAY) },
  { "tcd", MOS65XX_OPCREATE(TCD) },
  { "tcs", MOS65XX_OPCREATE(TCS) },
  { "tdc", MOS65XX_OPCREATE(TDC) },
  { "trb", MOS65XX_OPCREATE(TRB) },
  { "tsb", MOS65XX_OPCREATE(TSB) },
  { "tsc", MOS65XX_OPCREATE(TSC) },
  { "tsx", MOS65XX_OPCREATE(TSX) },
  { "txa", MOS65XX_OPCREATE(TXA) },
  { "txs", MOS65XX_OPCREATE(TSX) },
  { "txy", MOS65XX_OPCREATE(TXY) },
  { "tya", MOS65XX_OPCREATE(TYA) },
  { "tyx", MOS65XX_OPCREATE(TYX) },
  { "wai", MOS65XX_OPCREATE(WAI) },
  { "wdm", MOS65XX_OPCREATE(WDM)},
  { "xba", MOS65XX_OPCREATE(XBA) },
  { "xce", MOS65XX_OPCREATE(XCE) } 
};
#define MOS65XX_OPCODES_SIZE (sizeof(mos65xx_opcodes) / sizeof(mos65xx_opcodes[0]))

static int mos65xx_op_compare(const struct mos65xx_op *left, const struct mos65xx_op *right)
{
  return strcasecmp(left->name, right->name); 
}

struct mos65xx_op *mos65xx_opcode_lookup(const char *nmemonic)
{
  struct mos65xx_op dummy = {nmemonic, 0};
  return bsearch(&dummy, mos65xx_opcodes, 
    sizeof(mos65xx_opcodes) / sizeof(mos65xx_opcodes[0]),
    sizeof(mos65xx_opcodes[0]),
    (int(*)(const void *, const void *))mos65xx_op_compare);  
}

