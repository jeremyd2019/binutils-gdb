/* scfi.c - Support for synthesizing DWARF CFI for hand-written asm.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "scfi.h"
#include "subsegs.h"
#include "scfidw2gen.h"

# if defined (TARGET_USE_SCFI) && defined (TARGET_USE_GINSN)

/* Beyond the target defined number of registers to be tracked (SCFI_NUM_REGS),
   keep the next register ID, in sequence, for REG_CFA.  */
#define REG_CFA	      (SCFI_NUM_REGS+1)
/* Define the total number of registers being tracked.  Used as index into an
   array of cfi_reglocS.  */
#define MAX_NUM_SCFI_REGS   (REG_CFA+1)

enum cfi_reglocstate
{
  CFI_UNDEFINED,
  CFI_IN_REG,
  CFI_ON_STACK
};

/* Location at which CFI register is saved.

   A CFI register (callee-saved registers, RA/LR) are always an offset from
   the CFA.  REG_CFA itself, however, may have REG_SP or REG_FP as base
   register.  Hence, keep the base reg ID and offset per tracked register.  */

struct cfi_regloc
{
  /* Base reg ID (DWARF register number).  */
  uint32_t base;
  /* Location as offset from the CFA.  */
  int32_t offset;
  /* Current state of the CFI register.  */
  enum cfi_reglocstate state;
};

typedef struct cfi_regloc cfi_reglocS;

/* SCFI operation.

   An SCFI operation represents a single atomic change to the SCFI state.
   This can also be understood as an abstraction for what eventually gets
   emitted as a DWARF CFI operation.  */

struct scfi_op
{
  /* An SCFI op updates the state of either the CFA or other tracked
     (callee-saved, REG_SP etc) registers.  'reg' is in the DWARF register
     number space and must be strictly less than MAX_NUM_SCFI_REGS.  */
  uint32_t reg;
  /* Location of the reg.  */
  cfi_reglocS loc;
  /* DWARF CFI opcode.  */
  uint32_t dw2cfi_op;
  /* A linked list.  */
  struct scfi_op *next;
};

/* SCFI State - accumulated unwind information at a PC.

   SCFI state is the accumulated unwind information encompassing:
      - REG_SP, REG_FP,
      - RA, and
      - all callee-saved registers.
 
    Note that SCFI_NUM_REGS is target/ABI dependent and is provided by the
    backends.  The backend must also identify the REG_SP, and REG_FP
    registers.  */

struct scfi_state
{
  cfi_reglocS regs[MAX_NUM_SCFI_REGS];
  cfi_reglocS scratch[MAX_NUM_SCFI_REGS];
  /* Current stack size.  */
  int32_t stack_size;
  /* Is the stack size known?
     Stack size may become untraceable depending on the specific stack
     manipulation machine instruction, e.g., rsp = rsp op reg.  */
  bool traceable_p;
};

/* Initialize a new SCFI op.  */

static scfi_opS *
init_scfi_op (void)
{
  scfi_opS *op = XCNEW (scfi_opS);

  return op;
}

/* Compare two SCFI states.  */

static int
cmp_scfi_state (scfi_stateS *state1, scfi_stateS *state2)
{
  int ret;

  if (!state1 || !state2)
    ret = 1;

  /* Skip comparing the scratch[] value of registers.  The user visible
     unwind information is derived from the regs[] from the SCFI state.  */
  ret = memcmp (state1->regs, state2->regs,
		sizeof (cfi_reglocS) * MAX_NUM_SCFI_REGS);
  ret |= state1->stack_size != state2->stack_size;
  ret |= state1->traceable_p != state2->traceable_p;

  return ret;
}

#if 0
static void
scfi_state_update_reg (scfi_stateS *state, uint32_t dst, uint32_t base,
		       int32_t offset)
{
  if (dst >= MAX_NUM_SCFI_REGS)
    return;

  state->regs[dst].base = base;
  state->regs[dst].offset = offset;
}
#endif

/* Update the SCFI state of REG as available on execution stack at OFFSET
   from REG_CFA (BASE).

   Note that BASE must be REG_CFA, because any other base (REG_SP, REG_FP)
   is by definition transitory in the function.  */

static void
scfi_state_save_reg (scfi_stateS *state, uint32_t reg, uint32_t base,
		     int32_t offset)
{
  if (reg >= MAX_NUM_SCFI_REGS)
    return;

  gas_assert (base == REG_CFA);

  state->regs[reg].base = base;
  state->regs[reg].offset = offset;
  state->regs[reg].state = CFI_ON_STACK;
}

