/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Disassembler interface for targets using CGEN. -*- C -*-
   CGEN: Cpu tools GENerator

   THIS FILE IS MACHINE GENERATED WITH CGEN.
   - the resultant file is machine generated, cgen-dis.in isn't

   Copyright (C) 1996-2025 Free Software Foundation, Inc.

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/* ??? Eventually more and more of this stuff can go to cpu-independent files.
   Keep that in mind.  */

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "disassemble.h"
#include "bfd.h"
#include "symcat.h"
#include "libiberty.h"
#include "m32r-desc.h"
#include "m32r-opc.h"
#include "opintl.h"

/* Default text to print if an instruction isn't recognized.  */
#define UNKNOWN_INSN_MSG _("*unknown*")

static void print_normal
  (CGEN_CPU_DESC, void *, long, unsigned int, bfd_vma, int);
static void print_address
  (CGEN_CPU_DESC, void *, bfd_vma, unsigned int, bfd_vma, int) ATTRIBUTE_UNUSED;
static void print_keyword
  (CGEN_CPU_DESC, void *, CGEN_KEYWORD *, long, unsigned int) ATTRIBUTE_UNUSED;
static void print_insn_normal
  (CGEN_CPU_DESC, void *, const CGEN_INSN *, CGEN_FIELDS *, bfd_vma, int);
static int print_insn
  (CGEN_CPU_DESC, bfd_vma,  disassemble_info *, bfd_byte *, unsigned);
static int default_print_insn
  (CGEN_CPU_DESC, bfd_vma, disassemble_info *) ATTRIBUTE_UNUSED;
static int read_insn
  (CGEN_CPU_DESC, bfd_vma, disassemble_info *, bfd_byte *, int, CGEN_EXTRACT_INFO *,
   unsigned long *);

/* -- disassembler routines inserted here.  */

/* -- dis.c */

/* Print signed operands with '#' prefixes.  */

static void
print_signed_with_hash_prefix (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
			       void * dis_info,
			       long value,
			       unsigned int attrs ATTRIBUTE_UNUSED,
			       bfd_vma pc ATTRIBUTE_UNUSED,
			       int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  (*info->fprintf_func) (info->stream, "#");
  (*info->fprintf_func) (info->stream, "%ld", value);
}

/* Print unsigned operands with '#' prefixes.  */

static void
print_unsigned_with_hash_prefix (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
				 void * dis_info,
				 long value,
				 unsigned int attrs ATTRIBUTE_UNUSED,
				 bfd_vma pc ATTRIBUTE_UNUSED,
				 int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  (*info->fprintf_func) (info->stream, "#");
  (*info->fprintf_func) (info->stream, "0x%lx", value);
}

/* Handle '#' prefixes as operands.  */

static void
print_hash (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	    void * dis_info,
	    long value ATTRIBUTE_UNUSED,
	    unsigned int attrs ATTRIBUTE_UNUSED,
	    bfd_vma pc ATTRIBUTE_UNUSED,
	    int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  (*info->fprintf_func) (info->stream, "#");
}

#undef  CGEN_PRINT_INSN
#define CGEN_PRINT_INSN my_print_insn

