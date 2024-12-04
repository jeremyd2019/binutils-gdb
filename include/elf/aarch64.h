/* AArch64 ELF support for BFD.

   Copyright (C) 2009-2025 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _ELF_AARCH64_H
#define _ELF_AARCH64_H

#include "elf/reloc-macros.h"

/* Processor specific program header types.  */
#define PT_AARCH64_ARCHEXT	(PT_LOPROC + 0)

/* MTE memory tag segment type.  */
#define PT_AARCH64_MEMTAG_MTE     (PT_LOPROC + 0x2)

/* Name of the ELF section holding the attributes.  */
#define SEC_AARCH64_ATTRIBUTES	".ARM.attributes"
/* Additional section types.  */
/* Section holds attributes.  */
#define SHT_AARCH64_ATTRIBUTES	(SHT_LOPROC + 3)
/* Special aarch64-specific section for MTE support, as described in:
   https://github.com/ARM-software/abi-aa/blob/main/pauthabielf64/pauthabielf64.rst#section-types  */
#define SHT_AARCH64_AUTH_RELR   (SHT_LOPROC + 4)
/* Special aarch64-specific sections for MTE support, as described in:
   https://github.com/ARM-software/abi-aa/blob/main/memtagabielf64/memtagabielf64.rst#7section-types  */
#define SHT_AARCH64_MEMTAG_GLOBALS_STATIC  (SHT_LOPROC + 7)
#define SHT_AARCH64_MEMTAG_GLOBALS_DYNAMIC (SHT_LOPROC + 8)

/* AArch64-specific values for sh_flags.  */
#define SHF_ENTRYSECT		0x10000000   /* Section contains an
						entry point.  */
#define SHF_COMDEF		0x80000000   /* Section may be multiply defined
						in the input to a link step.  */
/* Processor specific dynamic array tags.  */
#define DT_AARCH64_BTI_PLT	(DT_LOPROC + 1)
#define DT_AARCH64_PAC_PLT	(DT_LOPROC + 3)
#define DT_AARCH64_VARIANT_PCS	(DT_LOPROC + 5)
#define DT_AARCH64_MEMTAG_MODE  (DT_LOPROC + 9)
#define DT_AARCH64_MEMTAG_STACK (DT_LOPROC + 12)

/* AArch64-specific values for st_other.  */
#define STO_AARCH64_VARIANT_PCS	0x80  /* Symbol may follow different call
					 convention from the base PCS.  */

/* Relocation types.  */

START_RELOC_NUMBERS (elf_aarch64_reloc_type)

/* Null relocations.  */
RELOC_NUMBER (R_AARCH64_NONE, 0) /* No reloc */

/* Basic data relocations.  */

/* .word:  (S+A) */
RELOC_NUMBER (R_AARCH64_P32_ABS32, 1)

/* .half: (S+A) */
RELOC_NUMBER (R_AARCH64_P32_ABS16, 2)

/* .word: (S+A-P) */
RELOC_NUMBER (R_AARCH64_P32_PREL32, 3)

/* .half:  (S+A-P) */
RELOC_NUMBER (R_AARCH64_P32_PREL16, 4)

/* Group relocations to create a 16, 32, 48 or 64 bit
   unsigned data or abs address inline.  */

/* MOV[ZK]:   ((S+A) >>  0) & 0xffff */
RELOC_NUMBER (R_AARCH64_P32_MOVW_UABS_G0, 5)

/* MOV[ZK]:   ((S+A) >>  0) & 0xffff */
RELOC_NUMBER (R_AARCH64_P32_MOVW_UABS_G0_NC, 6)

/* MOV[ZK]:   ((S+A) >> 16) & 0xffff */
RELOC_NUMBER (R_AARCH64_P32_MOVW_UABS_G1, 7)

/* Group relocations to create high part of a 16, 32, 48 or 64 bit
   signed data or abs address inline. Will change instruction
   to MOVN or MOVZ depending on sign of calculated value.  */

/* MOV[ZN]:   ((S+A) >>  0) & 0xffff */
RELOC_NUMBER (R_AARCH64_P32_MOVW_SABS_G0, 8)

/* Relocations to generate 19, 21 and 33 bit PC-relative load/store
   addresses: PG(x) is (x & ~0xfff).  */

/* LD-lit: ((S+A-P) >> 2) & 0x7ffff */
RELOC_NUMBER (R_AARCH64_P32_LD_PREL_LO19, 9)

/* ADR:    (S+A-P) & 0x1fffff */
RELOC_NUMBER (R_AARCH64_P32_ADR_PREL_LO21, 10)

