/* Definitions for frame unwinder, for GDB, the GNU debugger.

   Copyright (C) 2003-2024 Free Software Foundation, Inc.

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

#include "extract-store-integer.h"
#include "frame.h"
#include "frame-unwind.h"
#include "dummy-frame.h"
#include "inline-frame.h"
#include "value.h"
#include "regcache.h"
#include "gdbsupport/gdb_obstack.h"
#include "target.h"
#include "gdbarch.h"
#include "dwarf2/frame-tailcall.h"
#include "cli/cli-cmds.h"
#include "inferior.h"

/* Conversion list between the enum for frame_unwind_class and
   string.  */
static const char * unwind_class_conversion[] =
{
  "GDB",
  "EXTENSION",
  "DEBUGINFO",
  "ARCH",
};

/* Default sniffers, that must always be the first in the unwinder list,
   no matter the architecture.  */
static constexpr std::initializer_list<const frame_unwind *>
  standard_unwinders =
{
  &dummy_frame_unwind,
  /* The DWARF tailcall sniffer must come before the inline sniffer.
     Otherwise, we can end up in a situation where a DWARF frame finds
     tailcall information, but then the inline sniffer claims a frame
     before the tailcall sniffer, resulting in confusion.  This is
     safe to do always because the tailcall sniffer can only ever be
     activated if the newer frame was created using the DWARF
     unwinder, and it also found tailcall information.  */
  &dwarf2_tailcall_frame_unwind,
  &inline_frame_unwind,
};

/* If an unwinder should be prepended to the list, this is the
   index in which it should be inserted.  */
static constexpr int prepend_unwinder_index = standard_unwinders.size ();

static const registry<gdbarch>::key<std::vector<const frame_unwind *>>
     frame_unwind_data;

/* Retrieve the list of frame unwinders available in GDBARCH.
   If this list is empty, it is initialized before being returned.  */
static std::vector<const frame_unwind *> &
get_frame_unwind_table (struct gdbarch *gdbarch)
{
  std::vector<const frame_unwind *> *table = frame_unwind_data.get (gdbarch);
  if (table != nullptr)
    return *table;

  table = new std::vector<const frame_unwind *>;
  table->insert (table->begin (), standard_unwinders.begin (),
		 standard_unwinders.end ());

  frame_unwind_data.set (gdbarch, table);

  return *table;
}

static const char *
frame_unwinder_class_str (frame_unwind_class uclass)
{
  gdb_assert (uclass < UNWIND_CLASS_NUMBER);
  return unwind_class_conversion[uclass];
}

void
frame_unwind_prepend_unwinder (struct gdbarch *gdbarch,
				const struct frame_unwind *unwinder)
{
  std::vector<const frame_unwind *> &table = get_frame_unwind_table (gdbarch);

  table.insert (table.begin () + prepend_unwinder_index, unwinder);
}

void
frame_unwind_append_unwinder (struct gdbarch *gdbarch,
			      const struct frame_unwind *unwinder)
{
  get_frame_unwind_table (gdbarch).push_back (unwinder);
}

/* Call SNIFFER from UNWINDER.  If it succeeded set UNWINDER for
   THIS_FRAME and return true.  Otherwise the function keeps THIS_FRAME
   unchanged and returns false.  */

static bool
frame_unwind_try_unwinder (const frame_info_ptr &this_frame, void **this_cache,
			  const struct frame_unwind *unwinder)
{
  int res = 0;

  unsigned int entry_generation = get_frame_cache_generation ();

  frame_prepare_for_sniffer (this_frame, unwinder);

  try
    {
      frame_debug_printf ("trying unwinder \"%s\"", unwinder->name ());
      res = unwinder->sniff (this_frame, this_cache);
    }
  catch (const gdb_exception &ex)
    {
      frame_debug_printf ("caught exception: %s", ex.message->c_str ());

      /* Catch all exceptions, caused by either interrupt or error.
	 Reset *THIS_CACHE, unless something reinitialized the frame
	 cache meanwhile, in which case THIS_FRAME/THIS_CACHE are now
	 dangling.  */
      if (get_frame_cache_generation () == entry_generation)
	{
	  *this_cache = NULL;
	  frame_cleanup_after_sniffer (this_frame);
	}

      if (ex.error == NOT_AVAILABLE_ERROR)
	{
	  /* This usually means that not even the PC is available,
	     thus most unwinders aren't able to determine if they're
	     the best fit.  Keep trying.  Fallback prologue unwinders
	     should always accept the frame.  */
	  return false;
	}
      throw;
    }

  if (res)
    {
      frame_debug_printf ("yes");
      return true;
    }
  else
    {
      frame_debug_printf ("no");
      /* Don't set *THIS_CACHE to NULL here, because sniffer has to do
	 so.  */
      frame_cleanup_after_sniffer (this_frame);
      return false;
    }
  gdb_assert_not_reached ("frame_unwind_try_unwinder");
}