static int
my_print_insn (CGEN_CPU_DESC cd,
	       bfd_vma pc,
	       disassemble_info *info)
{
  bfd_byte buffer[CGEN_MAX_INSN_SIZE];
  bfd_byte *buf = buffer;
  int status;
  int buflen = (pc & 3) == 0 ? 4 : 2;
  int big_p = CGEN_CPU_INSN_ENDIAN (cd) == CGEN_ENDIAN_BIG;
  bfd_byte *x;

  /* Read the base part of the insn.  */

  status = (*info->read_memory_func) (pc - ((!big_p && (pc & 3) != 0) ? 2 : 0),
				      buf, buflen, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  /* 32 bit insn?  */
  x = (big_p ? &buf[0] : &buf[3]);
  if ((pc & 3) == 0 && (*x & 0x80) != 0)
    return print_insn (cd, pc, info, buf, buflen);

  /* Print the first insn.  */
  if ((pc & 3) == 0)
    {
      buf += (big_p ? 0 : 2);
      if (print_insn (cd, pc, info, buf, 2) == 0)
	(*info->fprintf_func) (info->stream, UNKNOWN_INSN_MSG);
      buf += (big_p ? 2 : -2);
    }

  x = (big_p ? &buf[0] : &buf[1]);
  if (*x & 0x80)
    {
      /* Parallel.  */
      (*info->fprintf_func) (info->stream, " || ");
      *x &= 0x7f;
    }
  else
    (*info->fprintf_func) (info->stream, " -> ");

  /* The "& 3" is to pass a consistent address.
     Parallel insns arguably both begin on the word boundary.
     Also, branch insns are calculated relative to the word boundary.  */
  if (print_insn (cd, pc & ~ (bfd_vma) 3, info, buf, 2) == 0)
    (*info->fprintf_func) (info->stream, UNKNOWN_INSN_MSG);

  return (pc & 3) ? 2 : 4;
}

/* -- */

void m32r_cgen_print_operand
  (CGEN_CPU_DESC, int, void *, CGEN_FIELDS *, void const *, bfd_vma, int);

/* Main entry point for printing operands.
   XINFO is a `void *' and not a `disassemble_info *' to not put a requirement
   of dis-asm.h on cgen.h.

   This function is basically just a big switch statement.  Earlier versions
   used tables to look up the function to use, but
   - if the table contains both assembler and disassembler functions then
     the disassembler contains much of the assembler and vice-versa,
   - there's a lot of inlining possibilities as things grow,
   - using a switch statement avoids the function call overhead.

   This function could be moved into `print_insn_normal', but keeping it
   separate makes clear the interface between `print_insn_normal' and each of
   the handlers.  */

void
m32r_cgen_print_operand (CGEN_CPU_DESC cd,
			   int opindex,
			   void * xinfo,
			   CGEN_FIELDS *fields,
			   void const *attrs ATTRIBUTE_UNUSED,
			   bfd_vma pc,
			   int length)
{
  disassemble_info *info = (disassemble_info *) xinfo;

  switch (opindex)
    {
    case M32R_OPERAND_ACC :
      print_keyword (cd, info, & m32r_cgen_opval_h_accums, fields->f_acc, 0);
      break;
    case M32R_OPERAND_ACCD :
      print_keyword (cd, info, & m32r_cgen_opval_h_accums, fields->f_accd, 0);
      break;
    case M32R_OPERAND_ACCS :
      print_keyword (cd, info, & m32r_cgen_opval_h_accums, fields->f_accs, 0);
      break;
    case M32R_OPERAND_DCR :
      print_keyword (cd, info, & m32r_cgen_opval_cr_names, fields->f_r1, 0);
      break;
    case M32R_OPERAND_DISP16 :
      print_address (cd, info, fields->f_disp16, 0|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case M32R_OPERAND_DISP24 :
      print_address (cd, info, fields->f_disp24, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case M32R_OPERAND_DISP8 :
      print_address (cd, info, fields->f_disp8, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case M32R_OPERAND_DR :
      print_keyword (cd, info, & m32r_cgen_opval_gr_names, fields->f_r1, 0);
      break;
    case M32R_OPERAND_HASH :
      print_hash (cd, info, 0, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case M32R_OPERAND_HI16 :
      print_normal (cd, info, fields->f_hi16, 0|(1<<CGEN_OPERAND_SIGN_OPT), pc, length);
      break;
    case M32R_OPERAND_IMM1 :
      print_unsigned_with_hash_prefix (cd, info, fields->f_imm1, 0, pc, length);
      break;
    case M32R_OPERAND_SCR :
      print_keyword (cd, info, & m32r_cgen_opval_cr_names, fields->f_r2, 0);
      break;
    case M32R_OPERAND_SIMM16 :
      print_signed_with_hash_prefix (cd, info, fields->f_simm16, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case M32R_OPERAND_SIMM8 :
      print_signed_with_hash_prefix (cd, info, fields->f_simm8, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case M32R_OPERAND_SLO16 :
      print_normal (cd, info, fields->f_simm16, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case M32R_OPERAND_SR :
      print_keyword (cd, info, & m32r_cgen_opval_gr_names, fields->f_r2, 0);
      break;
    case M32R_OPERAND_SRC1 :
      print_keyword (cd, info, & m32r_cgen_opval_gr_names, fields->f_r1, 0);
      break;
    case M32R_OPERAND_SRC2 :
      print_keyword (cd, info, & m32r_cgen_opval_gr_names, fields->f_r2, 0);
      break;
    case M32R_OPERAND_UIMM16 :
      print_unsigned_with_hash_prefix (cd, info, fields->f_uimm16, 0, pc, length);
      break;
    case M32R_OPERAND_UIMM24 :
      print_address (cd, info, fields->f_uimm24, 0|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_ABS_ADDR), pc, length);
      break;
    case M32R_OPERAND_UIMM3 :
      print_unsigned_with_hash_prefix (cd, info, fields->f_uimm3, 0, pc, length);
      break;
    case M32R_OPERAND_UIMM4 :
      print_unsigned_with_hash_prefix (cd, info, fields->f_uimm4, 0, pc, length);
      break;
    case M32R_OPERAND_UIMM5 :
      print_unsigned_with_hash_prefix (cd, info, fields->f_uimm5, 0, pc, length);
      break;
    case M32R_OPERAND_UIMM8 :
      print_unsigned_with_hash_prefix (cd, info, fields->f_uimm8, 0, pc, length);
      break;
    case M32R_OPERAND_ULO16 :
      print_normal (cd, info, fields->f_uimm16, 0, pc, length);
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while printing insn"),
	 opindex);
      abort ();
  }
}

cgen_print_fn * const m32r_cgen_print_handlers[] =
{
  print_insn_normal,
};


void
m32r_cgen_init_dis (CGEN_CPU_DESC cd)
{
  m32r_cgen_init_opcode_table (cd);
  m32r_cgen_init_ibld_table (cd);
  cd->print_handlers = & m32r_cgen_print_handlers[0];
  cd->print_operand = m32r_cgen_print_operand;
}


/* Default print handler.  */

static void
print_normal (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	      void *dis_info,
	      long value,
	      unsigned int attrs,
	      bfd_vma pc ATTRIBUTE_UNUSED,
	      int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  /* Print the operand as directed by the attributes.  */
  if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SEM_ONLY))
    ; /* nothing to do */
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SIGNED))
    (*info->fprintf_func) (info->stream, "%ld", value);
  else
    (*info->fprintf_func) (info->stream, "0x%lx", value);
}

/* Default address handler.  */

static void
print_address (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	       void *dis_info,
	       bfd_vma value,
	       unsigned int attrs,
	       bfd_vma pc ATTRIBUTE_UNUSED,
	       int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  /* Print the operand as directed by the attributes.  */
  if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SEM_ONLY))
    ; /* Nothing to do.  */
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_PCREL_ADDR))
    (*info->print_address_func) (value, info);
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_ABS_ADDR))
    (*info->print_address_func) (value, info);
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SIGNED))
    (*info->fprintf_func) (info->stream, "%ld", (long) value);
  else
    (*info->fprintf_func) (info->stream, "0x%lx", (long) value);
}

