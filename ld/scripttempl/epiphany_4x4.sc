# Copyright (C) 2014-2025 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
#
# Unusual variables checked by this code:
#	NOP - four byte opcode for no-op (defaults to 0)
#	NO_SMALL_DATA - no .sbss/.sbss2/.sdata/.sdata2 sections if not
#		empty.
#	SMALL_DATA_CTOR - .ctors contains small data.
#	SMALL_DATA_DTOR - .dtors contains small data.
#	DATA_ADDR - if end-of-text-plus-one-page isn't right for data start
#	INITIAL_READONLY_SECTIONS - at start of text segment
#	OTHER_READONLY_SECTIONS - other than .text .init .rodata ...
#		(e.g., .PARISC.milli)
#	OTHER_TEXT_SECTIONS - these get put in .text when relocating
#	OTHER_READWRITE_SECTIONS - other than .data .bss .ctors .sdata ...
#		(e.g., .PARISC.global)
#	OTHER_RELRO_SECTIONS - other than .data.rel.ro ...
#		(e.g. PPC32 .fixup, .got[12])
#	OTHER_BSS_SECTIONS - other than .bss .sbss ...
#	ATTRS_SECTIONS - at the end
#	OTHER_SECTIONS - at the end
#	EXECUTABLE_SYMBOLS - symbols that must be defined for an
#		executable (e.g., _DYNAMIC_LINK)
#       TEXT_START_ADDR - the first byte of the text segment, after any
#               headers.
#       TEXT_BASE_ADDRESS - the first byte of the text segment.
#	TEXT_START_SYMBOLS - symbols that appear at the start of the
#		.text section.
#	DATA_START_SYMBOLS - symbols that appear at the start of the
#		.data section.
#	DATA_END_SYMBOLS - symbols that appear at the end of the
#		writeable data sections.
#	OTHER_GOT_SYMBOLS - symbols defined just before .got.
#	OTHER_GOT_SECTIONS - sections just after .got.
#	OTHER_SDATA_SECTIONS - sections just after .sdata.
#	OTHER_BSS_SYMBOLS - symbols that appear at the start of the
#		.bss section besides ___bss_start.
#	DATA_PLT - .plt should be in data segment, not text segment.
#	PLT_BEFORE_GOT - .plt just before .got when .plt is in data segement.
#	BSS_PLT - .plt should be in bss segment
#	NO_REL_RELOCS - Don't include .rel.* sections in script
#	NO_RELA_RELOCS - Don't include .rela.* sections in script
#	NON_ALLOC_DYN - Place dynamic sections after data segment.
#	TEXT_DYNAMIC - .dynamic in text segment, not data segment.
#	EMBEDDED - whether this is for an embedded system.
#	SHLIB_TEXT_START_ADDR - if set, add to SIZEOF_HEADERS to set
#		start address of shared library.
#	INPUT_FILES - INPUT command of files to always include
#	WRITABLE_RODATA - if set, the .rodata section should be writable
#	INIT_START, INIT_END -  statements just before and just after
#	combination of .init sections.
#	FINI_START, FINI_END - statements just before and just after
#	combination of .fini sections.
#	OTHER_SYMBOLS - symbols to place right at the end of the script.
#	ETEXT_NAME - name of a symbol for the end of the text section,
#		normally etext.
#	SEPARATE_GOTPLT - if set, .got.plt should be separate output section,
#		so that .got can be in the RELRO area.  It should be set to
#		the number of bytes in the beginning of .got.plt which can be
#		in the RELRO area as well.
#	USER_LABEL_PREFIX - prefix to add to user-visible symbols.
#
# When adding sections, do note that the names of some sections are used
# when specifying the start address of the next.
#

#  Many sections come in three flavours.  There is the 'real' section,
#  like ".data".  Then there are the per-procedure or per-variable
#  sections, generated by -ffunction-sections and -fdata-sections in GCC,
#  and useful for --gc-sections, which for a variable "foo" might be
#  ".data.foo".  Then there are the linkonce sections, for which the linker
#  eliminates duplicates, which are named like ".gnu.linkonce.d.foo".
#  The exact correspondences are:
#
#  Section	Linkonce section
#  .text	.gnu.linkonce.t.foo
#  .rodata	.gnu.linkonce.r.foo
#  .data	.gnu.linkonce.d.foo
#  .bss		.gnu.linkonce.b.foo
#  .sdata	.gnu.linkonce.s.foo
#  .sbss	.gnu.linkonce.sb.foo
#  .sdata2	.gnu.linkonce.s2.foo
#  .sbss2	.gnu.linkonce.sb2.foo
#  .debug_info	.gnu.linkonce.wi.foo
#  .tdata	.gnu.linkonce.td.foo
#  .tbss	.gnu.linkonce.tb.foo
#  .lrodata	.gnu.linkonce.lr.foo
#  .ldata	.gnu.linkonce.l.foo
#  .lbss	.gnu.linkonce.lb.foo
#
#  Each of these can also have corresponding .rel.* and .rela.* sections.


