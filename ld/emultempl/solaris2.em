# This shell script emits a C file. -*- C -*-
# It does some substitutions.
if [ -z "$MACHINE" ]; then
  OUTPUT_ARCH=${ARCH}
else
  OUTPUT_ARCH=${ARCH}:${MACHINE}
fi
fragment <<EOF
/* This file is generated by a shell script.  DO NOT EDIT! */

/* Solaris 2 emulation code for ${EMULATION_NAME}
   Copyright (C) 2010-2025 Free Software Foundation, Inc.
   Written by Rainer Orth <ro@CeBiTec.Uni-Bielefeld.DE>

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#define TARGET_IS_${EMULATION_NAME}

/* The Solaris 2 ABI requires some global symbols to be present in the
   .dynsym table of executables and shared objects.  If generating a
   versioned shared object, they must always be bound to the base version.

   The Solaris 2 ABI also requires two local symbols to be emitted for
   every executable and shared object.

   Cf. Linker and Libraries Guide, Ch. 2, Link-Editor, Generating the Output
   File, p.63.  */

static void
elf_solaris2_before_allocation (void)
{
  /* Global symbols required by the Solaris 2 ABI.  */
  static const char *global_syms[] = {
    "_DYNAMIC",
    "_GLOBAL_OFFSET_TABLE_",
    "_PROCEDURE_LINKAGE_TABLE_",
    "_edata",
    "_end",
    "_etext",
    NULL
  };
  /* Local symbols required by the Solaris 2 ABI.  Already emitted by
     emulparams/solaris2.sh.  */
  static const char *local_syms[] = {
    "_START_",
    "_END_",
    NULL
  };
  const char **sym;

  /* Do this for both executables and shared objects.  */
  if (!bfd_link_relocatable (&link_info)
      && is_elf_hash_table (link_info.hash))
    {
      for (sym = global_syms; *sym != NULL; sym++)
	{
	  struct elf_link_hash_entry *h;

	  /* Lookup symbol.  */
	  h = elf_link_hash_lookup (elf_hash_table (&link_info), *sym,
				    false, false, false);
	  if (h == NULL)
	    continue;

	  /* Undo the hiding done by _bfd_elf_define_linkage_sym.  */
	  h->forced_local = 0;
	  h->other &= ~STV_HIDDEN;

	  /* Emit it into the .dynamic section, too.  */
	  bfd_elf_link_record_dynamic_symbol (&link_info, h);
	}

      for (sym = local_syms; *sym != NULL; sym++)
	{
	  struct elf_link_hash_entry *h;

	  /* Lookup symbol.  */
	  h = elf_link_hash_lookup (elf_hash_table (&link_info), *sym,
				    false, false, false);
	  if (h == NULL)
	    continue;

	  /* Turn it local.  */
	  h->forced_local = 1;
	  /* Type should be STT_OBJECT, not STT_NOTYPE.  */
	  h->type = STT_OBJECT;
	}
    }

  /* Only do this if emitting a shared object and versioning is in place. */
  if (bfd_link_dll (&link_info)
      && ((link_info.version_info != NULL
	   && link_info.version_info->name[0] != '\0')
	  || link_info.create_default_symver))
    {
      struct bfd_elf_version_expr *globals = NULL, *locals = NULL;
      struct bfd_elf_version_tree *basever;
      const char *soname;

      for (sym = global_syms; *sym != NULL; sym++)
	{
	  /* Create a version pattern for this symbol.  Some of them start
	     off as local, others as global, so try both.  */
	  globals = lang_new_vers_pattern (globals, *sym, NULL, true);
	  locals = lang_new_vers_pattern (locals, *sym, NULL, true);
	}

      /* New version node for those symbols.  */
      basever = lang_new_vers_node (globals, locals);

      /* The version name matches what bfd_elf_size_dynamic_sections uses
	 for the base version.  */
      soname = bfd_elf_get_dt_soname (link_info.output_bfd);
      if (soname == NULL)
	soname = lbasename (bfd_get_filename (link_info.output_bfd));

      /* Register the node.  */
      lang_register_vers_node (soname, basever, NULL);
      /* Enforce base version.  The encoded vd_ndx is vernum + 1.  */
      basever->vernum = 0;
    }

  gld${EMULATION_NAME}_before_allocation ();
}

EOF

LDEMUL_BEFORE_ALLOCATION=elf_solaris2_before_allocation