/* Keyword print handler.  */

static void
print_keyword (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	       void *dis_info,
	       CGEN_KEYWORD *keyword_table,
	       long value,
	       unsigned int attrs ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  const CGEN_KEYWORD_ENTRY *ke;

  ke = cgen_keyword_lookup_value (keyword_table, value);
  if (ke != NULL)
    (*info->fprintf_func) (info->stream, "%s", ke->name);
  else
    (*info->fprintf_func) (info->stream, "???");
}

/* Default insn printer.

   DIS_INFO is defined as `void *' so the disassembler needn't know anything
   about disassemble_info.  */

static void
print_insn_normal (CGEN_CPU_DESC cd,
		   void *dis_info,
		   const CGEN_INSN *insn,
		   CGEN_FIELDS *fields,
		   bfd_vma pc,
		   int length)
{
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  disassemble_info *info = (disassemble_info *) dis_info;
  const CGEN_SYNTAX_CHAR_TYPE *syn;

  CGEN_INIT_PRINT (cd);

  for (syn = CGEN_SYNTAX_STRING (syntax); *syn; ++syn)
    {
      if (CGEN_SYNTAX_MNEMONIC_P (*syn))
	{
	  (*info->fprintf_func) (info->stream, "%s", CGEN_INSN_MNEMONIC (insn));
	  continue;
	}
      if (CGEN_SYNTAX_CHAR_P (*syn))
	{
	  (*info->fprintf_func) (info->stream, "%c", CGEN_SYNTAX_CHAR (*syn));
	  continue;
	}

      /* We have an operand.  */
      m32r_cgen_print_operand (cd, CGEN_SYNTAX_FIELD (*syn), info,
				 fields, CGEN_INSN_ATTRS (insn), pc, length);
    }
}