/* ADRH:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff */
RELOC_NUMBER (R_AARCH64_P32_ADR_PREL_PG_HI21, 11)

/* ADD:    (S+A) & 0xfff */
RELOC_NUMBER (R_AARCH64_P32_ADD_ABS_LO12_NC, 12)

/* LD/ST8: (S+A) & 0xfff */
RELOC_NUMBER (R_AARCH64_P32_LDST8_ABS_LO12_NC, 13)

/* LD/ST16: (S+A) & 0xffe */
RELOC_NUMBER (R_AARCH64_P32_LDST16_ABS_LO12_NC, 14)

/* LD/ST32: (S+A) & 0xffc */
RELOC_NUMBER (R_AARCH64_P32_LDST32_ABS_LO12_NC, 15)

/* LD/ST64: (S+A) & 0xff8 */
RELOC_NUMBER (R_AARCH64_P32_LDST64_ABS_LO12_NC, 16)

/* LD/ST128: (S+A) & 0xff0 */
RELOC_NUMBER (R_AARCH64_P32_LDST128_ABS_LO12_NC, 17)

/* Relocations for control-flow instructions.  */

/* TBZ/NZ: ((S+A-P) >> 2) & 0x3fff.  */
RELOC_NUMBER (R_AARCH64_P32_TSTBR14, 18)

/* B.cond: ((S+A-P) >> 2) & 0x7ffff.  */
RELOC_NUMBER (R_AARCH64_P32_CONDBR19, 19)

/* B:      ((S+A-P) >> 2) & 0x3ffffff.  */
RELOC_NUMBER (R_AARCH64_P32_JUMP26, 20)

/* BL:     ((S+A-P) >> 2) & 0x3ffffff.  */
RELOC_NUMBER (R_AARCH64_P32_CALL26, 21)

/* Group relocations to create a 16 or 32 bit PC-relative offset inline.  */
RELOC_NUMBER (R_AARCH64_P32_MOVW_PREL_G0, 22)
RELOC_NUMBER (R_AARCH64_P32_MOVW_PREL_G0_NC, 23)
RELOC_NUMBER (R_AARCH64_P32_MOVW_PREL_G1, 24)

RELOC_NUMBER (R_AARCH64_P32_GOT_LD_PREL19, 25)
RELOC_NUMBER (R_AARCH64_P32_ADR_GOT_PAGE, 26)
RELOC_NUMBER (R_AARCH64_P32_LD32_GOT_LO12_NC, 27)
RELOC_NUMBER (R_AARCH64_P32_LD32_GOTPAGE_LO14, 28)

RELOC_NUMBER (R_AARCH64_P32_TLSGD_ADR_PREL21, 80)
RELOC_NUMBER (R_AARCH64_P32_TLSGD_ADR_PAGE21, 81)
RELOC_NUMBER (R_AARCH64_P32_TLSGD_ADD_LO12_NC, 82)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_ADR_PREL21, 83)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_ADR_PAGE21, 84)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_ADD_LO12_NC, 85)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_MOVW_DTPREL_G1, 87)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_MOVW_DTPREL_G0, 88)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_MOVW_DTPREL_G0_NC, 89)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_ADD_DTPREL_HI12, 90)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_ADD_DTPREL_LO12, 91)
RELOC_NUMBER (R_AARCH64_P32_TLSLD_ADD_DTPREL_LO12_NC, 92)
RELOC_NUMBER (R_AARCH64_P32_TLSIE_ADR_GOTTPREL_PAGE21, 103)
RELOC_NUMBER (R_AARCH64_P32_TLSIE_LD32_GOTTPREL_LO12_NC, 104)
RELOC_NUMBER (R_AARCH64_P32_TLSIE_LD_GOTTPREL_PREL19, 105)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_MOVW_TPREL_G1, 106)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_MOVW_TPREL_G0, 107)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_MOVW_TPREL_G0_NC, 108)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_ADD_TPREL_HI12, 109)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_ADD_TPREL_LO12, 110)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_ADD_TPREL_LO12_NC, 111)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST8_TPREL_LO12, 112)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST8_TPREL_LO12_NC, 113)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12, 114)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12_NC, 115)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST32_TPREL_LO12, 116)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST32_TPREL_LO12_NC, 117)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST64_TPREL_LO12, 118)
RELOC_NUMBER (R_AARCH64_P32_TLSLE_LDST64_TPREL_LO12_NC, 119)