test -z "$ENTRY" && ENTRY=_start
test -z "${BIG_OUTPUT_FORMAT}" && BIG_OUTPUT_FORMAT=${OUTPUT_FORMAT}
test -z "${LITTLE_OUTPUT_FORMAT}" && LITTLE_OUTPUT_FORMAT=${OUTPUT_FORMAT}
if [ -z "$MACHINE" ]; then OUTPUT_ARCH=${ARCH}; else OUTPUT_ARCH=${ARCH}:${MACHINE}; fi
test -z "${ELFSIZE}" && ELFSIZE=32
test -z "${ALIGNMENT}" && ALIGNMENT="${ELFSIZE} / 8"
test "$LD_FLAG" = "N" && DATA_ADDR=.
test -z "${ETEXT_NAME}" && ETEXT_NAME=etext
test -n "$CREATE_SHLIB$CREATE_PIE" && test -n "$SHLIB_DATA_ADDR" && COMMONPAGESIZE=""
test -z "$CREATE_SHLIB$CREATE_PIE" && test -n "$DATA_ADDR" && COMMONPAGESIZE=""
test -n "$RELRO_NOW" && unset SEPARATE_GOTPLT
test -z "$ATTRS_SECTIONS" && ATTRS_SECTIONS=".gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }"
DATA_SEGMENT_ALIGN="ALIGN(${SEGMENT_SIZE}) + (. & (${MAXPAGESIZE} - 1))"
DATA_SEGMENT_RELRO_END=""
DATA_SEGMENT_END=""
if test -n "${COMMONPAGESIZE}"; then
  DATA_SEGMENT_ALIGN="ALIGN (${SEGMENT_SIZE}) - ((${MAXPAGESIZE} - .) & (${MAXPAGESIZE} - 1)); . = DATA_SEGMENT_ALIGN (${MAXPAGESIZE}, ${COMMONPAGESIZE})"
  DATA_SEGMENT_END=". = DATA_SEGMENT_END (.);"
  DATA_SEGMENT_RELRO_END=". = DATA_SEGMENT_RELRO_END (${SEPARATE_GOTPLT-0}, .);"
fi
if test -z "${INITIAL_READONLY_SECTIONS}${CREATE_SHLIB}"; then
  INITIAL_READONLY_SECTIONS=".interp       ${RELOCATING-0} : { *(.interp) }"
fi
if test -z "$PLT"; then
  PLT=".plt          ${RELOCATING-0} : { *(.plt) }"
fi
test -n "${DATA_PLT-${BSS_PLT-text}}" && TEXT_PLT=yes
if test -z "$GOT"; then
  if test -z "$SEPARATE_GOTPLT"; then
    GOT=".got          ${RELOCATING-0} : {${RELOCATING+ *(.got.plt)} *(.got) }"
  else
    GOT=".got          ${RELOCATING-0} : { *(.got) }"
    GOTPLT=".got.plt      ${RELOCATING-0} : { *(.got.plt) }"
  fi
fi
DYNAMIC=".dynamic      ${RELOCATING-0} : { *(.dynamic) }"
RODATA=".rodata ${RELOCATING+ADDR(.data)+SIZEOF(.data)} ${RELOCATING-0} : { *(.rodata${RELOCATING+ .rodata.* .gnu.linkonce.r.*}) }  /*> INTERNAL_RAM*/"
DATARELRO=".data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }"
DISCARDED="/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) }"
if test -z "${NO_SMALL_DATA}"; then
  SBSS=".sbss         ${RELOCATING-0} :
  {
    ${RELOCATING+${SBSS_START_SYMBOLS}}
    ${CREATE_SHLIB+*(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)}
    ${RELOCATING+*(.dynsbss)}
    *(.sbss${RELOCATING+ .sbss.* .gnu.linkonce.sb.*})
    ${RELOCATING+*(.scommon)}
    ${RELOCATING+${SBSS_END_SYMBOLS}}
  }"
  SBSS2=".sbss2        ${RELOCATING-0} : { *(.sbss2${RELOCATING+ .sbss2.* .gnu.linkonce.sb2.*}) }"
  SDATA="/* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata        ${RELOCATING-0} :
  {
    ${RELOCATING+${SDATA_START_SYMBOLS}}
    ${RELOCATING+${CREATE_SHLIB+*(.sdata2 .sdata2.* .gnu.linkonce.s2.*)}}
    *(.sdata${RELOCATING+ .sdata.* .gnu.linkonce.s.*})
  }"
  SDATA2=".sdata2       ${RELOCATING-0} :
  {
    ${RELOCATING+${SDATA2_START_SYMBOLS}}
    *(.sdata2${RELOCATING+ .sdata2.* .gnu.linkonce.s2.*})
  }"
  REL_SDATA=".rel.sdata    ${RELOCATING-0} : { *(.rel.sdata${RELOCATING+ .rel.sdata.* .rel.gnu.linkonce.s.*}) }
  .rela.sdata   ${RELOCATING-0} : { *(.rela.sdata${RELOCATING+ .rela.sdata.* .rela.gnu.linkonce.s.*}) }"
  REL_SBSS=".rel.sbss     ${RELOCATING-0} : { *(.rel.sbss${RELOCATING+ .rel.sbss.* .rel.gnu.linkonce.sb.*}) }
  .rela.sbss    ${RELOCATING-0} : { *(.rela.sbss${RELOCATING+ .rela.sbss.* .rela.gnu.linkonce.sb.*}) }"
  REL_SDATA2=".rel.sdata2   ${RELOCATING-0} : { *(.rel.sdata2${RELOCATING+ .rel.sdata2.* .rel.gnu.linkonce.s2.*}) }
  .rela.sdata2  ${RELOCATING-0} : { *(.rela.sdata2${RELOCATING+ .rela.sdata2.* .rela.gnu.linkonce.s2.*}) }"
  REL_SBSS2=".rel.sbss2    ${RELOCATING-0} : { *(.rel.sbss2${RELOCATING+ .rel.sbss2.* .rel.gnu.linkonce.sb2.*}) }
  .rela.sbss2   ${RELOCATING-0} : { *(.rela.sbss2${RELOCATING+ .rela.sbss2.* .rela.gnu.linkonce.sb2.*}) }"