/* Subroutine of print_insn. Reads an insn into the given buffers and updates
   the extract info.
   Returns 0 if all is well, non-zero otherwise.  */

static int
read_insn (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	   bfd_vma pc,
	   disassemble_info *info,
	   bfd_byte *buf,
	   int buflen,
	   CGEN_EXTRACT_INFO *ex_info,
	   unsigned long *insn_value)
{
  int status = (*info->read_memory_func) (pc, buf, buflen, info);

  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  ex_info->dis_info = info;
  ex_info->valid = (1 << buflen) - 1;
  ex_info->insn_bytes = buf;

  *insn_value = bfd_get_bits (buf, buflen * 8, info->endian == BFD_ENDIAN_BIG);
  return 0;
}

/* Utility to print an insn.
   BUF is the base part of the insn, target byte order, BUFLEN bytes long.
   The result is the size of the insn in bytes or zero for an unknown insn
   or -1 if an error occurs fetching data (memory_error_func will have
   been called).  */

static int
print_insn (CGEN_CPU_DESC cd,
	    bfd_vma pc,
	    disassemble_info *info,
	    bfd_byte *buf,
	    unsigned int buflen)
{
  CGEN_INSN_INT insn_value;
  const CGEN_INSN_LIST *insn_list;
  CGEN_EXTRACT_INFO ex_info;
  int basesize;

  /* Extract base part of instruction, just in case CGEN_DIS_* uses it. */
  basesize = cd->base_insn_bitsize < buflen * 8 ?
                                     cd->base_insn_bitsize : buflen * 8;
  insn_value = cgen_get_insn_value (cd, buf, basesize, cd->insn_endian);


  /* Fill in ex_info fields like read_insn would.  Don't actually call
     read_insn, since the incoming buffer is already read (and possibly
     modified a la m32r).  */
  ex_info.valid = (1 << buflen) - 1;
  ex_info.dis_info = info;
  ex_info.insn_bytes = buf;

  /* The instructions are stored in hash lists.
     Pick the first one and keep trying until we find the right one.  */

  insn_list = CGEN_DIS_LOOKUP_INSN (cd, (char *) buf, insn_value);
  while (insn_list != NULL)
    {
      const CGEN_INSN *insn = insn_list->insn;
      CGEN_FIELDS fields;
      int length;
      unsigned long insn_value_cropped;

#ifdef CGEN_VALIDATE_INSN_SUPPORTED
      /* Not needed as insn shouldn't be in hash lists if not supported.  */
      /* Supported by this cpu?  */
      if (! m32r_cgen_insn_supported (cd, insn))
        {
          insn_list = CGEN_DIS_NEXT_INSN (insn_list);
	  continue;
        }
#endif

      /* Basic bit mask must be correct.  */
      /* ??? May wish to allow target to defer this check until the extract
	 handler.  */

      /* Base size may exceed this instruction's size.  Extract the
         relevant part from the buffer. */
      if ((unsigned) (CGEN_INSN_BITSIZE (insn) / 8) < buflen &&
	  (unsigned) (CGEN_INSN_BITSIZE (insn) / 8) <= sizeof (unsigned long))
	insn_value_cropped = bfd_get_bits (buf, CGEN_INSN_BITSIZE (insn),
					   info->endian == BFD_ENDIAN_BIG);
      else
	insn_value_cropped = insn_value;

      if ((insn_value_cropped & CGEN_INSN_BASE_MASK (insn))
	  == CGEN_INSN_BASE_VALUE (insn))
	{
	  /* Printing is handled in two passes.  The first pass parses the
	     machine insn and extracts the fields.  The second pass prints
	     them.  */

	  /* Make sure the entire insn is loaded into insn_value, if it
	     can fit.  */
	  if (((unsigned) CGEN_INSN_BITSIZE (insn) > cd->base_insn_bitsize) &&
	      (unsigned) (CGEN_INSN_BITSIZE (insn) / 8) <= sizeof (unsigned long))
	    {
	      unsigned long full_insn_value;
	      int rc = read_insn (cd, pc, info, buf,
				  CGEN_INSN_BITSIZE (insn) / 8,
				  & ex_info, & full_insn_value);
	      if (rc != 0)
		return rc;
	      length = CGEN_EXTRACT_FN (cd, insn)
		(cd, insn, &ex_info, full_insn_value, &fields, pc);
	    }
	  else
	    length = CGEN_EXTRACT_FN (cd, insn)
	      (cd, insn, &ex_info, insn_value_cropped, &fields, pc);

	  /* Length < 0 -> error.  */
	  if (length < 0)
	    return length;
	  if (length > 0)
	    {
	      CGEN_PRINT_FN (cd, insn) (cd, info, insn, &fields, pc, length);
	      /* Length is in bits, result is in bytes.  */
	      return length / 8;
	    }
	}

      insn_list = CGEN_DIS_NEXT_INSN (insn_list);
    }

  return 0;
}