RELOC_NUMBER (R_AARCH64_P32_TLSDESC_LD_PREL19, 122)
RELOC_NUMBER (R_AARCH64_P32_TLSDESC_ADR_PREL21, 123)
RELOC_NUMBER (R_AARCH64_P32_TLSDESC_ADR_PAGE21, 124)
RELOC_NUMBER (R_AARCH64_P32_TLSDESC_LD32_LO12_NC, 125)
RELOC_NUMBER (R_AARCH64_P32_TLSDESC_ADD_LO12_NC, 126)
RELOC_NUMBER (R_AARCH64_P32_TLSDESC_CALL, 127)

/* Dynamic relocations */

/* Copy symbol at runtime.  */
RELOC_NUMBER (R_AARCH64_P32_COPY, 180)

/* Create GOT entry.  */
RELOC_NUMBER (R_AARCH64_P32_GLOB_DAT, 181)

 /* Create PLT entry.  */
RELOC_NUMBER (R_AARCH64_P32_JUMP_SLOT, 182)

/* Adjust by program base.  */
RELOC_NUMBER (R_AARCH64_P32_RELATIVE, 183)
RELOC_NUMBER (R_AARCH64_P32_TLS_DTPMOD, 184)
RELOC_NUMBER (R_AARCH64_P32_TLS_DTPREL, 185)
RELOC_NUMBER (R_AARCH64_P32_TLS_TPREL, 186)
RELOC_NUMBER (R_AARCH64_P32_TLSDESC, 187)
RELOC_NUMBER (R_AARCH64_P32_IRELATIVE, 188)

RELOC_NUMBER (R_AARCH64_NULL, 256) /* No reloc */

/* Basic data relocations.  */

/* .xword: (S+A) */
RELOC_NUMBER (R_AARCH64_ABS64, 257)

/* .word:  (S+A) */
RELOC_NUMBER (R_AARCH64_ABS32, 258)

/* .half: (S+A) */
RELOC_NUMBER (R_AARCH64_ABS16, 259)

/* .xword: (S+A-P) */
RELOC_NUMBER (R_AARCH64_PREL64, 260)

/* .word: (S+A-P) */
RELOC_NUMBER (R_AARCH64_PREL32, 261)

/* .half:  (S+A-P) */
RELOC_NUMBER (R_AARCH64_PREL16, 262)

/* Group relocations to create a 16, 32, 48 or 64 bit
   unsigned data or abs address inline.  */

/* MOV[ZK]:   ((S+A) >>  0) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G0,		263)

/* MOV[ZK]:   ((S+A) >>  0) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G0_NC, 264)

/* MOV[ZK]:   ((S+A) >> 16) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G1, 265)

/* MOV[ZK]:   ((S+A) >> 16) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G1_NC, 266)

/* MOV[ZK]:   ((S+A) >> 32) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G2, 267)

/* MOV[ZK]:   ((S+A) >> 32) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G2_NC, 268)

/* MOV[ZK]:   ((S+A) >> 48) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_UABS_G3, 269)

/* Group relocations to create high part of a 16, 32, 48 or 64 bit
   signed data or abs address inline. Will change instruction
   to MOVN or MOVZ depending on sign of calculated value.  */

/* MOV[ZN]:   ((S+A) >>  0) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_SABS_G0, 270)

/* MOV[ZN]:   ((S+A) >> 16) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_SABS_G1, 271)

/* MOV[ZN]:   ((S+A) >> 32) & 0xffff */
RELOC_NUMBER (R_AARCH64_MOVW_SABS_G2, 272)

/* Relocations to generate 19, 21 and 33 bit PC-relative load/store
   addresses: PG(x) is (x & ~0xfff).  */

/* LD-lit: ((S+A-P) >> 2) & 0x7ffff */
RELOC_NUMBER (R_AARCH64_LD_PREL_LO19, 273)

/* ADR:    (S+A-P) & 0x1fffff */
RELOC_NUMBER (R_AARCH64_ADR_PREL_LO21, 274)

/* ADRH:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff */
RELOC_NUMBER (R_AARCH64_ADR_PREL_PG_HI21, 275)

/* ADRH:   ((PG(S+A)-PG(P)) >> 12) & 0x1fffff */
RELOC_NUMBER (R_AARCH64_ADR_PREL_PG_HI21_NC, 276)

/* ADD:    (S+A) & 0xfff */
RELOC_NUMBER (R_AARCH64_ADD_ABS_LO12_NC, 277)

/* LD/ST8: (S+A) & 0xfff */
RELOC_NUMBER (R_AARCH64_LDST8_ABS_LO12_NC, 278)

/* Relocations for control-flow instructions.  */

/* TBZ/NZ: ((S+A-P) >> 2) & 0x3fff.  */
RELOC_NUMBER (R_AARCH64_TSTBR14, 279)