static void
scfi_state_restore_reg (scfi_stateS *state, uint32_t reg)
{
  if (reg >= MAX_NUM_SCFI_REGS)
    return;

  /* Sanity check.  See Rule 4. */
  gas_assert (state->regs[reg].state == CFI_ON_STACK);
  gas_assert (state->regs[reg].base == REG_CFA);

  state->regs[reg].base = reg;
  state->regs[reg].offset = 0;
  /* PS: the register may still be on stack much after the restore, but the
     SCFI state keeps the state as 'in register'.  */
  state->regs[reg].state = CFI_IN_REG;
}

/* Identify if the given GAS instruction GINSN saves a register
   (of interest) on stack.  */

static bool
ginsn_scfi_save_reg_p (ginsnS *ginsn, scfi_stateS *state)
{
  bool save_reg_p = false;
  struct ginsn_src *src;
  struct ginsn_dst *dst;

  src = ginsn_get_src1 (ginsn);
  dst = ginsn_get_dst (ginsn);

  if (!ginsn_track_reg_p (ginsn_get_src_reg (src), GINSN_GEN_SCFI))
    return save_reg_p;

  /* A register save insn may be an indirect mov.  */
  if (ginsn->type == GINSN_TYPE_MOV
      && ginsn_get_dst_type (dst) == GINSN_DST_INDIRECT
      && (ginsn_get_dst_reg (dst) == REG_SP
	  || (ginsn_get_dst_reg (dst) == REG_FP
	      && state->regs[REG_CFA].base == REG_FP)))
    save_reg_p = true;
  /* or an explicit store to stack.  */
  else if (ginsn->type == GINSN_TYPE_STS)
    save_reg_p = true;

  return save_reg_p;
}

/* Identify if the given GAS instruction GINSN restores a register
   (of interest) on stack.  */

static bool
ginsn_scfi_reg_restore_p (ginsnS *ginsn, scfi_stateS *state)
{
  bool reg_restore_p = false;
  struct ginsn_dst *dst;
  struct ginsn_src *src1;

  dst = ginsn_get_dst (ginsn);
  src1 = ginsn_get_src1 (ginsn);

  if (!ginsn_track_reg_p (ginsn_get_dst_reg (dst), GINSN_GEN_SCFI))
    return reg_restore_p;

  /* A register restore insn may be an indirect mov.  */
  if (ginsn->type == GINSN_TYPE_MOV
      && ginsn_get_dst_type (dst) == GINSN_DST_INDIRECT
      && (ginsn_get_src_reg (src1) == REG_SP
	  || (ginsn_get_src_reg (src1) == REG_FP
	      && state->regs[REG_CFA].base == REG_FP)))
    reg_restore_p = true;
  /* or an explicit load from stack.  */
  else if (ginsn->type == GINSN_TYPE_LDS)
    reg_restore_p = true;

  return reg_restore_p;
}

/* Append the SCFI operation OP to the list of SCFI operations in the
   given GINSN.  */

static int
ginsn_append_scfi_op (ginsnS *ginsn, scfi_opS *op)
{
  scfi_opS *sop;

  if (!ginsn || !op)
    return 1;

  if (!ginsn->scfi_ops)
    {
      ginsn->scfi_ops = XCNEW (scfi_opS *);
      *ginsn->scfi_ops = op;
    }
  else
    {
      /* Add to tail.  Most ginsns have a single SCFI operation,
	 so this traversal for every insertion is acceptable for now.  */
      sop = *ginsn->scfi_ops;
      while (sop->next)
	sop = sop->next;

      sop->next = op;
    }
  ginsn->num_scfi_ops++;

  return 0;
}

