/* GDB target debugging macros

   Copyright (C) 2014-2024 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef GDB_TARGET_DEBUG_H
#define GDB_TARGET_DEBUG_H

/* Printers for the debug target.  Each prints an object of a given
   type to a string that needn't be freed.  Most printers are macros,
   for brevity, but a few are static functions where more complicated
   behavior is needed.

   References to these printers are automatically generated by
   make-target-delegates.  See the generated file target-delegates-gen.c.

   In a couple cases, a special printing function is defined and then
   used via the TARGET_DEBUG_PRINTER macro.  See target.h.

   A few methods still have some explicit targetdebug code in
   target.c.  In most cases this is because target delegation hasn't
   been done for the method; but individual cases vary.  For instance,
   target_store_registers does some special register printing that is
   more simply done there, and target_xfer_partial additionally
   bypasses the debug target.  */

#include "gdbarch.h"
#include "gdbsupport/x86-xstate.h"
#include "progspace.h"
#include "target.h"
#include "target/wait.h"
#include "target/waitstatus.h"

/* The functions defined in this header file are not marked "inline", such
   that any function not used by target-delegates-gen.c (the only user of this
   file) will be flagged as unused.  */

static std::string
target_debug_print_target_object (target_object object)
{ return plongest (object); }

static std::string
target_debug_print_CORE_ADDR (CORE_ADDR addr)
{ return core_addr_to_string (addr); }

static std::string
target_debug_print_const_char_p (const char *s)
{ return s != nullptr ? s : "(null)"; }

static std::string
target_debug_print_int (int v)
{ return plongest (v); }

static std::string
target_debug_print_bool (bool v)
{ return v ? "true" : "false"; }

static std::string
target_debug_print_long (long v)
{ return plongest (v); }

static std::string
target_debug_print_target_xfer_status (target_xfer_status status)
{ return plongest (status); }

static std::string
target_debug_print_exec_direction_kind (exec_direction_kind kind)
{ return plongest (kind); }

static std::string
target_debug_print_trace_find_type (trace_find_type type)
{ return plongest (type); }

static std::string
target_debug_print_btrace_read_type (btrace_read_type type)
{ return plongest (type); }

static std::string
target_debug_print_btrace_error (btrace_error error)
{ return plongest (error); }

static std::string
target_debug_print_ptid_t (ptid_t ptid)
{ return plongest (ptid.pid ()); }

static std::string
target_debug_print_gdbarch_p (gdbarch *arch)
{ return gdbarch_bfd_arch_info (arch)->printable_name; }

static std::string
target_debug_print_const_gdb_byte_p (const gdb_byte *p)
{ return host_address_to_string (p); }

static std::string
target_debug_print_gdb_byte_p (gdb_byte *p)
{ return host_address_to_string (p); }

static std::string
target_debug_print_const_gdb_byte_pp (const gdb_byte **p)
{ return host_address_to_string (*p); }

static std::string
target_debug_print_gdb_signal (gdb_signal sig)
{ return gdb_signal_to_name (sig); }

static std::string
target_debug_print_ULONGEST (ULONGEST v)
{ return hex_string (v); }

static std::string
target_debug_print_ULONGEST_p (ULONGEST *p)
{ return hex_string (*p); }

static std::string
target_debug_print_LONGEST (LONGEST v)
{ return phex (v, 0); }

static std::string
target_debug_print_LONGEST_p (LONGEST *p)
{ return phex (*p, 0); }

static std::string
target_debug_print_bp_target_info_p (bp_target_info *bp)
{ return core_addr_to_string (bp->placed_address); }

static std::string
target_debug_print_expression_p (expression *exp)
{ return host_address_to_string (exp); }

static std::string
target_debug_print_CORE_ADDR_p (CORE_ADDR *p)
{ return core_addr_to_string (*p); }

static std::string
target_debug_print_int_p (int *p)
{ return plongest (*p); }

static std::string
target_debug_print_regcache_p (regcache *regcache)
{ return host_address_to_string (regcache); }

static std::string
target_debug_print_thread_info_p (thread_info *thread)
{ return host_address_to_string (thread); }

static std::string
target_debug_print_ui_file_p (ui_file *file)
{ return host_address_to_string (file); }

static std::string
target_debug_print_const_std_vector_target_section_p
  (const std::vector<target_section> *vec)
{ return host_address_to_string (vec->data ()); }

static std::string
target_debug_print_void_p (void *p)
{ return host_address_to_string (p); }

static std::string
target_debug_print_find_memory_region_ftype (find_memory_region_ftype func)
{ return host_address_to_string (func); }

static std::string
target_debug_print_bfd_p (bfd *bfd)
{ return host_address_to_string (bfd); }

static std::string
target_debug_print_std_vector_mem_region (const std::vector<mem_region> &vec)
{ return host_address_to_string (vec.data ()); }

static std::string
target_debug_print_std_vector_static_tracepoint_marker
  (const std::vector<static_tracepoint_marker> &vec)
{ return host_address_to_string (vec.data ()); }

static std::string
target_debug_print_const_target_desc_p (const target_desc *tdesc)
{ return host_address_to_string (tdesc); }

static std::string
target_debug_print_bp_location_p (bp_location *loc)
{ return host_address_to_string (loc); }