/* Iterate through sniffers for THIS_FRAME frame until one returns with an
   unwinder implementation.  THIS_FRAME->UNWIND must be NULL, it will get set
   by this function.  Possibly initialize THIS_CACHE.  */

void
frame_unwind_find_by_frame (const frame_info_ptr &this_frame, void **this_cache)
{
  FRAME_SCOPED_DEBUG_ENTER_EXIT;
  frame_debug_printf ("this_frame=%d", frame_relative_level (this_frame));

  const struct frame_unwind *unwinder_from_target;

  unwinder_from_target = target_get_unwinder ();
  if (unwinder_from_target != NULL
      && frame_unwind_try_unwinder (this_frame, this_cache,
				   unwinder_from_target))
    return;

  unwinder_from_target = target_get_tailcall_unwinder ();
  if (unwinder_from_target != NULL
      && frame_unwind_try_unwinder (this_frame, this_cache,
				   unwinder_from_target))
    return;

  struct gdbarch *gdbarch = get_frame_arch (this_frame);
  std::vector<const frame_unwind *> &table = get_frame_unwind_table (gdbarch);
  for (const auto &unwinder : table)
    if (frame_unwind_try_unwinder (this_frame, this_cache, unwinder))
      return;

  internal_error (_("frame_unwind_find_by_frame failed"));
}

/* A default frame sniffer which always accepts the frame.  Used by
   fallback prologue unwinders.  */

int
default_frame_sniffer (const struct frame_unwind *self,
		       const frame_info_ptr &this_frame,
		       void **this_prologue_cache)
{
  return 1;
}

/* The default frame unwinder stop_reason callback.  */

enum unwind_stop_reason
default_frame_unwind_stop_reason (const frame_info_ptr &this_frame,
				  void **this_cache)
{
  struct frame_id this_id = get_frame_id (this_frame);

  if (this_id == outer_frame_id)
    return UNWIND_OUTERMOST;
  else
    return UNWIND_NO_REASON;
}

/* See frame-unwind.h.  */

CORE_ADDR
default_unwind_pc (struct gdbarch *gdbarch, const frame_info_ptr &next_frame)
{
  int pc_regnum = gdbarch_pc_regnum (gdbarch);
  CORE_ADDR pc = frame_unwind_register_unsigned (next_frame, pc_regnum);
  pc = gdbarch_addr_bits_remove (gdbarch, pc);
  return pc;
}

/* See frame-unwind.h.  */

CORE_ADDR
default_unwind_sp (struct gdbarch *gdbarch, const frame_info_ptr &next_frame)
{
  int sp_regnum = gdbarch_sp_regnum (gdbarch);
  return frame_unwind_register_unsigned (next_frame, sp_regnum);
}

/* Helper functions for value-based register unwinding.  These return
   a (possibly lazy) value of the appropriate type.  */

/* Return a value which indicates that FRAME did not save REGNUM.  */

struct value *
frame_unwind_got_optimized (const frame_info_ptr &frame, int regnum)
{
  struct gdbarch *gdbarch = frame_unwind_arch (frame);
  struct type *type = register_type (gdbarch, regnum);

  return value::allocate_optimized_out (type);
}

/* Return a value which indicates that FRAME copied REGNUM into
   register NEW_REGNUM.  */

struct value *
frame_unwind_got_register (const frame_info_ptr &frame,
			   int regnum, int new_regnum)
{
  return value_of_register_lazy (get_next_frame_sentinel_okay (frame),
				 new_regnum);
}

/* Return a value which indicates that FRAME saved REGNUM in memory at
   ADDR.  */

struct value *
frame_unwind_got_memory (const frame_info_ptr &frame, int regnum, CORE_ADDR addr)
{
  struct gdbarch *gdbarch = frame_unwind_arch (frame);
  struct value *v = value_at_lazy (register_type (gdbarch, regnum), addr);

  v->set_stack (true);
  return v;
}

/* Return a value which indicates that FRAME's saved version of
   REGNUM has a known constant (computed) value of VAL.  */

struct value *
frame_unwind_got_constant (const frame_info_ptr &frame, int regnum,
			   ULONGEST val)
{
  struct gdbarch *gdbarch = frame_unwind_arch (frame);
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  struct value *reg_val;

  reg_val = value::zero (register_type (gdbarch, regnum), not_lval);
  store_unsigned_integer (reg_val->contents_writeable ().data (),
			  register_size (gdbarch, regnum), byte_order, val);
  return reg_val;
}