/* B.cond: ((S+A-P) >> 2) & 0x7ffff.  */
RELOC_NUMBER (R_AARCH64_CONDBR19, 280)

/* 281 unused */

/* B:      ((S+A-P) >> 2) & 0x3ffffff.  */
RELOC_NUMBER (R_AARCH64_JUMP26, 282)

/* BL:     ((S+A-P) >> 2) & 0x3ffffff.  */
RELOC_NUMBER (R_AARCH64_CALL26, 283)

/* LD/ST16: (S+A) & 0xffe */
RELOC_NUMBER (R_AARCH64_LDST16_ABS_LO12_NC, 284)

/* LD/ST32: (S+A) & 0xffc */
RELOC_NUMBER (R_AARCH64_LDST32_ABS_LO12_NC, 285)

/* LD/ST64: (S+A) & 0xff8 */
RELOC_NUMBER (R_AARCH64_LDST64_ABS_LO12_NC, 286)

/* Group relocations to create a 16, 32, 48, or 64 bit PC-relative
   offset inline.  */

RELOC_NUMBER (R_AARCH64_MOVW_PREL_G0, 287)
RELOC_NUMBER (R_AARCH64_MOVW_PREL_G0_NC, 288)
RELOC_NUMBER (R_AARCH64_MOVW_PREL_G1, 289)
RELOC_NUMBER (R_AARCH64_MOVW_PREL_G1_NC, 290)
RELOC_NUMBER (R_AARCH64_MOVW_PREL_G2, 291)
RELOC_NUMBER (R_AARCH64_MOVW_PREL_G2_NC, 292)
RELOC_NUMBER (R_AARCH64_MOVW_PREL_G3, 293)

/* LD/ST128: (S+A) & 0xff0 */
RELOC_NUMBER (R_AARCH64_LDST128_ABS_LO12_NC, 299)

/* Group relocations to create a 16, 32, 48, or 64 bit GOT-relative
   offset inline.  */

RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G0, 300)
RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G0_NC, 301)
RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G1, 302)
RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G1_NC, 303)
RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G2, 304)
RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G2_NC, 305)
RELOC_NUMBER (R_AARCH64_MOVW_GOTOFF_G3, 306)

/* GOT-relative data relocations.  */

RELOC_NUMBER (R_AARCH64_GOTREL64, 307)
RELOC_NUMBER (R_AARCH64_GOTREL32, 308)

/* GOT-relative instruction relocations.  */

RELOC_NUMBER (R_AARCH64_GOT_LD_PREL19, 309)
RELOC_NUMBER (R_AARCH64_LD64_GOTOFF_LO15, 310)
RELOC_NUMBER (R_AARCH64_ADR_GOT_PAGE, 311)
RELOC_NUMBER (R_AARCH64_LD64_GOT_LO12_NC, 312)
RELOC_NUMBER (R_AARCH64_LD64_GOTPAGE_LO15, 313)

/* General Dynamic TLS relocations.  */

RELOC_NUMBER (R_AARCH64_TLSGD_ADR_PREL21, 512)
RELOC_NUMBER (R_AARCH64_TLSGD_ADR_PAGE21, 513)
RELOC_NUMBER (R_AARCH64_TLSGD_ADD_LO12_NC, 514)
RELOC_NUMBER (R_AARCH64_TLSGD_MOVW_G1, 515)
RELOC_NUMBER (R_AARCH64_TLSGD_MOVW_G0_NC, 516)

/* Local Dynamic TLS relocations.  */

RELOC_NUMBER (R_AARCH64_TLSLD_ADR_PREL21, 517)
RELOC_NUMBER (R_AARCH64_TLSLD_ADR_PAGE21, 518)
RELOC_NUMBER (R_AARCH64_TLSLD_ADD_LO12_NC, 519)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_G1, 520)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_G0_NC, 521)
RELOC_NUMBER (R_AARCH64_TLSLD_LD_PREL19, 522)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_DTPREL_G2, 523)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_DTPREL_G1, 524)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_DTPREL_G1_NC, 525)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_DTPREL_G0, 526)
RELOC_NUMBER (R_AARCH64_TLSLD_MOVW_DTPREL_G0_NC, 527)
RELOC_NUMBER (R_AARCH64_TLSLD_ADD_DTPREL_HI12, 528)
RELOC_NUMBER (R_AARCH64_TLSLD_ADD_DTPREL_LO12, 529)
RELOC_NUMBER (R_AARCH64_TLSLD_ADD_DTPREL_LO12_NC, 530)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST8_DTPREL_LO12, 531)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC, 532)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST16_DTPREL_LO12, 533)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC, 534)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST32_DTPREL_LO12, 535)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC, 536)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST64_DTPREL_LO12, 537)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC, 538)