else
  NO_SMALL_DATA=" "
fi
if test -z "${DATA_GOT}"; then
  if test -n "${NO_SMALL_DATA}"; then
    DATA_GOT=" "
  fi
fi
if test -z "${SDATA_GOT}"; then
  if test -z "${NO_SMALL_DATA}"; then
    SDATA_GOT=" "
  fi
fi
test -n "$SEPARATE_GOTPLT" && SEPARATE_GOTPLT=" "
test "${LARGE_SECTIONS}" = "yes" && REL_LARGE="
  .rel.ldata    ${RELOCATING-0} : { *(.rel.ldata${RELOCATING+ .rel.ldata.* .rel.gnu.linkonce.l.*}) }
  .rela.ldata   ${RELOCATING-0} : { *(.rela.ldata${RELOCATING+ .rela.ldata.* .rela.gnu.linkonce.l.*}) }
  .rel.lbss     ${RELOCATING-0} : { *(.rel.lbss${RELOCATING+ .rel.lbss.* .rel.gnu.linkonce.lb.*}) }
  .rela.lbss    ${RELOCATING-0} : { *(.rela.lbss${RELOCATING+ .rela.lbss.* .rela.gnu.linkonce.lb.*}) }
  .rel.lrodata  ${RELOCATING-0} : { *(.rel.lrodata${RELOCATING+ .rel.lrodata.* .rel.gnu.linkonce.lr.*}) }
  .rela.lrodata ${RELOCATING-0} : { *(.rela.lrodata${RELOCATING+ .rela.lrodata.* .rela.gnu.linkonce.lr.*}) }"
test "${LARGE_SECTIONS}" = "yes" && OTHER_BSS_SECTIONS="
  ${OTHER_BSS_SECTIONS}
  .lbss ${RELOCATING-0} :
  {
    ${RELOCATING+*(.dynlbss)}
    *(.lbss${RELOCATING+ .lbss.* .gnu.linkonce.lb.*})
    ${RELOCATING+*(LARGE_COMMON)}
  }"
test "${LARGE_SECTIONS}" = "yes" && LARGE_SECTIONS="
  .lrodata ${RELOCATING-0} ${RELOCATING+ALIGN(${MAXPAGESIZE}) + (. & (${MAXPAGESIZE} - 1))} :
  {
    *(.lrodata${RELOCATING+ .lrodata.* .gnu.linkonce.lr.*})
  }
  .ldata ${RELOCATING-0} ${RELOCATING+ALIGN(${MAXPAGESIZE}) + (. & (${MAXPAGESIZE} - 1))} :
  {
    *(.ldata${RELOCATING+ .ldata.* .gnu.linkonce.l.*})
    ${RELOCATING+. = ALIGN(. != 0 ? ${ALIGNMENT} : 1);}
  }"
CTOR=".ctors    ADDR(.text) + SIZEOF(.text)      ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+${CTOR_START}}
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */

    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))

    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */

    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o $OTHER_EXCLUDE_FILES) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ${CONSTRUCTING+${CTOR_END}}
  }  /*> INTERNAL_RAM*/"
DTOR=".dtors     ADDR(.ctors) + SIZEOF(.ctors)    ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+${DTOR_START}}
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o $OTHER_EXCLUDE_FILES) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ${CONSTRUCTING+${DTOR_END}}
  }   /*> INTERNAL_RAM*/ "

# If this is for an embedded system, don't add SIZEOF_HEADERS.
if [ -z "$EMBEDDED" ]; then
   test -z "${TEXT_BASE_ADDRESS}" && TEXT_BASE_ADDRESS="${TEXT_START_ADDR} + SIZEOF_HEADERS"