struct value *
frame_unwind_got_bytes (const frame_info_ptr &frame, int regnum,
			gdb::array_view<const gdb_byte> buf)
{
  struct gdbarch *gdbarch = frame_unwind_arch (frame);
  struct value *reg_val;

  reg_val = value::zero (register_type (gdbarch, regnum), not_lval);
  gdb::array_view<gdb_byte> val_contents = reg_val->contents_raw ();

  /* The value's contents buffer is zeroed on allocation so if buf is
     smaller, the remaining space will be filled with zero.

     This can happen when unwinding through signal frames.  For example, if
     an AArch64 program doesn't use SVE, then the Linux kernel will only
     save in the signal frame the first 128 bits of the vector registers,
     which is their minimum size, even if the vector length says they're
     bigger.  */
  gdb_assert (buf.size () <= val_contents.size ());

  memcpy (val_contents.data (), buf.data (), buf.size ());
  return reg_val;
}

/* Return a value which indicates that FRAME's saved version of REGNUM
   has a known constant (computed) value of ADDR.  Convert the
   CORE_ADDR to a target address if necessary.  */

struct value *
frame_unwind_got_address (const frame_info_ptr &frame, int regnum,
			  CORE_ADDR addr)
{
  struct gdbarch *gdbarch = frame_unwind_arch (frame);
  struct value *reg_val;

  reg_val = value::zero (register_type (gdbarch, regnum), not_lval);
  pack_long (reg_val->contents_writeable ().data (),
	     register_type (gdbarch, regnum), addr);
  return reg_val;
}

/* See frame-unwind.h.  */

enum unwind_stop_reason
frame_unwind_legacy::stop_reason (const frame_info_ptr &this_frame,
				  void **this_prologue_cache) const
{
  return m_stop_reason (this_frame, this_prologue_cache);
}

/* See frame-unwind.h.  */

void
frame_unwind_legacy::this_id (const frame_info_ptr &this_frame,
			      void **this_prologue_cache,
			      struct frame_id *id) const
{
  return m_this_id (this_frame, this_prologue_cache, id);
}

/* See frame-unwind.h.  */

struct value *
frame_unwind_legacy::prev_register (const frame_info_ptr &this_frame,
				    void **this_prologue_cache,
				    int regnum) const
{
  return m_prev_register (this_frame, this_prologue_cache, regnum);
}

/* See frame-unwind.h.  */

int
frame_unwind_legacy::sniff (const frame_info_ptr &this_frame,
			    void **this_prologue_cache) const
{
  return m_sniffer (this, this_frame, this_prologue_cache);
}

/* See frame-unwind.h.  */

void
frame_unwind_legacy::dealloc_cache (frame_info *self, void *this_cache) const
{
  if (m_dealloc_cache != nullptr)
    m_dealloc_cache (self, this_cache);
}

/* See frame-unwind.h.  */

struct gdbarch *
frame_unwind_legacy::prev_arch (const frame_info_ptr &this_frame,
				void **this_prologue_cache) const
{
  if (m_prev_arch == nullptr)
    return frame_unwind::prev_arch (this_frame, this_prologue_cache);
  return m_prev_arch (this_frame, this_prologue_cache);
}

/* Implement "maintenance info frame-unwinders" command.  */

static void
maintenance_info_frame_unwinders (const char *args, int from_tty)
{
  gdbarch *gdbarch = current_inferior ()->arch ();
  std::vector<const frame_unwind *> &table = get_frame_unwind_table (gdbarch);

  ui_out *uiout = current_uiout;
  ui_out_emit_table table_emitter (uiout, 3, -1, "FrameUnwinders");
  uiout->table_header (27, ui_left, "name", "Name");
  uiout->table_header (25, ui_left, "type", "Type");
  uiout->table_header (9, ui_left, "class", "Class");
  uiout->table_body ();

  for (const auto &unwinder : table)
    {
      ui_out_emit_list tuple_emitter (uiout, nullptr);
      uiout->field_string ("name", unwinder->name ());
      uiout->field_string ("type", frame_type_str (unwinder->type ()));
      uiout->field_string ("class", frame_unwinder_class_str (
					unwinder->unwinder_class ()));
      uiout->text ("\n");
    }
}

void _initialize_frame_unwind ();
void
_initialize_frame_unwind ()
{
  /* Add "maint info frame-unwinders".  */
  add_cmd ("frame-unwinders",
	   class_maintenance,
	   maintenance_info_frame_unwinders,
	   _("\
List the frame unwinders currently in effect.\n\
Unwinders are listed starting with the highest priority."),
	   &maintenanceinfolist);
}