static void
scfi_op_add_def_cfa_reg (scfi_stateS *state, ginsnS *ginsn, uint32_t reg)
{
  scfi_opS *op = NULL;

  state->regs[REG_CFA].base = reg;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_def_cfa_register;
  op->reg = REG_CFA;
  op->loc = state->regs[REG_CFA];

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_cfa_offset_inc (scfi_stateS *state, ginsnS *ginsn, int32_t num)
{
  scfi_opS *op = NULL;

  state->regs[REG_CFA].offset -= num;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_def_cfa_offset;
  op->reg = REG_CFA;
  op->loc = state->regs[REG_CFA];

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_cfa_offset_dec (scfi_stateS *state, ginsnS *ginsn, int32_t num)
{
  scfi_opS *op = NULL;

  state->regs[REG_CFA].offset += num;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_def_cfa_offset;
  op->reg = REG_CFA;
  op->loc = state->regs[REG_CFA];

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_def_cfa (scfi_stateS *state, ginsnS *ginsn, uint32_t reg,
		     int32_t num)
{
  scfi_opS *op = NULL;

  /* On most architectures, CFA is already somewhere on stack.  */
  gas_assert (num > 0);

  state->regs[REG_CFA].base = reg;
  state->regs[REG_CFA].offset = num;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_def_cfa;
  op->reg = REG_CFA;
  op->loc = state->regs[REG_CFA];

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_cfi_offset (scfi_stateS *state, ginsnS *ginsn, uint32_t reg)
{
  scfi_opS *op = NULL;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_offset;
  op->reg = reg;
  op->loc = state->regs[reg];

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_cfa_restore (ginsnS *ginsn, uint32_t reg)
{
  scfi_opS *op = NULL;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_restore;
  op->reg = reg;
  op->loc.base = -1; /* FIXME invalidate.  */
  op->loc.offset = 0;

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_cfi_remember_state (ginsnS *ginsn)
{
  scfi_opS *op = NULL;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_remember_state;

  ginsn_append_scfi_op (ginsn, op);
}

static void
scfi_op_add_cfi_restore_state (ginsnS *ginsn)
{
  scfi_opS *op = NULL;

  op = init_scfi_op ();

  op->dw2cfi_op = DW_CFA_restore_state;

  /* FIXME - add to the beginning of the scfi_ops.  */
  ginsn_append_scfi_op (ginsn, op);
}

static int
verify_heuristic_traceable_reg_bp (ginsnS *ginsn, scfi_stateS *state)
{
  /* The function uses this variable to issue error to user right away.  */
  int reg_bp_scratch_p = 0;
  struct ginsn_dst *dst;
  struct ginsn_src *src1;
  struct ginsn_src *src2;

  src1 = ginsn_get_src1 (ginsn);
  src2 = ginsn_get_src2 (ginsn);
  dst = ginsn_get_dst (ginsn);

  /* Stack manipulation can be done in a variety of ways.  A program may
     allocate in statically in epilogue or may need to do dynamic stack
     allocation.

     The SCFI machinery in GAS is based on some heuristics:

       - Rule 3 If the base register for CFA tracking is REG_FP, the program
       must not clobber REG_FP, unless it is for switch to REG_SP based CFA
       tracking (via say, a pop %rbp in X86).  Currently the code does not
       guard the programmer from violations of this rule.  */

  /* Check add/sub insn with imm usage when CFA base register is REG_FP.  */
  if (state->regs[REG_CFA].base == REG_FP && ginsn_get_dst_reg (dst) == REG_FP)
    {
      if ((ginsn->type == GINSN_TYPE_ADD || ginsn->type == GINSN_TYPE_SUB)
	  && ginsn_get_src_reg (src1) == REG_FP
	  && ginsn_get_src_type (src2) == GINSN_SRC_IMM)
	reg_bp_scratch_p = 0;
      /* REG_FP restore is allowed.  */
      else if (ginsn->type == GINSN_TYPE_LDS)
	reg_bp_scratch_p = 0;
      /* mov's to memory with REG_FP base.  */
      else if (ginsn->type == GINSN_TYPE_MOV
	       && ginsn_get_dst_type (dst) == GINSN_DST_INDIRECT)
	reg_bp_scratch_p = 0;
      /* All other ginsns with REG_FP as destination make REG_FP not
	 traceable.  */
      else
	reg_bp_scratch_p = 1;
    }

  if (reg_bp_scratch_p)
    as_bad_where (ginsn->file, ginsn->line,
		  _("SCFI: usage of REG_FP as scratch not supported"));

  return reg_bp_scratch_p;
}

static int
verify_heuristic_traceable_stack_manipulation (ginsnS *ginsn,
					       scfi_stateS *state)
{
  /* The function uses this variable to issue error to user right away.  */
  int not_traceable = 0;
  struct ginsn_dst *dst;
  struct ginsn_src *src1;
  struct ginsn_src *src2;

  src1 = ginsn_get_src1 (ginsn);
  src2 = ginsn_get_src2 (ginsn);
  dst = ginsn_get_dst (ginsn);

  /* Stack manipulation can be done in a variety of ways.  A program may
     allocate in statically in epilogue or may need to do dynamic stack
     allocation.

     The SCFI machinery in GAS is based on some heuristics:

       - Rule 1 The base register for CFA tracking may be either REG_SP or
       REG_FP.

       - Rule 2 If the base register for CFA tracking is REG_SP, the precise
       amount of stack usage (and hence, the value of rsp) must be known at
       all times.  */

  /* Check add/sub/and insn usage when CFA base register is REG_SP.
     Any stack size manipulation, including stack realignment is not allowed
     if CFA base register is REG_SP.  */
  if (ginsn_get_dst_reg (dst) == REG_SP
      && (((ginsn->type == GINSN_TYPE_ADD || ginsn->type == GINSN_TYPE_SUB)
	   && ginsn_get_src_type (src2) != GINSN_SRC_IMM)
	  || ginsn->type == GINSN_TYPE_AND))
    {
      /* See Rule 2. For SP-based CFA, this (src2 not being imm) makes CFA
	 tracking not possible.  Propagate now to caller.  */
      if (state->regs[REG_CFA].base == REG_SP)
	not_traceable = 1;
      else if (state->traceable_p)
	{
	  /* An extension of Rule 2.
	     For FP-based CFA, this may be a problem *if* certain specific
	     changes to the SCFI state are seen beyond this point. E.g.,
	     register save / restore from stack.  */
	  gas_assert (state->regs[REG_CFA].base == REG_FP);
	  /* Simply make a note in the SCFI state object for now and
	     continue.  Indicate an error when register save / restore
	     for callee-saved registers is seen.  */
	  not_traceable = 0;
	  state->traceable_p = false;
	}
    }
  else if (ginsn_scfi_save_reg_p (ginsn, state) && !state->traceable_p)
    {
      if (ginsn->type == GINSN_TYPE_MOV
	  && ginsn_get_dst_type (dst) == GINSN_DST_INDIRECT
	  && (ginsn_get_dst_reg (dst) == REG_SP
	      || (ginsn_get_dst_reg (dst) == REG_FP
		  && state->regs[REG_CFA].base != REG_FP)))
	not_traceable = 1;
    }
  else if (ginsn_scfi_reg_restore_p (ginsn, state) && !state->traceable_p)
    {
      if (ginsn->type == GINSN_TYPE_MOV
	  && ginsn_get_dst_type (dst) == GINSN_DST_INDIRECT
	  && (ginsn_get_src_reg (src1) == REG_SP
	      || (ginsn_get_src_reg (src1) == REG_FP
		  && state->regs[REG_CFA].base != REG_FP)))
	not_traceable = 1;
    }

  if (not_traceable)
    as_bad_where (ginsn->file, ginsn->line,
		  _("SCFI: unsupported stack manipulation pattern"));

  return not_traceable;
}

static int
verify_heuristic_symmetrical_restore_reg (scfi_stateS *state, uint32_t reg,
					  int32_t expected_offset)
{
  int sym_restore;

  /* Rule 4: Save and Restore of callee-saved registers must be symmetrical.
     It is expected that value of the saved register is restored correctly.
     E.g.,
	push  reg1
	push  reg2
	...
	body of func which uses reg1 , reg2 as scratch,
	and may be even spills them to stack.
	...
	pop   reg2
	pop   reg1
     It is difficult to verify the Rule 4 in all cases.  For the SCFI machinery,
     it is difficult to separate prologue-epilogue from the body of the function

     Hence, the SCFI machinery at this time, should only warn on an asymmetrical
     restore.  */

  /* The register must have been saved on stack, for sure.  */
  gas_assert (state->regs[reg].state == CFI_ON_STACK);
  gas_assert (state->regs[reg].base == REG_CFA);

  sym_restore = (expected_offset == state->regs[reg].offset);

  return sym_restore;
}

/* Perform symbolic execution of the GINSN and update its list of scfi_ops.
   scfi_ops are later used to directly generate the DWARF CFI directives.
   Also update the SCFI state object STATE for the caller.  */

static int
gen_scfi_ops (ginsnS *ginsn, scfi_stateS *state)
{
  int ret = 0;
  int32_t offset;
  int32_t expected_offset;
  struct ginsn_src *src1;
  struct ginsn_src *src2;
  struct ginsn_dst *dst;

  if (!ginsn || !state)
    ret = 1;

  /* For the first ginsn (of type GINSN_TYPE_SYMBOL) in the gbb, generate
     the SCFI op with DW_CFA_def_cfa.  Note that the register and offset are
     target-specific.  */
  if (GINSN_F_FUNC_BEGIN_P(ginsn))
    {
      scfi_op_add_def_cfa (state, ginsn, REG_SP, SCFI_INIT_CFA_OFFSET);
      state->stack_size += SCFI_INIT_CFA_OFFSET;
      return ret;
    }

  src1 = ginsn_get_src1 (ginsn);
  src2 = ginsn_get_src2 (ginsn);
  dst = ginsn_get_dst (ginsn);

  ret = verify_heuristic_traceable_stack_manipulation (ginsn, state);
  if (ret)
    return ret;

  ret = verify_heuristic_traceable_reg_bp (ginsn, state);
  if (ret)
    return ret;

  switch (ginsn->dst.type)
    {
    case GINSN_DST_REG:
      switch (ginsn->type)
	{
	case GINSN_TYPE_MOV:
	  if (ginsn_get_src_type (src1) == GINSN_SRC_REG
	      && ginsn_get_src_reg (src1) == REG_SP
	      && ginsn_get_dst_reg (dst) == REG_FP
	      && state->regs[REG_CFA].base == REG_SP)
	    {
	      /* mov %rsp, %rbp.  */
	      scfi_op_add_def_cfa_reg (state, ginsn, ginsn_get_dst_reg (dst));
	    }
	  else if (ginsn_get_src_type (src1) == GINSN_SRC_REG
		   && ginsn_get_src_reg (src1) == REG_FP
		   && ginsn_get_dst_reg (dst) == REG_SP
		   && state->regs[REG_CFA].base == REG_FP)
	    {
	      /* mov %rbp, %rsp.  */
	      state->stack_size = -state->regs[REG_FP].offset;
	      scfi_op_add_def_cfa_reg (state, ginsn, ginsn_get_dst_reg (dst));
	      state->traceable_p = true;
	    }
	  else if (ginsn_get_src_type (src1) == GINSN_SRC_INDIRECT
		   && (ginsn_get_src_reg (src1) == REG_SP
		       || ginsn_get_src_reg (src1) == REG_FP)
		   && ginsn_track_reg_p (ginsn_get_dst_reg (dst), GINSN_GEN_SCFI))
	    {
	      /* mov disp(%rsp), reg.  */
	      /* mov disp(%rbp), reg.  */
	      expected_offset = (((ginsn_get_src_reg (src1) == REG_SP)
				  ? -state->stack_size
				  : state->regs[REG_FP].offset)
				 + ginsn_get_src_disp (src1));
	      if (verify_heuristic_symmetrical_restore_reg (state, ginsn_get_dst_reg (dst),
							    expected_offset))
		{
		  scfi_state_restore_reg (state, ginsn_get_dst_reg (dst));
		  scfi_op_add_cfa_restore (ginsn, ginsn_get_dst_reg (dst));
		}
	      else
		as_warn_where (ginsn->file, ginsn->line,
			       _("SCFI: asymetrical register restore"));
	    }
	  else if (ginsn_get_src_type (src1) == GINSN_SRC_REG
		   && ginsn_get_dst_type (dst) == GINSN_DST_REG
		   && ginsn_get_src_reg (src1) == REG_SP)
	    {
	      /* mov %rsp, %reg.  */
	      /* The value of rsp is taken directly from state->stack_size.
		 IMP: The workflow in gen_scfi_ops must keep it updated.
		 PS: Not taking the value from state->scratch[REG_SP] is
		 intentional.  */
	      state->scratch[ginsn_get_dst_reg (dst)].base = REG_CFA;
	      state->scratch[ginsn_get_dst_reg (dst)].offset = -state->stack_size;
	    }
	  else if (ginsn_get_src_type (src1) == GINSN_SRC_REG
		   && ginsn_get_dst_type (dst) == GINSN_DST_REG
		   && ginsn_get_dst_reg (dst) == REG_SP)
	    {
	      /* mov %reg, %rsp.  */
	      /* Keep the value of REG_SP updated.  */
	      state->stack_size = -state->scratch[ginsn_get_src_reg (src1)].offset;
# if 0
	      scfi_state_update_reg (state, ginsn_get_dst_reg (dst),
				     state->scratch[ginsn_get_src_reg (src1)].base,
				     state->scratch[ginsn_get_src_reg (src1)].offset);
#endif

	      state->traceable_p = true;
	    }
	  break;
	case GINSN_TYPE_SUB:
	  if (ginsn_get_src_reg (src1) == REG_SP
	      && ginsn_get_dst_reg (dst) == REG_SP)
	    {
	      /* Stack inc/dec offset, when generated due to stack push and pop is
		 target-specific.  Use the value encoded in the ginsn.  */
	      state->stack_size += ginsn_get_src_imm (src2);
	      if (state->regs[REG_CFA].base == REG_SP)
		{
		  /* push reg.  */
		  scfi_op_add_cfa_offset_dec (state, ginsn, ginsn_get_src_imm (src2));
		}
	    }
	  break;
	case GINSN_TYPE_ADD:
	  if (ginsn_get_src_reg (src1) == REG_SP
	      && ginsn_get_dst_reg (dst) == REG_SP)
	    {
	      /* Stack inc/dec offset is target-specific.  Use the value
		 encoded in the ginsn.  */
	      state->stack_size -= ginsn_get_src_imm (src2);
	      /* pop %reg affects CFA offset only if CFA is currently
		 stack-pointer based.  */
	      if (state->regs[REG_CFA].base == REG_SP)
		{
		  scfi_op_add_cfa_offset_inc (state, ginsn, ginsn_get_src_imm (src2));
		}
	    }
	  else if (ginsn_get_src_reg (src1) == REG_FP
		   && ginsn_get_dst_reg (dst) == REG_SP
		   && state->regs[REG_CFA].base == REG_FP)
	    {
	      /* FIXME - what is this for ? */
	      state->stack_size =  0 - (state->regs[REG_FP].offset + ginsn_get_src_imm (src2));
	    }
	  break;
	case GINSN_TYPE_LDS:
	  /* pop %rbp when CFA tracking is frame-pointer based.  */
	  if (ginsn_get_dst_reg (dst) == REG_FP && state->regs[REG_CFA].base == REG_FP)
	    {
	      scfi_op_add_def_cfa_reg (state, ginsn, REG_SP);
	    }
	  if (ginsn_track_reg_p (ginsn_get_dst_reg (dst), GINSN_GEN_SCFI))
	    {
	      expected_offset = -state->stack_size;
	      if (verify_heuristic_symmetrical_restore_reg (state, ginsn_get_dst_reg (dst),
							    expected_offset))
		{
		  scfi_state_restore_reg (state, ginsn_get_dst_reg (dst));
		  scfi_op_add_cfa_restore (ginsn, ginsn_get_dst_reg (dst));
		}
	      else
		as_warn_where (ginsn->file, ginsn->line,
			       _("SCFI: asymetrical register restore"));
	    }
	  break;
	default:
	  break;
	}
      break;

    case GINSN_DST_STACK:
      gas_assert (ginsn->type == GINSN_TYPE_STS);

      if (ginsn_track_reg_p (ginsn_get_src_reg (src1), GINSN_GEN_SCFI)
	  && state->regs[ginsn_get_src_reg (src1)].state != CFI_ON_STACK)
	{
	  /* reg is saved on stack at the current value of REG_SP.  */
	  offset = 0 - state->stack_size;
	  scfi_state_save_reg (state, ginsn_get_src_reg (src1), REG_CFA, offset);
	  /* Track callee-saved registers.  */
	  scfi_op_add_cfi_offset (state, ginsn, ginsn_get_src_reg (src1));
	}
      break;

    case GINSN_DST_INDIRECT:
      gas_assert (ginsn->type == GINSN_TYPE_MOV);
      /* mov reg, disp(%rbp) */
      /* mov reg, disp(%rsp) */
      if (ginsn_track_reg_p (ginsn_get_src_reg (src1), GINSN_GEN_SCFI)
	  && state->regs[ginsn_get_src_reg (src1)].state != CFI_ON_STACK)
	{
	  if (ginsn_get_dst_reg (dst) == REG_SP)
	    {
	      /* mov reg, disp(%rsp) */
	      offset = 0 - state->stack_size + ginsn_get_dst_disp (dst);
	      scfi_state_save_reg (state, ginsn_get_src_reg (src1), REG_CFA, offset);
	      scfi_op_add_cfi_offset (state, ginsn, ginsn_get_src_reg (src1));
	    }
	  else if (ginsn_get_dst_reg (dst) == REG_FP)
	    {
	      gas_assert (state->regs[REG_CFA].base == REG_FP);
	      /* mov reg, disp(%rbp) */
	      offset = 0 - state->regs[REG_CFA].offset + ginsn_get_dst_disp (dst);
	      scfi_state_save_reg (state, ginsn_get_src_reg (src1), REG_CFA, offset);
	      scfi_op_add_cfi_offset (state, ginsn, ginsn_get_src_reg (src1));
	    }
	}
      break;

    default:
      /* Skip GINSN_DST_UNKNOWN and GINSN_DST_MEM as they are uninteresting
	 currently for SCFI.  */
      break;
    }

  return ret;
}

/* Recursively perform forward flow of the (unwind information) SCFI state
   starting at basic block GBB.

   The forward flow process propagates the SCFI state at exit of a basic block
   to the successor basic block.

   Returns error code, if any.  */

static int
forward_flow_scfi_state (gcfgS *gcfg, gbbS *gbb, scfi_stateS *state)
{
  ginsnS *ginsn;
  gbbS *prev_bb;
  gedgeS *gedge = NULL;
  int ret = 0;

  if (gbb->visited)
    {
      /* Check that the SCFI state is the same as previous.  */
      ret = cmp_scfi_state (state, gbb->entry_state);
      if (ret)
	as_bad (_("SCFI: Bad CFI propagation perhaps"));
      return ret;
    }

  gbb->visited = true;

  gbb->entry_state = XCNEW (scfi_stateS);
  memcpy (gbb->entry_state, state, sizeof (scfi_stateS));

  /* Perform symbolic execution of each ginsn in the gbb and update the
     scfi_ops list of each ginsn (and also update the STATE object).   */
  bb_for_each_insn(gbb, ginsn)
    {
      ret = gen_scfi_ops (ginsn, state);
      if (ret)
	goto fail;
    }

  gbb->exit_state = XCNEW (scfi_stateS);
  memcpy (gbb->exit_state, state, sizeof (scfi_stateS));

  /* Forward flow the SCFI state.  Currently, we process the next basic block
     in DFS order.  But any forward traversal order should be fine.  */
  prev_bb = gbb;
  if (gbb->num_out_gedges)
    {
      bb_for_each_edge(gbb, gedge)
	{
	  gbb = gedge->dst_bb;
	  if (gbb->visited)
	    {
	      ret = cmp_scfi_state (gbb->entry_state, state);
	      if (ret)
		goto fail;
	    }

	  if (!gedge->visited)
	    {
	      gedge->visited = true;

	      /* Entry SCFI state for the destination bb of the edge is the
		 same as the exit SCFI state of the source bb of the edge.  */
	      memcpy (state, prev_bb->exit_state, sizeof (scfi_stateS));
	      ret = forward_flow_scfi_state (gcfg, gbb, state);
	      if (ret)
		goto fail;
	    }
	}
    }

  return 0;

fail:

  if (gedge)
    gedge->visited = true;
  return 1;
}

static int
backward_flow_scfi_state (symbolS *func ATTRIBUTE_UNUSED, gcfgS *gcfg)
{
  gbbS **prog_order_bbs;
  gbbS **restore_bbs;
  gbbS *current_bb;
  gbbS *prev_bb;
  gbbS *dst_bb;
  ginsnS *ginsn;
  gedgeS *gedge;

  int ret = 0;
  int i, j;

  /* Basic blocks in reverse program order.  */
  prog_order_bbs = XCNEWVEC (gbbS *, gcfg->num_gbbs);
  /* Basic blocks for which CFI remember op needs to be generated.  */
  restore_bbs = XCNEWVEC (gbbS *, gcfg->num_gbbs);

  gcfg_get_bbs_in_prog_order (gcfg, prog_order_bbs);

  i = gcfg->num_gbbs - 1;
  /* Traverse in reverse program order.  */
  while (i > 0)
    {
      current_bb = prog_order_bbs[i];
      prev_bb = prog_order_bbs[i-1];
      if (cmp_scfi_state (prev_bb->exit_state, current_bb->entry_state))
	{
	  /* Candidate for .cfi_restore_state found.  */
	  ginsn = bb_get_first_ginsn (current_bb);
	  scfi_op_add_cfi_restore_state (ginsn);
	  /* Memorize current_bb now to find location for its remember state
	     later.  */
	  restore_bbs[i] = current_bb;
	}
      else
	{
	  bb_for_each_edge (current_bb, gedge)
	    {
	      dst_bb = gedge->dst_bb;
	      for (j = 0; j < gcfg->num_gbbs; j++)
		if (restore_bbs[j] == dst_bb)
		  {
		    ginsn = bb_get_last_ginsn (current_bb);
		    scfi_op_add_cfi_remember_state (ginsn);
		    /* Remove the memorised restore_bb from the list.  */
		    restore_bbs[j] = NULL;
		    break;
		  }
	    }
	}
      i--;
    }

  /* All .cfi_restore_state pseudo-ops must have a corresponding
     .cfi_remember_state by now.  */
  for (j = 0; j < gcfg->num_gbbs; j++)
    if (restore_bbs[j] != NULL)
      {
	ret = 1;
	break;
      }

  free (restore_bbs);
  free (prog_order_bbs);

  return ret;
}

/* Synthesize DWARF CFI for a function.  */

int
scfi_synthesize_dw2cfi (symbolS *func, gcfgS *gcfg, gbbS *root_bb)
{
  int ret;
  scfi_stateS *init_state;

  init_state = XCNEW (scfi_stateS);
  init_state->traceable_p = true;

  /* Traverse the input GCFG and perform forward flow of information.
     Update the scfi_op(s) per ginsn.  */
  ret = forward_flow_scfi_state (gcfg, root_bb, init_state);
  if (ret)
    {
      as_warn (_("SCFI: forward pass failed for func '%s'"), S_GET_NAME (func));
      goto end;
    }

  ret = backward_flow_scfi_state (func, gcfg);
  if (ret)
    {
      as_warn (_("SCFI: backward pass failed for func '%s'"), S_GET_NAME (func));
      goto end;
    }

end:
  free (init_state);
  return ret;
}

static int
handle_scfi_dot_cfi (ginsnS *ginsn)
{
  scfi_opS *op;

  /* Nothing to do.  */
  if (!ginsn->scfi_ops)
    return 0;

  op = *ginsn->scfi_ops;
  if (!op)
    goto bad;

  while (op)
    {
      switch (op->dw2cfi_op)
	{
	case DW_CFA_def_cfa_register:
	  scfi_dot_cfi (DW_CFA_def_cfa_register, op->loc.base, 0, 0,
			ginsn->sym);
	  break;
	case DW_CFA_def_cfa_offset:
	  scfi_dot_cfi (DW_CFA_def_cfa_offset, op->loc.base, 0,
			op->loc.offset, ginsn->sym);
	  break;
	case DW_CFA_def_cfa:
	  scfi_dot_cfi (DW_CFA_def_cfa, op->loc.base, 0, op->loc.offset,
			ginsn->sym );
	  break;
	case DW_CFA_offset:
	  scfi_dot_cfi (DW_CFA_offset, op->reg, 0, op->loc.offset, ginsn->sym);
	  break;
	case DW_CFA_restore:
	  scfi_dot_cfi (DW_CFA_restore, op->reg, 0, 0, ginsn->sym);
	  break;
	case DW_CFA_remember_state:
	  scfi_dot_cfi (DW_CFA_remember_state, 0, 0, 0, ginsn->sym);
	  break;
	case DW_CFA_restore_state:
	  scfi_dot_cfi (DW_CFA_restore_state, 0, 0, 0, ginsn->sym);
	  break;
	default:
	  goto bad;
	  break;
	}
      op = op->next;
    }

  return 0;
bad:
  as_bad (_("SCFI: Invalid DWARF CFI opcode data"));
  return 1;
}

/* Emit Synthesized DWARF CFI.  */

int
scfi_emit_dw2cfi (symbolS *func)
{
  struct frch_ginsn_data *frch_gdata;
  ginsnS* ginsn = NULL;

  frch_gdata = frchain_now->frch_ginsn_data;
  ginsn = frch_gdata->gins_rootP;

  while (ginsn)
    {
      switch (ginsn->type)
	{
	  case GINSN_TYPE_SYMBOL:
	    /* .cfi_startproc and .cfi_endproc pseudo-ops.  */
	    if (GINSN_F_FUNC_BEGIN_P(ginsn))
	      {
		scfi_dot_cfi_startproc (frch_gdata->start_addr);
		break;
	      }
	    else if (GINSN_F_FUNC_END_P(ginsn))
	      {
		scfi_dot_cfi_endproc (ginsn->sym);
		break;
	      }
	    /* Fall through.  */
	  case GINSN_TYPE_ADD:
	  case GINSN_TYPE_AND:
	  case GINSN_TYPE_CALL:
	  case GINSN_TYPE_JUMP:
	  case GINSN_TYPE_JUMP_COND:
	  case GINSN_TYPE_MOV:
	  case GINSN_TYPE_LDS:
	  case GINSN_TYPE_STS:
	  case GINSN_TYPE_SUB:
	  case GINSN_TYPE_OTHER:
	  case GINSN_TYPE_RETURN:

	    /* For all other SCFI ops, invoke the handler.  */
	    if (ginsn->scfi_ops)
	      handle_scfi_dot_cfi (ginsn);
	    break;

	  default:
	    /* No other GINSN_TYPE_* expected.  */
	    as_bad (_("SCFI: bad ginsn for func '%s'"),
		    S_GET_NAME (func));
	    break;
	}
      ginsn = ginsn->next;
    }
  return 0;
}

#else

int
scfi_emit_dw2cfi (symbolS *func ATTRIBUTE_UNUSED)
{
  as_bad (_("SCFI: unsupported for target"));
  return 1;
}

int
scfi_synthesize_dw2cfi (symbolS *func ATTRIBUTE_UNUSED,
			gcfgS *gcfg ATTRIBUTE_UNUSED,
			gbbS *root_bb ATTRIBUTE_UNUSED)
{
  as_bad (_("SCFI: unsupported for target"));
  return 1;
}

#endif  /* defined (TARGET_USE_SCFI) && defined (TARGET_USE_GINSN).  */