static std::string
target_debug_print_const_trace_state_variable_r
  (const trace_state_variable &tsv)
{ return host_address_to_string (&tsv); }

static std::string
target_debug_print_trace_status_p (trace_status *status)
{ return host_address_to_string (status); }

static std::string
target_debug_print_tracepoint_p (tracepoint *tp)
{ return host_address_to_string (tp); }

static std::string
target_debug_print_uploaded_tp_p (uploaded_tp *tp)
{ return host_address_to_string (tp); }

static std::string
target_debug_print_uploaded_tp_pp (uploaded_tp **v)
{ return host_address_to_string (*v); }

static std::string
target_debug_print_uploaded_tsv_pp (uploaded_tsv **tsv)
{ return host_address_to_string (tsv); }

static std::string
target_debug_print_static_tracepoint_marker_p (static_tracepoint_marker *marker)
{ return host_address_to_string (marker); }

static std::string
target_debug_print_btrace_target_info_p (btrace_target_info *info)
{ return host_address_to_string (info); }

static std::string
target_debug_print_const_frame_unwind_p (const frame_unwind *fu)
{ return host_address_to_string (fu); }

static std::string
target_debug_print_btrace_data_p (btrace_data *data)
{ return host_address_to_string (data); }

static std::string
target_debug_print_record_method (record_method method)
{ return plongest (method); }

static std::string
target_debug_print_const_btrace_config_p (const btrace_config *config)
{ return host_address_to_string (config); }

static std::string
target_debug_print_const_btrace_target_info_p (const btrace_target_info *info)
{ return host_address_to_string (info); }

static std::string
target_debug_print_target_hw_bp_type (target_hw_bp_type type)
{ return plongest (type); }

static std::string
target_debug_print_bptype (bptype type)
{ return plongest (type); }

static std::string
target_debug_print_inferior_p (inferior *inf)
{ return host_address_to_string (inf); }

static std::string
target_debug_print_remove_bp_reason (remove_bp_reason reason)
{ return plongest (reason); }

static std::string
target_debug_print_gdb_disassembly_flags (gdb_disassembly_flags flags)
{ return plongest (flags); }

static std::string
target_debug_print_traceframe_info_up (std::unique_ptr<traceframe_info> &info)
{ return host_address_to_string (info.get ()); }

static std::string
target_debug_print_gdb_array_view_const_int
  (const gdb::array_view<const int> &view)
{ return host_address_to_string (view.data ()); }

static std::string
target_debug_print_record_print_flags (record_print_flags flags)
{ return plongest (flags); }

static std::string
target_debug_print_thread_control_capabilities (thread_control_capabilities cap)
{ return plongest (cap); }

static std::string
target_debug_print_std_string (const std::string &str)
{ return str.c_str (); }

static std::string
target_debug_print_gdb_unique_xmalloc_ptr_char
  (const gdb::unique_xmalloc_ptr<char> &p)
{ return p.get (); }

static std::string
target_debug_print_target_waitkind (target_waitkind kind)
{ return pulongest (kind); }

static std::string
target_debug_print_gdb_thread_options (gdb_thread_options options)
{ return to_string (options); }

static std::string
target_debug_print_target_waitstatus_p (struct target_waitstatus *status)
{ return status->to_string (); }

/* Functions that are used via TARGET_DEBUG_PRINTER.  */

static std::string
target_debug_print_step (int step)
{ return step ? "step" : "continue"; }

static std::string
target_debug_print_target_wait_flags (target_wait_flags options)
{ return target_options_to_string (options); }

static std::string
target_debug_print_signals (gdb::array_view<const unsigned char> sigs)
{
  std::string s = "{";

  for (size_t i = 0; i < sigs.size (); i++)
    if (sigs[i] != 0)
      string_appendf (s, " %s",
		      gdb_signal_to_name (static_cast<gdb_signal>(i)));

  s += " }";

  return s;
}

static std::string
target_debug_print_size_t (size_t size)
{
  return pulongest (size);
}

static std::string
target_debug_print_gdb_array_view_const_gdb_byte (gdb::array_view<const gdb_byte> vector)
{
  std::string s = "{";

  for (const auto b : vector)
    string_appendf (s, " %s", phex_nz (b, 1));

  s += " }";

  return s;
}

static std::string
target_debug_print_const_gdb_byte_vector_r (const gdb::byte_vector &vector)
{ return target_debug_print_gdb_array_view_const_gdb_byte (vector); }

static std::string
target_debug_print_gdb_byte_vector_r (gdb::byte_vector &vector)
{ return target_debug_print_const_gdb_byte_vector_r (vector); }

static std::string
target_debug_print_x86_xsave_layout (const x86_xsave_layout &layout)
{
  std::string s = string_printf ("{ sizeof_xsave=%d", layout.sizeof_xsave);

#define POFFS(region)						\
  if (layout.region##_offset != 0)				\
    string_appendf (s, ", " #region "_offset=%d", layout.region##_offset);

  POFFS(avx);
  POFFS(k);
  POFFS(zmm_h);
  POFFS(zmm);
  POFFS(pkru);

#undef POFFS

  s += " }";

  return s;
}
#endif /* GDB_TARGET_DEBUG_H */