else
   test -z "${TEXT_BASE_ADDRESS}" && TEXT_BASE_ADDRESS="${TEXT_START_ADDR}"
fi

cat <<EOF
/* Copyright (C) 2014-2025 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}", "${BIG_OUTPUT_FORMAT}",
	      "${LITTLE_OUTPUT_FORMAT}")
OUTPUT_ARCH(${OUTPUT_ARCH})
EOF

test -n "${RELOCATING}" && cat <<EOF
ENTRY(${ENTRY})

${EXECUTABLE_SYMBOLS}
${INPUT_FILES}

/* BSP specific*/
__PROG_SIZE_FOR_CORE__ = 1M;
__HEAP_SIZE_FOR_CORE__ = 1M;

__MAX_NUM_CORES_IN_ROWS__ = 4;
__MAX_NUM_CORES_IN_COLS__ = 4;

__FIRST_CORE_ROW_ = 0x20;
__FIRST_CORE_COL_ = 0x24;



PROVIDE (__CORE_ROW_ = __FIRST_CORE_ROW_);
PROVIDE (__CORE_COL_ = __FIRST_CORE_COL_);
/* generic do not touch */
/* used to calculated the slice address in the external memory */
__CORE_NUM_ =  (__CORE_ROW_ -  __FIRST_CORE_ROW_ )* __MAX_NUM_CORES_IN_COLS__ + (__CORE_COL_ - __FIRST_CORE_COL_ ) ;


MEMORY
 {
	EXTERNAL_DRAM_0 (WXAI) :     ORIGIN = 0x80000000,      LENGTH = 0x1000000  /*.text, data, rodata, bss and .stack*/
    EXTERNAL_DRAM_1 (WXAI) :        ORIGIN = 0x81000000,      LENGTH = 0x1000000 /*.heap */

    EXTERNAL_SRAM (WXAI) :       ORIGIN = 0x92000000,      LENGTH =   8K /* small external RAM, used for testing*/

    /* run time lib and crt0*/
    RESERVED_CRT0_RAM (WXAI) :    ORIGIN = 0,      LENGTH = 0x400

    /* user program, per bank usage */
    BANK0_SRAM (WXAI) : ORIGIN = LENGTH(RESERVED_CRT0_RAM),   LENGTH = 8K - LENGTH(RESERVED_CRT0_RAM)
    BANK1_SRAM (WXAI) : ORIGIN = 0x2000, LENGTH = 8K
    BANK2_SRAM (WXAI) : ORIGIN = 0x4000, LENGTH = 8K
    BANK3_SRAM (WXAI) : ORIGIN = 0x6000, LENGTH = 8K

    /* user program, continious placement */
    INTERNAL_RAM        (WXAI) :     ORIGIN = LENGTH(RESERVED_CRT0_RAM),  LENGTH = 32K - LENGTH(RESERVED_CRT0_RAM)

    MMR (WAI)	      : ORIGIN = 0xF000, LENGTH = 32K

    /* multi cores space */
	CORE_0x20_0x24_INTERNAL_RAM :      ORIGIN = 0x82400000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x20_0x25_INTERNAL_RAM :      ORIGIN = 0x82500000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x20_0x26_INTERNAL_RAM :      ORIGIN = 0x82600000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x20_0x27_INTERNAL_RAM :      ORIGIN = 0x82700000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x21_0x24_INTERNAL_RAM :      ORIGIN = 0x86400000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x21_0x25_INTERNAL_RAM :      ORIGIN = 0x86500000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x21_0x26_INTERNAL_RAM :      ORIGIN = 0x86600000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x21_0x27_INTERNAL_RAM :      ORIGIN = 0x86700000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x22_0x24_INTERNAL_RAM :      ORIGIN = 0x8a400000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x22_0x25_INTERNAL_RAM :      ORIGIN = 0x8a500000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x22_0x26_INTERNAL_RAM :      ORIGIN = 0x8a600000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x22_0x27_INTERNAL_RAM :      ORIGIN = 0x8a700000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x23_0x24_INTERNAL_RAM :      ORIGIN = 0x8e400000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x23_0x25_INTERNAL_RAM :      ORIGIN = 0x8e500000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x23_0x26_INTERNAL_RAM :      ORIGIN = 0x8e600000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x23_0x27_INTERNAL_RAM :      ORIGIN = 0x8e700000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)
	CORE_0x24_0x24_INTERNAL_RAM :      ORIGIN = 0x82000000+LENGTH(RESERVED_CRT0_RAM),      LENGTH = 32K- LENGTH(RESERVED_CRT0_RAM)

 }

EOF