/* Default value for CGEN_PRINT_INSN.
   The result is the size of the insn in bytes or zero for an unknown insn
   or -1 if an error occured fetching bytes.  */

#ifndef CGEN_PRINT_INSN
#define CGEN_PRINT_INSN default_print_insn
#endif

static int
default_print_insn (CGEN_CPU_DESC cd, bfd_vma pc, disassemble_info *info)
{
  bfd_byte buf[CGEN_MAX_INSN_SIZE];
  int buflen;
  int status;

  /* Attempt to read the base part of the insn.  */
  buflen = cd->base_insn_bitsize / 8;
  status = (*info->read_memory_func) (pc, buf, buflen, info);

  /* Try again with the minimum part, if min < base.  */
  if (status != 0 && (cd->min_insn_bitsize < cd->base_insn_bitsize))
    {
      buflen = cd->min_insn_bitsize / 8;
      status = (*info->read_memory_func) (pc, buf, buflen, info);
    }

  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  return print_insn (cd, pc, info, buf, buflen);
}

/* Main entry point.
   Print one instruction from PC on INFO->STREAM.
   Return the size of the instruction (in bytes).  */

typedef struct cpu_desc_list
{
  struct cpu_desc_list *next;
  CGEN_BITSET *isa;
  int mach;
  int endian;
  int insn_endian;
  CGEN_CPU_DESC cd;
} cpu_desc_list;