/* Initial Exec TLS relocations.  */

RELOC_NUMBER (R_AARCH64_TLSIE_MOVW_GOTTPREL_G1, 539)
RELOC_NUMBER (R_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC, 540)
RELOC_NUMBER (R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21, 541)
RELOC_NUMBER (R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC, 542)
RELOC_NUMBER (R_AARCH64_TLSIE_LD_GOTTPREL_PREL19, 543)

/* Local Exec TLS relocations.  */

RELOC_NUMBER (R_AARCH64_TLSLE_MOVW_TPREL_G2, 544)
RELOC_NUMBER (R_AARCH64_TLSLE_MOVW_TPREL_G1, 545)
RELOC_NUMBER (R_AARCH64_TLSLE_MOVW_TPREL_G1_NC, 546)
RELOC_NUMBER (R_AARCH64_TLSLE_MOVW_TPREL_G0, 547)
RELOC_NUMBER (R_AARCH64_TLSLE_MOVW_TPREL_G0_NC, 548)
RELOC_NUMBER (R_AARCH64_TLSLE_ADD_TPREL_HI12, 549)
RELOC_NUMBER (R_AARCH64_TLSLE_ADD_TPREL_LO12, 550)
RELOC_NUMBER (R_AARCH64_TLSLE_ADD_TPREL_LO12_NC, 551)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST8_TPREL_LO12, 552)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC, 553)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST16_TPREL_LO12, 554)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC, 555)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST32_TPREL_LO12, 556)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC, 557)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST64_TPREL_LO12, 558)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC, 559)

/* TLS descriptor relocations.  */

RELOC_NUMBER (R_AARCH64_TLSDESC_LD_PREL19, 560)
RELOC_NUMBER (R_AARCH64_TLSDESC_ADR_PREL21, 561)
RELOC_NUMBER (R_AARCH64_TLSDESC_ADR_PAGE21, 562)
RELOC_NUMBER (R_AARCH64_TLSDESC_LD64_LO12, 563)
RELOC_NUMBER (R_AARCH64_TLSDESC_ADD_LO12, 564)
RELOC_NUMBER (R_AARCH64_TLSDESC_OFF_G1, 565)
RELOC_NUMBER (R_AARCH64_TLSDESC_OFF_G0_NC, 566)
RELOC_NUMBER (R_AARCH64_TLSDESC_LDR, 567)
RELOC_NUMBER (R_AARCH64_TLSDESC_ADD, 568)
RELOC_NUMBER (R_AARCH64_TLSDESC_CALL, 569)

RELOC_NUMBER (R_AARCH64_TLSLE_LDST128_TPREL_LO12, 570)
RELOC_NUMBER (R_AARCH64_TLSLE_LDST128_TPREL_LO12_NC, 571)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST128_DTPREL_LO12, 572)
RELOC_NUMBER (R_AARCH64_TLSLD_LDST128_DTPREL_LO12_NC, 573)

/* Dynamic relocations */

/* Copy symbol at runtime.  */
RELOC_NUMBER (R_AARCH64_COPY, 1024)

/* Create GOT entry.  */
RELOC_NUMBER (R_AARCH64_GLOB_DAT, 1025)

 /* Create PLT entry.  */
RELOC_NUMBER (R_AARCH64_JUMP_SLOT, 1026)

/* Adjust by program base.  */
RELOC_NUMBER (R_AARCH64_RELATIVE, 1027)
RELOC_NUMBER (R_AARCH64_TLS_DTPMOD64, 1028)
RELOC_NUMBER (R_AARCH64_TLS_DTPREL64, 1029)
RELOC_NUMBER (R_AARCH64_TLS_TPREL64, 1030)
/* Aliasing relocs are guarded by RELOC_MACROS_GEN_FUNC
   so that readelf.c won't generate duplicated case
   statements.  */
#ifndef RELOC_MACROS_GEN_FUNC
RELOC_NUMBER (R_AARCH64_TLS_DTPMOD, 1028)
RELOC_NUMBER (R_AARCH64_TLS_DTPREL, 1029)
RELOC_NUMBER (R_AARCH64_TLS_TPREL, 1030)
#endif
RELOC_NUMBER (R_AARCH64_TLSDESC, 1031)
RELOC_NUMBER (R_AARCH64_IRELATIVE, 1032)

END_RELOC_NUMBERS (R_AARCH64_end)

#endif /* _ELF_AARCH64_H */