cat <<EOF
SECTIONS
{
EOF

test -n "${RELOCATING}" && cat <<EOF
   IVT 0 : {*.o(IVT)  } > RESERVED_CRT0_RAM
   RESERVED_CRT0 : {*.o(RESERVED_CRT0)  } > RESERVED_CRT0_RAM
   RESERVED_CRT0 : {*.o(reserved_crt0)  } > RESERVED_CRT0_RAM

   CORE_RAM_0 :   {*.o(core_ram_0)  }  > BANK0_SRAM
   CORE_RAM_1 :   {*.o(core_ram_1)  }  > BANK1_SRAM
   CORE_RAM_2 :   {*.o(core_ram_2)  }  > BANK2_SRAM
   CORE_RAM_3 :   {*.o(core_ram_3)  }  > BANK3_SRAM

   SRAM_SOUTH :  {*.o(sram)  }  > EXTERNAL_SRAM
   DRAM_WEST :  {*.o(dram)  }  > EXTERNAL_DRAM_1

   CORE_INTERNAL :  {*.o(core_ram_internal)  }    /*> INTERNAL_RAM*/

   /* the newlib  (libc and libm)  library is maped to the dedicated section */

   __new_lib_start_external_ =  ( ORIGIN(EXTERNAL_DRAM_0) + __PROG_SIZE_FOR_CORE__ *__CORE_NUM_ );
   __new_lib_start_ = DEFINED(__USE_INTERNAL_MEM_FOR_NEW_LIB_) ? ORIGIN(BANK1_SRAM) :  __new_lib_start_external_ ;

   NEW_LIB_RO __new_lib_start_ : { lib_a-*.o(.text .rodata) *.o(libgloss_epiphany) } /* > INTERNAL_RAM */
   GNU_C_BUILTIN_LIB_RO     ADDR(NEW_LIB_RO) + SIZEOF(NEW_LIB_RO) : {
								*mulsi3.o(.text  .rodata)  *modsi3.o(.text  .rodata)
								*divsi3.o(.text  .rodata)	*udivsi3.o(.text  .rodata)
							    *umodsi3.o(.text  .rodata)   _*.o(.text  .rodata)
   }

   NEW_LIB_WR   ADDR(GNU_C_BUILTIN_LIB_RO) + SIZEOF(GNU_C_BUILTIN_LIB_RO)  : { lib_a-*.o(.data ) }    /* >  INTERNAL_RAM*/


   __init_start = DEFINED(__USE_INTERNAL_MEM_) ? ORIGIN(BANK1_SRAM) :  (ADDR(NEW_LIB_WR) + SIZEOF(NEW_LIB_WR) ) ;
   __init_start = DEFINED(__USE_INTERNAL_MEM_FOR_NEW_LIB_) ? ADDR(NEW_LIB_WR) + SIZEOF(NEW_LIB_WR)  : __init_start;
EOF

cat <<EOF
  /* Read-only sections, merged into text segment: */
  /*${CREATE_SHLIB-${CREATE_PIE-${RELOCATING+PROVIDE (__executable_start = ${TEXT_START_ADDR}); . = ${TEXT_BASE_ADDRESS};}}}*/
  ${CREATE_SHLIB+${RELOCATING+. = ${SHLIB_TEXT_START_ADDR:-0} + SIZEOF_HEADERS;}}
  ${CREATE_PIE+${RELOCATING+. = ${SHLIB_TEXT_START_ADDR:-0} + SIZEOF_HEADERS;}}
  ${INITIAL_READONLY_SECTIONS}
  .note.gnu.build-id ${RELOCATING-0}: { *(.note.gnu.build-id) }
EOF

test -n "${RELOCATING+0}" || unset NON_ALLOC_DYN
test -z "${NON_ALLOC_DYN}" || TEXT_DYNAMIC=
cat > ldscripts/dyntmp.$$ <<EOF
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash         ${RELOCATING-0} : { *(.hash) }
  .gnu.hash     ${RELOCATING-0} : { *(.gnu.hash) }
  .dynsym       ${RELOCATING-0} : { *(.dynsym) }
  .dynstr       ${RELOCATING-0} : { *(.dynstr) }
  .gnu.version  ${RELOCATING-0} : { *(.gnu.version) }
  .gnu.version_d ${RELOCATING-0}: { *(.gnu.version_d) }
  .gnu.version_r ${RELOCATING-0}: { *(.gnu.version_r) }
EOF

if [ "x$COMBRELOC" = x ]; then
  COMBRELOCCAT="cat >> ldscripts/dyntmp.$$"
else
  COMBRELOCCAT="cat > $COMBRELOC"
fi
eval $COMBRELOCCAT <<EOF
  .rel.init     ${RELOCATING-0} : { *(.rel.init) }
  .rela.init    ${RELOCATING-0} : { *(.rela.init) }
  .rel.text     ${RELOCATING-0} : { *(.rel.text${RELOCATING+ .rel.text.* .rel.gnu.linkonce.t.*}) }
  .rela.text    ${RELOCATING-0} : { *(.rela.text${RELOCATING+ .rela.text.* .rela.gnu.linkonce.t.*}) }
  .rel.fini     ${RELOCATING-0} : { *(.rel.fini) }
  .rela.fini    ${RELOCATING-0} : { *(.rela.fini) }
  .rel.rodata   ${RELOCATING-0} : { *(.rel.rodata${RELOCATING+ .rel.rodata.* .rel.gnu.linkonce.r.*}) }
  .rela.rodata  ${RELOCATING-0} : { *(.rela.rodata${RELOCATING+ .rela.rodata.* .rela.gnu.linkonce.r.*}) }
  ${OTHER_READONLY_RELOC_SECTIONS}
  .rel.data.rel.ro ${RELOCATING-0} : { *(.rel.data.rel.ro${RELOCATING+* .rel.gnu.linkonce.d.rel.ro.*}) }
  .rela.data.rel.ro ${RELOCATING-0} : { *(.rela.data.rel.ro${RELOCATING+* .rela.gnu.linkonce.d.rel.ro.*}) }
  .rel.data     ${RELOCATING-0} : { *(.rel.data${RELOCATING+ .rel.data.* .rel.gnu.linkonce.d.*}) }
  .rela.data    ${RELOCATING-0} : { *(.rela.data${RELOCATING+ .rela.data.* .rela.gnu.linkonce.d.*}) }
  .rel.tdata	${RELOCATING-0} : { *(.rel.tdata${RELOCATING+ .rel.tdata.* .rel.gnu.linkonce.td.*}) }
  .rela.tdata	${RELOCATING-0} : { *(.rela.tdata${RELOCATING+ .rela.tdata.* .rela.gnu.linkonce.td.*}) }
  .rel.tbss	${RELOCATING-0} : { *(.rel.tbss${RELOCATING+ .rel.tbss.* .rel.gnu.linkonce.tb.*}) }
  .rela.tbss	${RELOCATING-0} : { *(.rela.tbss${RELOCATING+ .rela.tbss.* .rela.gnu.linkonce.tb.*}) }
  .rel.ctors    ${RELOCATING-0} : { *(.rel.ctors) }
  .rela.ctors   ${RELOCATING-0} : { *(.rela.ctors) }
  .rel.dtors    ${RELOCATING-0} : { *(.rel.dtors) }
  .rela.dtors   ${RELOCATING-0} : { *(.rela.dtors) }
  .rel.got      ${RELOCATING-0} : { *(.rel.got) }
  .rela.got     ${RELOCATING-0} : { *(.rela.got) }
  ${OTHER_GOT_RELOC_SECTIONS}
  ${REL_SDATA}
  ${REL_SBSS}
  ${REL_SDATA2}
  ${REL_SBSS2}
  .rel.bss      ${RELOCATING-0} : { *(.rel.bss${RELOCATING+ .rel.bss.* .rel.gnu.linkonce.b.*}) }
  .rela.bss     ${RELOCATING-0} : { *(.rela.bss${RELOCATING+ .rela.bss.* .rela.gnu.linkonce.b.*}) }
  ${REL_LARGE}
EOF

if [ -n "$COMBRELOC" ]; then
cat >> ldscripts/dyntmp.$$ <<EOF
  .rel.dyn      ${RELOCATING-0} :
    {
EOF
sed -e '/^[	 ]*[{}][	 ]*$/d;/:[	 ]*$/d;/\.rela\./d;s/^.*: { *\(.*\)}$/      \1/' $COMBRELOC >> ldscripts/dyntmp.$$
cat >> ldscripts/dyntmp.$$ <<EOF
    }
  .rela.dyn     ${RELOCATING-0} :
    {
EOF
sed -e '/^[	 ]*[{}][	 ]*$/d;/:[	 ]*$/d;/\.rel\./d;s/^.*: { *\(.*\)}/      \1/' $COMBRELOC >> ldscripts/dyntmp.$$
cat >> ldscripts/dyntmp.$$ <<EOF
    }
EOF
fi

cat >> ldscripts/dyntmp.$$ <<EOF
  .rel.plt      ${RELOCATING-0} : { *(.rel.plt) }
  .rela.plt     ${RELOCATING-0} : { *(.rela.plt) }
  ${OTHER_PLT_RELOC_SECTIONS}
EOF

if test -z "${NON_ALLOC_DYN}"; then
  if test -z "${NO_REL_RELOCS}${NO_RELA_RELOCS}"; then
    cat ldscripts/dyntmp.$$
  else
    if test -z "${NO_REL_RELOCS}"; then
      sed -e '/^[	 ]*\.rela\.[^}]*$/,/}/d' -e '/^[	 ]*\.rela\./d' ldscripts/dyntmp.$$
    fi
    if test -z "${NO_RELA_RELOCS}"; then
      sed -e '/^[	 ]*\.rel\.[^}]*$/,/}/d' -e '/^[	 ]*\.rel\./d' ldscripts/dyntmp.$$
    fi
  fi
  rm -f ldscripts/dyntmp.$$
fi

cat <<EOF

  .init  ${RELOCATING-0}${RELOCATING+__init_start}  :
  {
    ${RELOCATING+${INIT_START}}
    KEEP (*(SORT_NONE(.init)))
    ${RELOCATING+${INIT_END}}
  } /* ${RELOCATING+ > INTERNAL_RAM} */ =${NOP-0}

  ${TEXT_PLT+${PLT}}
  ${TINY_READONLY_SECTION}

  .fini ${RELOCATING-0}${RELOCATING+ADDR(.init)+SIZEOF(.init)} :
  {
    ${RELOCATING+${FINI_START}}
    KEEP (*(SORT_NONE(.fini)))
    ${RELOCATING+${FINI_END}}
  } /* ${RELOCATING+ > INTERNAL_RAM} */ =${NOP-0}

  .text ${RELOCATING-0}${RELOCATING+ADDR(.fini)+SIZEOF(.fini)} :
  {
    ${RELOCATING+${TEXT_START_SYMBOLS}}
    *(.text .stub${RELOCATING+ .text.* .gnu.linkonce.t.*})
    /* .gnu.warning sections are handled specially by elf.em.  */
    *(.gnu.warning)
    ${RELOCATING+${OTHER_TEXT_SECTIONS}}
  } /* ${RELOCATING+ > INTERNAL_RAM} */ =${NOP-0}

  ${RELOCATING+PROVIDE (__${ETEXT_NAME} = .);}
  ${RELOCATING+PROVIDE (_${ETEXT_NAME} = .);}
  ${RELOCATING+PROVIDE (${ETEXT_NAME} = .);}
  ${WRITABLE_RODATA-${RODATA}}
  .rodata1      ${RELOCATING-0} : { *(.rodata1) }
  ${CREATE_SHLIB-${SDATA2}}
  ${CREATE_SHLIB-${SBSS2}}
  ${OTHER_READONLY_SECTIONS}
  .eh_frame_hdr ${RELOCATING-0} : { *(.eh_frame_hdr) }
  .eh_frame     ${RELOCATING-0} : ONLY_IF_RO { KEEP (*(.eh_frame)) }
  .gcc_except_table ${RELOCATING-0} : ONLY_IF_RO { *(.gcc_except_table${RELOCATING+ .gcc_except_table.*}) }

  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  ${CREATE_SHLIB-${CREATE_PIE-${RELOCATING+. = ${DATA_ADDR-${DATA_SEGMENT_ALIGN}};}}}
  ${CREATE_SHLIB+${RELOCATING+. = ${SHLIB_DATA_ADDR-${DATA_SEGMENT_ALIGN}};}}
  ${CREATE_PIE+${RELOCATING+. = ${SHLIB_DATA_ADDR-${DATA_SEGMENT_ALIGN}};}}

  /* Exception handling  */
  .eh_frame     ${RELOCATING-0} : ONLY_IF_RW { KEEP (*(.eh_frame)) }
  .gcc_except_table ${RELOCATING-0} : ONLY_IF_RW { *(.gcc_except_table${RELOCATING+ .gcc_except_table.*}) }

  /* Thread Local Storage sections  */
  .tdata	${RELOCATING-0} : { *(.tdata${RELOCATING+ .tdata.* .gnu.linkonce.td.*}) }
  .tbss		${RELOCATING-0} : { *(.tbss${RELOCATING+ .tbss.* .gnu.linkonce.tb.*})${RELOCATING+ *(.tcommon)} }

  .preinit_array   ${RELOCATING-0} :
  {
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__preinit_array_start = .);}}
    KEEP (*(.preinit_array))
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__preinit_array_end = .);}}
  }
  .init_array   ${RELOCATING-0} :
  {
     ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__init_array_start = .);}}
     ${RELOCATING+KEEP (*(SORT(.init_array.*)))}
     KEEP (*(.init_array))
     ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__init_array_end = .);}}
  }
  .fini_array   ${RELOCATING-0} :
  {
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__fini_array_start = .);}}
    KEEP (*(.fini_array))
    ${RELOCATING+KEEP (*(SORT(.fini_array.*)))}
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__fini_array_end = .);}}
  }
  ${SMALL_DATA_CTOR-${RELOCATING+${CTOR}}}
  ${SMALL_DATA_DTOR-${RELOCATING+${DTOR}}}
  .jcr          ${RELOCATING-0} : { KEEP (*(.jcr)) }

  ${RELOCATING+${DATARELRO}}
  ${OTHER_RELRO_SECTIONS}
  ${TEXT_DYNAMIC-${DYNAMIC}}
  ${DATA_GOT+${RELRO_NOW+${GOT}}}
  ${DATA_GOT+${RELRO_NOW+${GOTPLT}}}
  ${DATA_GOT+${RELRO_NOW-${SEPARATE_GOTPLT+${GOT}}}}
  ${RELOCATING+${DATA_SEGMENT_RELRO_END}}
  ${DATA_GOT+${RELRO_NOW-${SEPARATE_GOTPLT-${GOT}}}}
  ${DATA_GOT+${RELRO_NOW-${GOTPLT}}}

  ${DATA_PLT+${PLT_BEFORE_GOT-${PLT}}}

  .data ${RELOCATING-0}${RELOCATING+ADDR(.dtors)+SIZEOF(.dtors)} :
  {
    ${RELOCATING+${DATA_START_SYMBOLS}}
    *(.data${RELOCATING+ .data.* .gnu.linkonce.d.*})
    ${CONSTRUCTING+SORT(CONSTRUCTORS)}
  } /* ${RELOCATING+ > INTERNAL_RAM} */
  .data1        ${RELOCATING-0} : { *(.data1) }
  ${WRITABLE_RODATA+${RODATA}}
  ${OTHER_READWRITE_SECTIONS}
  ${SMALL_DATA_CTOR+${RELOCATING+${CTOR}}}
  ${SMALL_DATA_DTOR+${RELOCATING+${DTOR}}}
  ${DATA_PLT+${PLT_BEFORE_GOT+${PLT}}}
  ${SDATA_GOT+${RELOCATING+${OTHER_GOT_SYMBOLS}}}
  ${SDATA_GOT+${GOT}}
  ${SDATA_GOT+${OTHER_GOT_SECTIONS}}
  ${SDATA}
  ${OTHER_SDATA_SECTIONS}
  ${RELOCATING+${DATA_END_SYMBOLS-${USER_LABEL_PREFIX}_edata = .; PROVIDE (${USER_LABEL_PREFIX}edata = .);}}
  /* Align ___bss_start and _end to a multiple of 8 so that we can use strd
     to clear bss.  N.B., without adding any extra alignment, we would have
     to clear the bss byte by byte.  */
  ${RELOCATING+. = ALIGN(MAX(8,ALIGNOF(NEXT_SECTION)));}
  ${RELOCATING+___bss_start = .;}
  ${RELOCATING+${OTHER_BSS_SYMBOLS}}
  ${SBSS}
  ${BSS_PLT+${PLT}}
  .bss ${RELOCATING-0}${RELOCATING+ADDR(.rodata)+SIZEOF(.rodata)} :
  {
   ${RELOCATING+*(.dynbss)}
   *(.bss${RELOCATING+ .bss.* .gnu.linkonce.b.*})
   ${RELOCATING+*(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.
      FIXME: Why do we need it? When there is no .bss section, we do not
      pad the .data section.  */
   . = ALIGN(. != 0 ? ${ALIGNMENT} : 1);}
  } /* ${RELOCATING+ > INTERNAL_RAM} */
  ${OTHER_BSS_SECTIONS}
  ${RELOCATING+${OTHER_BSS_END_SYMBOLS}}
  ${RELOCATING+. = ALIGN(${ALIGNMENT});}
  ${LARGE_SECTIONS}
  ${RELOCATING+. = ALIGN(${ALIGNMENT});}
  ${RELOCATING+. = ALIGN(8);}
  ${RELOCATING+${OTHER_END_SYMBOLS}}
  ${RELOCATING+${END_SYMBOLS-${USER_LABEL_PREFIX}_end = .; PROVIDE (${USER_LABEL_PREFIX}end = .);}}
  ${RELOCATING+${DATA_SEGMENT_END}}

  ${RELOCATING+PROVIDE ( __stack_start_ = ORIGIN(EXTERNAL_DRAM_0) + __PROG_SIZE_FOR_CORE__ * __CORE_NUM_ + __PROG_SIZE_FOR_CORE__  - 0x10) ;}
  .stack ${RELOCATING-0}${RELOCATING+__stack_start_} :  {    ${RELOCATING+___stack = .;}    *(.stack)  }

  ${RELOCATING+PROVIDE (  ___heap_start = ORIGIN(EXTERNAL_DRAM_1)  + __HEAP_SIZE_FOR_CORE__ * __CORE_NUM_ );}
  ${RELOCATING+PROVIDE (  ___heap_end =   ORIGIN(EXTERNAL_DRAM_1)  + __HEAP_SIZE_FOR_CORE__ * __CORE_NUM_  + __HEAP_SIZE_FOR_CORE__ - 4 );}
EOF

if test -n "${NON_ALLOC_DYN}"; then
  if test -z "${NO_REL_RELOCS}${NO_RELA_RELOCS}"; then
    cat ldscripts/dyntmp.$$
  else
    if test -z "${NO_REL_RELOCS}"; then
      sed -e '/^[	 ]*\.rela\.[^}]*$/,/}/d' -e '/^[	 ]*\.rela\./d' ldscripts/dyntmp.$$
    fi
    if test -z "${NO_RELA_RELOCS}"; then
      sed -e '/^[	 ]*\.rel\.[^}]*$/,/}/d' -e '/^[	 ]*\.rel\./d' ldscripts/dyntmp.$$
    fi
  fi
  rm -f ldscripts/dyntmp.$$
fi

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
  ${ATTRS_SECTIONS}
  ${OTHER_SECTIONS}
  ${RELOCATING+${OTHER_SYMBOLS}}
  ${RELOCATING+${DISCARDED}}
}
EOF