int
print_insn_m32r (bfd_vma pc, disassemble_info *info)
{
  static cpu_desc_list *cd_list = 0;
  cpu_desc_list *cl = 0;
  static CGEN_CPU_DESC cd = 0;
  static CGEN_BITSET *prev_isa;
  static int prev_mach;
  static int prev_endian;
  static int prev_insn_endian;
  int length;
  CGEN_BITSET *isa;
  int mach;
  int endian = (info->endian == BFD_ENDIAN_BIG
		? CGEN_ENDIAN_BIG
		: CGEN_ENDIAN_LITTLE);
  int insn_endian = (info->endian_code == BFD_ENDIAN_BIG
                     ? CGEN_ENDIAN_BIG
                     : CGEN_ENDIAN_LITTLE);
  enum bfd_architecture arch;

  /* ??? gdb will set mach but leave the architecture as "unknown" */
#ifndef CGEN_BFD_ARCH
#define CGEN_BFD_ARCH bfd_arch_m32r
#endif
  arch = info->arch;
  if (arch == bfd_arch_unknown)
    arch = CGEN_BFD_ARCH;

  /* There's no standard way to compute the machine or isa number
     so we leave it to the target.  */
#ifdef CGEN_COMPUTE_MACH
  mach = CGEN_COMPUTE_MACH (info);
#else
  mach = info->mach;
#endif

#ifdef CGEN_COMPUTE_ISA
  {
    static CGEN_BITSET *permanent_isa;

    if (!permanent_isa)
      permanent_isa = cgen_bitset_create (MAX_ISAS);
    isa = permanent_isa;
    cgen_bitset_clear (isa);
    cgen_bitset_add (isa, CGEN_COMPUTE_ISA (info));
  }
#else
  isa = info->private_data;
#endif

  /* If we've switched cpu's, try to find a handle we've used before */
  if (cd
      && (cgen_bitset_compare (isa, prev_isa) != 0
	  || mach != prev_mach
	  || endian != prev_endian))
    {
      cd = 0;
      for (cl = cd_list; cl; cl = cl->next)
	{
	  if (cgen_bitset_compare (cl->isa, isa) == 0 &&
	      cl->mach == mach &&
	      cl->endian == endian)
	    {
	      cd = cl->cd;
 	      prev_isa = cd->isas;
	      break;
	    }
	}
    }

  /* If we haven't initialized yet, initialize the opcode table.  */
  if (! cd)
    {
      const bfd_arch_info_type *arch_type = bfd_lookup_arch (arch, mach);
      const char *mach_name;

      if (!arch_type)
	abort ();
      mach_name = arch_type->printable_name;

      prev_isa = cgen_bitset_copy (isa);
      prev_mach = mach;
      prev_endian = endian;
      prev_insn_endian = insn_endian;
      cd = m32r_cgen_cpu_open (CGEN_CPU_OPEN_ISAS, prev_isa,
				 CGEN_CPU_OPEN_BFDMACH, mach_name,
				 CGEN_CPU_OPEN_ENDIAN, prev_endian,
                                 CGEN_CPU_OPEN_INSN_ENDIAN, prev_insn_endian,
				 CGEN_CPU_OPEN_END);
      if (!cd)
	abort ();

      /* Save this away for future reference.  */
      cl = xmalloc (sizeof (struct cpu_desc_list));
      cl->cd = cd;
      cl->isa = prev_isa;
      cl->mach = mach;
      cl->endian = endian;
      cl->next = cd_list;
      cd_list = cl;

      m32r_cgen_init_dis (cd);
    }

  /* We try to have as much common code as possible.
     But at this point some targets need to take over.  */
  /* ??? Some targets may need a hook elsewhere.  Try to avoid this,
     but if not possible try to move this hook elsewhere rather than
     have two hooks.  */
  length = CGEN_PRINT_INSN (cd, pc, info);
  if (length > 0)
    return length;
  if (length < 0)
    return -1;

  (*info->fprintf_func) (info->stream, UNKNOWN_INSN_MSG);
  return cd->default_insn_bitsize / 8;
}
