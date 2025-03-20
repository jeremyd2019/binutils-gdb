/* CTF format description.
   Copyright (C) 2019-2025 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_CTF_H
#define	_CTF_H

#include <sys/types.h>
#include <limits.h>
#include <stdint.h>


#ifdef	__cplusplus
extern "C"
{
#endif

/* CTF - Compact ANSI-C Type Format

   This file format can be used to compactly represent the information needed
   by a debugger to interpret the ANSI-C types used by a given program.
   Traditionally, this kind of information is generated by the compiler when
   invoked with the -g flag and is stored in "stabs" strings or in the more
   modern DWARF format.  CTF provides a representation of only the information
   that is relevant to debugging a complex, optimized C program such as the
   operating system kernel in a form that is significantly more compact than
   the equivalent stabs or DWARF representation.  The format is data-model
   independent, so consumers do not need different code depending on whether
   they are 32-bit or 64-bit programs; libctf automatically compensates for
   endianness variations.  CTF assumes that a standard ELF symbol table is
   available for use in the debugger, and uses the structure and data of the
   symbol table to avoid storing redundant information.  The CTF data may be
   compressed on disk or in memory, indicated by a bit in the header.  CTF may
   be interpreted in a raw disk file, or it may be stored in an ELF section,
   typically named .ctf.  Data structures are aligned so that a raw CTF file or
   CTF ELF section may be manipulated using mmap(2).

   The CTF file or section is a superset of BTF, and has the following structure:

   +--------+--------+---------+----------+--------+----------+...
   +   BTF  |   CTF  |  data   | function | object | function |...
   | header | header | objects |   info   | index  |  index   |...
   +--------+--------+----------+--------+-------------------+...

   ...+-------+--------+
   ...| data  | string |
   ...| types | table  |
      +-------+--------+

   The file header stores a magic number and version information, encoding
   flags, and the byte offset and length of each of the sections relative to
   the end of the header itself.  There are two headers: the BTF header contains
   offsets relative to the end of the BTF header, and immediately following it
   there may be a CTF header containing offsets relative to the end of the CTF
   header.  If the BTF header is not followed by a CTFv4_MAGIC, no CTF header
   is present and this dict is pure BTF (and cannot contain CTF-specific type
   kinds).

   If the CTF data has been uniquified against another set of CTF data, a
   reference to that data also appears in the the header.  This reference is the
   name of the parent dict containing the types uniquified against.

   Data object and function records (collectively, "symtypetabs") are stored in
   the same order as they appear in the corresponding symbol table, except that
   symbols marked SHN_UNDEF are not stored and symbols that have no type data
   are padded out with zeroes.  For each entry in these tables, the type ID (a
   small integer) is recorded.  (Functions get CTF_K_FUNCTION types, just like
   data objects that are function pointers.)

   For situations in which the order of the symbols in the symtab is not known,
   or most symbols have no type in this dict and most entries would be
   zero-pads, a pair of optional indexes follow the data object and function
   info sections: each of these is an array of strtab indexes, mapped 1:1 to the
   corresponding data object / function info section, giving each entry in those
   sections a name so that the linker can correlate them with final symtab
   entries and reorder them accordingly (dropping the indexes in the process).

   Variable records (as distinct from data objects) provide a modicum of support
   for non-ELF systems, mapping a variable or function name to a CTF type ID.
   The names are sorted into ASCIIbetical order, permitting binary searching.
   We do not define how the consumer maps these variable names to addresses or
   anything else, or indeed what these names represent: they might be names
   looked up at runtime via dlsym() or names extracted at runtime by a debugger
   or anything else the consumer likes.  Variable records with identically-
   named entries in the data object or function index section are removed.

   The data types section is a list of variable size records that represent each
   type, in order by their ID.  The types themselves form a directed graph,
   where each node may contain one or more outgoing edges to other type nodes,
   denoted by their ID.  Most type nodes are standalone or point backwards to
   earlier nodes, but this is not required: nodes can point to later nodes,
   particularly structure and union members.

   Strings are recorded as a string table ID (0 or 1) and a byte offset into the
   string table.  String table 0 is the internal CTF string table.  String table
   1 is the external string table, which is the string table associated with the
   ELF dynamic symbol table for this object.  CTF does not record any strings
   that are already in the symbol table, and the CTF string table does not
   contain any duplicated strings.

   If the CTF data has been merged with another parent CTF object, some outgoing
   edges may refer to type nodes that exist in another CTF object.  The debugger
   and libctf library are responsible for connecting the appropriate objects
   together so that the full set of types can be explored and manipulated.

   This connection is done purely using the ctf_import() function.  The
   ctf_archive machinery (and thus ctf_open et al) automatically imports archive
   members named ".ctf" into child dicts if available in the same archive, to
   match the relationship set up by the linker, but callers can call ctf_import
   themselves as well if need be, if they know a different relationship is in
   force.  */

#define CTF_MAX_TYPE	0xfffffffe	/* Max type identifier value.  */
#define CTF_MAX_PTYPE	0x7fffffff	/* Max parent type identifier value.  */
#define CTF_MAX_NAME 0x7fffffff		/* Max offset into a string table.  */
#define CTF_MAX_VLEN_V2	0xffffff /* Max struct, union, enum members or args.  */
#define CTF_MAX_VLEN	0xffffffff /* Max struct, union, enum members or args:
				      may need CTFv4-only CTF_K_BIG.  */
#define CTF_MAX_RAW_VLEN 0xffff /* Max BTF struct, union, enum members or args.  */

/* See ctf_v3_type_t.  */
#define CTF_MAX_SIZE	0xfffffffe	/* Max size of a v2+/BTF type in bytes. */
#define CTF_MAX_RAW_SIZE 0xfffffffffffffffeULL	/* Max size ofr a CTF_K_BIG type.  */
#define CTF_LSIZE_SENT	0xffffffff	/* Sentinel for v2 ctt_size.  */

# define CTF_MAX_TYPE_V1	0xffff	/* Max type identifier value.  */
# define CTF_MAX_PTYPE_V1	0x7fff	/* Max parent type identifier value.  */
# define CTF_MAX_VLEN_V1	0x3ff	/* Max struct, union, enums or args.  */
# define CTF_MAX_SIZE_V1	0xfffe	/* Max size of a type in bytes. */
# define CTF_LSIZE_SENT_V1	0xffff	/* Sentinel for v1 ctt_size.  */

/* Start of actual data structure definitions.

   Every field in these structures must have corresponding code in the
   endianness-swapping machinery in libctf/ctf-open.c.  */

/* Warning: not aligned with the BTF preamble, though most of the fields are
   usually overlapping.  */

typedef struct ctf_preamble_v3
{
  unsigned short ctp_magic;	/* Magic number (CTF_MAGIC).  */
  unsigned char ctp_version;	/* Data format version number (CTF_VERSION).  */
  unsigned char ctp_flags;	/* Flags (see below).  */
} ctf_preamble_v3_t;

/* Header for CTFv1 and v2.  */
typedef struct ctf_header_v2
{
  ctf_preamble_v3_t cth_preamble;
  uint32_t cth_parlabel;	/* Ref to name of parent lbl uniq'd against.  */
  uint32_t cth_parname;		/* Ref to basename of parent.  */
  uint32_t cth_lbloff;		/* Offset of label section.  */
  uint32_t cth_objtoff;		/* Offset of object section.  */
  uint32_t cth_funcoff;		/* Offset of function section.  */
  uint32_t cth_varoff;		/* Offset of variable section.  */
  uint32_t cth_typeoff;		/* Offset of type section.  */
  uint32_t cth_stroff;		/* Offset of string section.  */
  uint32_t cth_strlen;		/* Length of string section in bytes.  */
} ctf_header_v2_t;

/* Header for CTFv3 only.  */
typedef struct ctf_header_v3
{
  ctf_preamble_v3_t cth_preamble;
  uint32_t cth_parlabel;	/* Ref to name of parent lbl uniq'd against.  */
  uint32_t cth_parname;		/* Ref to basename of parent.  */
  uint32_t cth_cuname;		/* Ref to CU name (may be 0).  */
  uint32_t cth_lbloff;		/* Offset of label section.  */
  uint32_t cth_objtoff;		/* Offset of object section.  */
  uint32_t cth_funcoff;		/* Offset of function section.  */
  uint32_t cth_objtidxoff;	/* Offset of object index section.  */
  uint32_t cth_funcidxoff;	/* Offset of function index section.  */
  uint32_t cth_varoff;		/* Offset of variable section.  */
  uint32_t cth_typeoff;		/* Offset of type section.  */
  uint32_t cth_stroff;		/* Offset of string section.  */
  uint32_t cth_strlen;		/* Length of string section in bytes.  */
} ctf_header_v3_t;

/* Derived from btf.h in the Linux kernel, but independent (to ensure that btf.h
   changes do not change the CTF file format) and using userspace types.  */

typedef struct ctf_btf_preamble
{
  uint16_t btf_magic;		/* BTF_MAGIC */
  uint8_t btf_version;		/* Always 1, for now */
  uint8_t btf_flags;		/* Always 0, for now */
} ctf_btf_preamble_t;

typedef struct ctf_btf_header
{
  ctf_btf_preamble_t bth_preamble;
  uint32_t bth_hdr_len;		/* De-facto BTF version number */
  uint32_t bth_type_off;	/* Offset of type section.  */
  uint32_t bth_type_len;	/* Length of type section.  */
  uint32_t bth_str_off;		/* Offset of string section.  */
  uint32_t bth_str_len;		/* Length of string section.  */
} ctf_btf_header_t;

typedef struct ctf_preamble
{
  uint64_t ctp_magic_version;	/* Magic number (CTFv4_MAGIC) and version.  */
  uint64_t ctp_flags;		/* Flags (see below).  */
} ctf_preamble_t;

/* Offsets in this header are relative to the end of the ctf_btf_header.  */

typedef struct ctf_header
{
  ctf_btf_header_t btf;		/* Leading component is BTF. */
  ctf_preamble_t cth_preamble;
  uint32_t cth_cu_name;		/* Ref to CU name (may be 0).  */
  uint32_t cth_parent_name;	/* Ref to basename of parent.  */
  uint32_t cth_parent_strlen;	/* cth_strlen of parent (may be 0).  */
  uint32_t cth_parent_ntypes;	/* Number of types in parent (may be 0).  */
  uint32_t cth_objt_off;	/* Offset of object section.  */
  uint32_t cth_objt_len;	/* Length of object section.  */
  uint32_t cth_func_off;	/* Offset of function section.  */
  uint32_t cth_func_len;	/* Length of function section.  */
  uint32_t cth_objtidx_off;	/* Offset of object index section.  */
  uint32_t cth_objtidx_len;	/* Length of object index section.  */
  uint32_t cth_funcidx_off;	/* Offset of function index section.  */
  uint32_t cth_funcidx_len;	/* Length of function index section.  */
} ctf_header_t;

/* The ctp_magic_version field is a magic number (high 48 bits) and a version
   (low 16).  Of course this may be in the wrong endianness for the running
   system. */
#define CTH_MAGIC(hdr)   ((hdr->cth_preamble.ctp_magic_version) >> 16)
#define CTH_VERSION(hdr) ((hdr->cth_preamble.ctp_magic_version) & (((uint64_t) ~0) >> 48))
#define cth_flags   cth_preamble.ctp_flags

#define CTF_MAGIC	0xdff2	/* v3 and below: magic number identifying header.  */
#define CTF_BTF_MAGIC	0xeb9f
#define CTFv4_MAGIC	0xd167ae03a2c5		/* 48 bits */

/* Data format version number.  */

/* v1 upgraded to v2/v3 is not quite the same as the native form, because the
   boundary between parent and child types is different but not recorded
   anywhere, and you can write it out again via ctf_compress_write(), so we must
   track whether the thing was originally v1 or not.  If we were writing the
   header from scratch, we would add a *pair* of version number fields to allow
   for this, but this will do for now.  (A flag will not do, because we need to
   encode both the version we came from and the version we went to, not just "we
   were upgraded".)

   When upgrading to v4, we can simply record the boundary in
   cth_parent_typemax.  */

#define CTF_VERSION_1 1
#define CTF_VERSION_1_UPGRADED_3 2
#define CTF_VERSION_2 3
#define CTF_VERSION_3 4

#define CTF_VERSION_4 5
#define CTF_VERSION CTF_VERSION_4 /* Current version.  */
#define CTF_STABLE_VERSION 4

#define CTF_BTF_VERSION 1

/* All of these flags bar CTF_F_COMPRESS and CTF_F_IDXSORTED are bug-workaround
   flags and are valid only in format v3: in v2 and below they cannot occur and
   in v4 and later, they will be recycled for other purposes.  */

#define CTF_F_COMPRESS	0x1		/* Data buffer is compressed by libctf.  */
#define CTF_F_NEWFUNCINFO 0x2		/* New v3 func info section format.  */
#define CTF_F_IDXSORTED 0x4		/* Index sections already sorted.  */
#define CTF_F_DYNSTR 0x8		/* Strings come from .dynstr.  */
#define CTF_F_MAX_3 (CTF_F_COMPRESS | CTF_F_NEWFUNCINFO | CTF_F_IDXSORTED	\
		     | CTF_F_DYNSTR)

#define CTF_F_MAX (CTF_F_COMPRESS | CTF_F_IDXSORTED)

  /* CTFv3 and below: variable entries.  */
typedef struct ctf_varent_v3
{
  uint32_t ctv_name;		/* Reference to name in string table.  */
  uint32_t ctv_type;		/* Index of type of this variable.  */
} ctf_varent_v3_t;

/* In format v2 and v3, type sizes, measured in bytes, come in two flavours.
   Nearly all of them fit into a (UINT_MAX - 1), and thus can be stored in the
   ctt_size member of a ctf_stype_v2_t.  The maximum value for these sizes is
   CTF_MAX_SIZE.  Types larger than this must be stored in the ctf_lsize member
   of a ctf_type_t.  Use of this member is indicated by the presence of
   CTF_LSIZE_SENT in ctt_size.

   In CTFv4, the CTF_K_BIG prefixed kind is used for the same purpose.  */

/* In v1, the same applies, only the limit is (USHRT_MAX - 1) and
   CTF_MAX_SIZE_V1, and CTF_LSIZE_SENT_V1 is the sentinel.  */

typedef struct ctf_stype_v1
{
  uint32_t ctt_name;		/* Reference to name in string table.  */
  unsigned short ctt_info;	/* Encoded kind, variant length (see below).  */
#ifndef __GNUC__
  union
  {
    unsigned short _size;	/* Size of entire type in bytes.  */
    unsigned short _type;	/* Reference to another type.  */
  } _u;
#else
  __extension__
  union
  {
    unsigned short ctt_size;	/* Size of entire type in bytes.  */
    unsigned short ctt_type;	/* Reference to another type.  */
  };
#endif
} ctf_stype_v1_t;

typedef struct ctf_type_v1
{
  uint32_t ctt_name;		/* Reference to name in string table.  */
  unsigned short ctt_info;	/* Encoded kind, variant length (see below).  */
#ifndef __GNUC__
  union
  {
    unsigned short _size;	/* Always CTF_LSIZE_SENT_V1.  */
    unsigned short _type;	/* Do not use.  */
  } _u;
#else
  __extension__
  union
  {
    unsigned short ctt_size;	/* Always CTF_LSIZE_SENT_V1.  */
    unsigned short ctt_type;	/* Do not use.  */
  };
#endif
  uint32_t ctt_lsizehi;		/* High 32 bits of type size in bytes.  */
  uint32_t ctt_lsizelo;		/* Low 32 bits of type size in bytes.  */
} ctf_type_v1_t;


typedef struct ctf_v2_stype
{
  uint32_t ctt_name;		/* Reference to name in string table.  */
  uint32_t ctt_info;		/* Encoded kind, variant length (see below).  */
#ifndef __GNUC__
  union
  {
    uint32_t _size;		/* Size of entire type in bytes.  */
    uint32_t _type;		/* Reference to another type.  */
  } _u;
#else
  __extension__
  union
  {
    uint32_t ctt_size;		/* Size of entire type in bytes.  */
    uint32_t ctt_type;		/* Reference to another type.  */
  };
#endif
} ctf_stype_v2_t;

typedef struct ctf_type_v2
{
  uint32_t ctt_name;		/* Reference to name in string table.  */
  uint32_t ctt_info;		/* Encoded kind, variant length (see below).  */
#ifndef __GNUC__
union
  {
    uint32_t _size;		/* Always CTF_LSIZE_SENT.  */
    uint32_t _type;		/* Do not use.  */
  } _u;
#else
  __extension__
  union
  {
    uint32_t ctt_size;		/* Always CTF_LSIZE_SENT.  */
    uint32_t ctt_type;		/* Do not use.  */
  };
#endif
  uint32_t ctt_lsizehi;		/* High 32 bits of type size in bytes.  */
  uint32_t ctt_lsizelo;		/* Low 32 bits of type size in bytes.  */
} ctf_type_v2_t;

/* Identical to btf_type.  */
typedef struct ctf_type
{
  uint32_t ctt_name;		/* Reference to name in string table.  */
  uint32_t ctt_info;		/* Encoded kind, variant length (see below).  */
#ifndef __GNUC__
union
  {
    uint32_t _size;		/* Always CTF_LSIZE_SENT.  */
    uint32_t _type;		/* Do not use.  */
  } _u;
#else
  __extension__
  union
  {
    uint32_t ctt_size;		/* Always CTF_LSIZE_SENT.  */
    uint32_t ctt_type;		/* Do not use.  */
  };
#endif
} ctf_type_t;

#ifndef __GNUC__
#define ctt_size _u._size	/* For fundamental types that have a size.  */
#define ctt_type _u._type	/* For types that reference another type.  */
#endif
  
/* The following macros and inline functions compose and decompose values for
   ctt_info and ctt_name, as well as other structures that contain name
   references.  Use outside libdtrace-ctf itself is explicitly for access to CTF
   files directly: types returned from the library will always appear to be
   CTF_V2.

   v1: (transparently upgraded to v2 at open time: may be compiled out of the
   library)
               ------------------------
   ctt_info:   | kind | isroot | vlen |
               ------------------------
               15   11    10    9     0

   v2 and v3:
               ------------------------
   ctt_info:   | kind | isroot | vlen |
               ------------------------
               31    26    25  24     0

   v4 and BTF:

   * bits  0-15: vlen (e.g. # of struct's members)
   * bits 16-23: unused
   * bits 24-28: kind (e.g. int, ptr, array...etc)
   * bits 29-30: unused
   * bit     31: kind_flag, currently used by
   *             struct, union, enum, fwd and enum64

   Types requiring larger bits use prefix kinds, CTF_K_PREFIX
   below.

   CTF_V1 and V2 _INFO_VLEN have the same interface:

   kind = CTF_*_INFO_KIND(c.ctt_info);     <-- CTF_K_* value (see below)
   vlen = CTF_*_INFO_VLEN(fp, c.ctt_info); <-- length of variable data list

   stid = CTF_NAME_STID(c.ctt_name);     <-- string table id number (0 or 1)
   offset = CTF_NAME_OFFSET(c.ctt_name); <-- string table byte offset

   c.ctt_info = CTF_TYPE_INFO(kind, vlen);
   c.ctt_name = CTF_TYPE_NAME(stid, offset);  */

#define CTF_V1_INFO_KIND(info)		(((info) & 0xf800) >> 11)
#define CTF_V1_INFO_ISROOT(info)	(((info) & 0x0400) >> 10)
#define CTF_V1_INFO_VLEN(info)		(((info) & CTF_MAX_VLEN_V1))

#define CTF_V2_INFO_KIND(info)		(((info) & 0xfc000000) >> 26)
#define CTF_V2_INFO_ISROOT(info)	(((info) & 0x2000000) >> 25)
#define CTF_V2_INFO_VLEN(info)		(((info) & CTF_MAX_VLEN_V2))

#define CTF_INFO_KFLAG(info)		(((info) & 0x7fffffff) >> 31)
#define CTF_INFO_KIND(info)		(((info) >> 24) & 0x1f)
#define CTF_INFO_VLEN(info)		(((info) & CTF_MAX_RAW_VLEN))

#define CTF_NAME_STID(name)		((name) >> 31)
#define CTF_NAME_OFFSET(name)		((name) & CTF_MAX_NAME)
#define CTF_SET_STID(name, stid)	((name) | ((unsigned int) stid) << 31)

/* V4 only. */
#define CTF_TYPE_INFO(kind, kflag, vlen) \
	(((kind) << 24) | (((kflag) ? 1 : 0) << 31) | ((vlen) & CTF_MAX_RAW_VLEN))

#define CTF_TYPE_NAME(stid, offset) \
	(((stid) << 31) | ((offset) & CTF_MAX_NAME))

/* The next set of macros are for public consumption only.  Not used internally,
   since the relevant type boundary is dependent upon the version of the file at
   *opening* time, not the version after transparent upgrade.  Use
   ctf_type_isparent() / ctf_type_ischild() for that.  */

#define CTF_V2_TYPE_ISPARENT(fp, id)	((id) <= CTF_MAX_PTYPE)
#define CTF_V2_TYPE_ISCHILD(fp, id)	((id) > CTF_MAX_PTYPE)
#define CTF_V2_TYPE_TO_INDEX(id)	((id) & CTF_MAX_PTYPE)
#define CTF_V2_INDEX_TO_TYPE(id, child) ((child) ? ((id) | (CTF_MAX_PTYPE+1)) : (id))

#define CTF_V1_TYPE_ISPARENT(fp, id)	((id) <= CTF_MAX_PTYPE_V1)
#define CTF_V1_TYPE_ISCHILD(fp, id)	((id) > CTF_MAX_PTYPE_V1)
#define CTF_V1_TYPE_TO_INDEX(id)	((id) & CTF_MAX_PTYPE_V1)
#define CTF_V1_INDEX_TO_TYPE(id, child) ((child) ? ((id) | (CTF_MAX_PTYPE_V1+1)) : (id))

/* Valid for V1 -- V3, but not V4. */
#define CTF_V3_TYPE_LSIZE(cttp) \
	(((uint64_t)(cttp)->ctt_lsizehi) << 32 | (cttp)->ctt_lsizelo)

/* Valid for v4 as well: splits sizes into prefix-type and non-prefix-type
   portions.  */
#define CTF_SIZE_TO_LSIZE_HI(size)	((uint32_t)((uint64_t)(size) >> 32))
#define CTF_SIZE_TO_LSIZE_LO(size)	((uint32_t)(size))

#define CTF_VLEN_TO_VLEN_HI(vlen)	((uint16_t)((uint32_t)(vlen) >> 16))
#define CTF_VLEN_TO_VLEN_LO(vlen)	((uint16_t)(vlen))

/* CTF_STRTAB_1 not valid in BTF, since strtab offsets high enough to be in
   strtab 1 have no meaning there.  */
#define CTF_STRTAB_0	0	/* String table id 0 (in-CTF).  */
#define CTF_STRTAB_1	1	/* String table id 1 (ELF strtab).  */

/* Values for CTF_TYPE_KIND().  If the kind has an associated data list,
   CTF_INFO_VLEN() will extract the number of elements in the list, and
   the type of each element is shown in the comments below.  */

#define CTF_V3_K_UNKNOWN 0	/* Unknown type (used for padding and
				   unrepresentable types).  */
#define CTF_V3_K_INTEGER 1	/* Variant data is CTF_INT_DATA (see below).  */
#define CTF_V3_K_FLOAT   2	/* Variant data is CTF_FP_DATA (see below).  */
#define CTF_V3_K_POINTER 3	/* ctt_type is referenced type.  */
#define CTF_V3_K_ARRAY   4	/* Variant data is single ctf_array_t.  */
#define CTF_V3_K_FUNCTION 5	/* ctt_type is return type, variant data is
				   list of argument types (unsigned short's for v1,
				   uint32_t's for v2).  */
#define CTF_V3_K_STRUCT  6	/* Variant data is list of ctf_member_t's.  */
#define CTF_V3_K_UNION   7	/* Variant data is list of ctf_member_t's.  */
#define CTF_V3_K_ENUM    8	/* Variant data is list of ctf_enum_t's.  */
#define CTF_V3_K_FORWARD 9	/* No additional data; ctt_name is tag.  */
#define CTF_V3_K_TYPEDEF 10	/* ctt_type is referenced type.  */
#define CTF_V3_K_VOLATILE 11	/* ctt_type is base type.  */
#define CTF_V3_K_CONST   12	/* ctt_type is base type.  */
#define CTF_V3_K_RESTRICT 13	/* ctt_type is base type.  */
#define CTF_V3_K_SLICE   14	/* Variant data is a ctf_slice_t.  */

#define CTF_V3_K_MAX	14	/* Maximum possible (V3) CTF_K_* value.  */

/* Values for CTF_TYPE_KIND() for BTF, shared by CTFv4.  Kind names as unchanged
   as possible, since they are user-exposed, but their values all differ.  */

#define CTF_K_UNKNOWN  0	/* Unknown type (used for padding and
				   unrepresentable and suppressed types).  */
#define CTF_K_INTEGER  1	/* Variant data is CTF_INT_DATA (see below).  */
#define CTF_K_POINTER  2	/* ctt_type is referenced type.  */
#define CTF_K_ARRAY    3	/* Variant data is single ctf_array_t.  */
#define CTF_K_STRUCT   4	/* Variant data is list of ctf_member_t's;
				   kind_flag 1 if bitfields present.  */
#define CTF_K_UNION    5	/* Ditto.  */
#define CTF_K_ENUM     6	/* Variant data is list of ctf_enum_t's: if 0,
				   this is a forward.  kflag 1 is signed.  */
#define CTF_K_FORWARD  7	/* No additional data; kind_flag 1 for unions.  */
#define CTF_K_TYPEDEF  8	/* ctt_type is referenced type.  */
#define CTF_K_VOLATILE 9	/* ctt_type is base type.  */
#define CTF_K_CONST    10	/* ctt_type is base type.  */
#define CTF_K_RESTRICT 11	/* ctt_type is base type.  */
#define CTF_K_FUNC_LINKAGE 12	/* Variant data is ctf_linkage_t; ctt_type
				   is CTF_K_FUNC_PROTO.  Named.  */
#define CTF_K_FUNCTION 13	/* ctt_type is return type, variant data is
				   list of ctf_param_t.  Unnamed.  */
#define CTF_K_VAR      14	/* Variable.  ctt_type is variable type.
				   Variant data is ctf_linkage_t.  */
#define CTF_K_DATASEC  15	/* Variant data is list of ctf_var_secinfo_t.  */
#define CTF_K_BTF_FLOAT 16	/* No data beyond a size.  */
#define CTF_K_DECL_TAG 17	/* ctt_type is referenced type.  Variant data is
				   ctf_decl_tag_t.  */
#define CTF_K_TYPE_TAG 18	/* ctt_type is referenced type.  */
#define CTF_K_ENUM64  19	/* Variant data is list of ctf_enum64_t.  kflag
				   1 is signed.  */

/* Values for CTF_TYPE_KIND() for CTFv4.  Count down from the top of the ID
   space, */

#define CTF_K_FLOAT   31	/* Variant data is a CTF_FP_* value.  */
#define CTF_K_SLICE   30	/* Variant data is a ctf_slice_t.  */
#define CTF_K_BIG     29	/* Prefix type.
				   vlen is high 16 bits of type vlen;
				   size is high 32 bits of type size.  */
#define CTF_K_CONFLICTING 28	/* Prefix type.  Name is disambiguator for
				   conflicting type (e.g. translation unit
				   name).

				   If a type is both CONFLICTING and BIG,
				   CONFLICTING will always prefix BIG.  */
#define CTF_BTF_K_MAX	19	/* Maximum possible (V4) BTF_K_* value.  */
#define CTF_K_MAX	31	/* Maximum possible (V4) CTF_K_* value.  */


#define CTF_PREFIX_KIND(kind) ((kind) == CTF_K_BIG || (kind) == CTF_K_CONFLICTING)

/* Values for ctt_type when kind is CTF_K_INTEGER.  The flags, offset in bits,
   and size in bits are encoded as a single word using the following macros.
   (However, you can also encode the offset and bitness in a slice, or directly
   in a struct: many clients, e.g. libbpf, do not allow nonzero bit offsets
   or bits values in base types at all.)  */

#define CTF_INT_ENCODING(data) (((data) & 0xff000000) >> 24)
#define CTF_INT_OFFSET(data)   (((data) & 0x00ff0000) >> 16)
#define CTF_INT_BITS(data)     (((data) & 0x0000ffff))

#define CTF_INT_DATA(encoding, offset, bits) \
       (((encoding) << 24) | ((offset) << 16) | (bits))

#define CTF_INT_SIGNED	0x01	/* Integer is signed (otherwise unsigned).  */
#define CTF_INT_CHAR	0x02	/* Character display format.  */
#define CTF_INT_BOOL	0x04	/* Boolean display format.  */

/* Use CTF_CHAR to produce a char that agrees with the system's native
   char signedness.  */
#if CHAR_MIN == 0
# define CTF_CHAR (CTF_INT_CHAR)
#else
# define CTF_CHAR (CTF_INT_CHAR | CTF_INT_SIGNED)
#endif

/* Values for ctt_type when kind is CTF_K_FLOAT in CTFv3 and below.  The
   encoding, offset in bits, and size in bits are encoded as a single word using
   the following macros.  (However, you can also encode the offset and bitness
   in a slice.)  */

#define CTF_FP_ENCODING(data)  (((data) & 0xff000000) >> 24)
#define CTF_FP_OFFSET(data)    (((data) & 0x00ff0000) >> 16)
#define CTF_FP_BITS(data)      (((data) & 0x0000ffff))

#define CTF_FP_DATA(encoding, offset, bits) \
       (((encoding) << 24) | ((offset) << 16) | (bits))

/* Variant data when kind is CTF_K_FLOAT is an encoding in the top eight bits.
   In v4, it's a straight encoding of the CTF_FP_* type.  Dicts translated from
   v3 lose their offset and bits flags (which were meaningless anyway). */
#define CTF_V3_FP_ENCODING(data)	(((data) & 0xff000000) >> 24)

#define CTF_FP_UNKNOWN  0	/* Unknown encoding.  */
#define CTF_FP_SINGLE	1	/* IEEE 32-bit float encoding.  */
#define CTF_FP_DOUBLE	2	/* IEEE 64-bit float encoding.  */
#define CTF_FP_CPLX	3	/* Complex encoding.  */
#define CTF_FP_DCPLX	4	/* Double complex encoding.  */
#define CTF_FP_LDCPLX	5	/* Long double complex encoding.  */
#define CTF_FP_LDOUBLE	6	/* Long double encoding.  */

#define CTF_FP_MAX	6	/* Maximum possible CTF_FP_* value.  */

/* CTFv3 and below only.  Never generated by GCC.  */

#define CTF_FP_INTRVL	7	/* Interval (2x32-bit) encoding.  */
#define CTF_FP_DINTRVL	8	/* Double interval (2x64-bit) encoding.  */
#define CTF_FP_LDINTRVL	9	/* Long double interval (2x128-bit) encoding.  */
#define CTF_FP_IMAGRY	10	/* Imaginary (32-bit) encoding.  */
#define CTF_FP_DIMAGRY	11	/* Long imaginary (64-bit) encoding.  */
#define CTF_FP_LDIMAGRY	12	/* Long double imaginary (128-bit) encoding.  */

#define CTF_V3_FP_MAX	12	/* Maximum possible CTF_FP_* value in v3 and
				   below.  */

/* A slice increases the offset and reduces the bitness of the referenced
   ctt_type, which must be a type which has an encoding (int or enum).  We
   also store the referenced type in here, because it is easier to keep the
   ctt_size correct for the slice than to shuffle the size into here and keep
   the ctt_type where it is for other types.

   CTFv4 only, not BTF.  */

typedef struct ctf_slice
{
  uint32_t cts_type;
  unsigned short cts_offset;
  unsigned short cts_bits;
} ctf_slice_t;

typedef struct ctf_array_v1
{
  unsigned short cta_contents;	/* Reference to type of array contents.  */
  unsigned short cta_index;	/* Reference to type of array index.  */
  uint32_t cta_nelems;		/* Number of elements.  */
} ctf_array_v1_t;

typedef struct ctf_array
{
  uint32_t cta_contents;	/* Reference to type of array contents.  */
  uint32_t cta_index;		/* Reference to type of array index.  */
  uint32_t cta_nelems;		/* Number of elements.  */
} ctf_array_t;

/* (CTF < v4.)

   Most structure members have bit offsets that can be expressed using a short.
   Some don't.  ctf_member_t is used for structs which cannot contain any of
   these large offsets, whereas ctf_lmember_t is used in the latter case.  If
   any member of a given struct has an offset that cannot be expressed using a
   uint32_t, all members will be stored as type ctf_lmember_t.  This is expected
   to be very rare (but nonetheless possible).  */

#define CTF_LSTRUCT_THRESH	536870912

/* In v1, the same is true, except that lmembers are used for structs >= 8192
   bytes in size.  (The ordering of members in the ctf_member_* structures is
   different to improve padding.)  */

#define CTF_LSTRUCT_THRESH_V1	8192

typedef struct ctf_member_v1
{
  uint32_t ctm_name;		/* Reference to name in string table.  */
  unsigned short ctm_type;	/* Reference to type of member.  */
  unsigned short ctm_offset;	/* Offset of this member in bits.  */
} ctf_member_v1_t;

typedef struct ctf_lmember_v1
{
  uint32_t ctlm_name;		/* Reference to name in string table.  */
  unsigned short ctlm_type;	/* Reference to type of member.  */
  unsigned short ctlm_pad;	/* Padding.  */
  uint32_t ctlm_offsethi;	/* High 32 bits of member offset in bits.  */
  uint32_t ctlm_offsetlo;	/* Low 32 bits of member offset in bits.  */
} ctf_lmember_v1_t;

typedef struct ctf_member_v2
{
  uint32_t ctm_name;		/* Reference to name in string table.  */
  uint32_t ctm_offset;		/* Offset of this member in bits.  */
  uint32_t ctm_type;		/* Reference to type of member.  */
} ctf_member_v2_t;

typedef struct ctf_lmember_v2
{
  uint32_t ctlm_name;		/* Reference to name in string table.  */
  uint32_t ctlm_offsethi;	/* High 32 bits of member offset in bits.  */
  uint32_t ctlm_type;		/* Reference to type of member.  */
  uint32_t ctlm_offsetlo;	/* Low 32 bits of member offset in bits.  */
} ctf_lmember_v2_t;

#define	CTF_V3_LMEM_OFFSET(ctlmp) \
	(((uint64_t)(ctlmp)->ctlm_offsethi) << 32 | (ctlmp)->ctlm_offsetlo)
#define	CTF_V3_OFFSET_TO_LMEMHI(offset)	((uint32_t)((uint64_t)(offset) >> 32))
#define	CTF_V3_OFFSET_TO_LMEMLO(offset)	((uint32_t)(offset))

/* Aligned with btf_member.  */
typedef struct ctf_member
{
  uint32_t ctm_name;		/* Reference to name in string table.  */
  uint32_t ctm_type;		/* Reference to type of member.  */
  uint32_t ctm_offset;		/* Offset of this member in bits; possibly bit
				   offset.  In CTF_KIND_BIG, offset from the
				   *previous* member.  */
} ctf_member_t;

/* Used when the CTF_KIND_{STRUCT,UNION} kind_flag is on, indicating
   bitfields.  Bit offset and size override offsets from the underlying
   encoding, including slices.  */

#define CTF_MAX_BIT_OFFSET 0xffffff
#define CTF_MEMBER_BIT_SIZE(val) ((val) >> 24)
#define CTF_MEMBER_BIT_OFFSET(val) ((val) & CTF_MAX_BIT_OFFSET)
#define CTF_MEMBER_MAKE_BIT_OFFSET(size, offset) ((size) << 24 | offset)

/* Data sections, aligned with btf_var_secinfo.

   TODO: Do we want a CTFv4 extended variant with 64-bit size for
   CTF_KIND_BIG?  */
typedef struct ctf_var_secinfo
{
  uint32_t cvs_type;
  uint32_t cvs_offset;
  uint32_t cvs_size;		/* If 0, use type size.  */
} ctf_var_secinfo_t;

/* Linkages, aligned with enum btf_func_linkage.  */
#define CTF_VAR_STATIC 0
#define CTF_VAR_GLOBAL_ALLOCATED 1
#define CTF_VAR_GLOBAL_EXTERN 2

#define CTF_FUNC_STATIC 0
#define CTF_FUNC_GLOBAL 1
#define CTF_FUNC_EXTERN 2

/* Linkage of a CTF_K_FUNC_LINKAGE and CTF_K_VAR (holds CTF_FUNC_*
   or CTF_VAR_*, depending).  */
typedef struct ctf_linkage
{
  uint32_t ctl_linkage;
} ctf_linkage_t;

/* Parameter data for CTF_K_FUNCTION.  Aligned with btf_param.  */

typedef struct ctf_param
{
  uint32_t cfp_name;
  uint32_t cfp_type;
} ctf_param_t;

/* Variant data of CTF_K_DECL_TAG.  component_idx != -1 means that this tag
   applies to the given member or func argument.  */

typedef struct ctf_decl_tag
{
  int32_t cdt_component_idx;
} ctf_decl_tag_t;

typedef struct ctf_enum
{
  uint32_t cte_name;		/* Reference to name in string table.  */
  int32_t cte_value;		/* Value associated with this name.  */
} ctf_enum_t;

typedef struct ctf_enum64
{
  uint32_t cte_name;		/* Reference to name in string table.  */
  int64_t cte_value;		/* Value associated with this name.
				   (May actually be signed.)  */
} ctf_enum64_t;

/* The ctf_archive is a collection of ctf_dict_t's stored together. The format
   is suitable for mmap()ing: this control structure merely describes the
   mmap()ed archive (and overlaps the first few bytes of it), hence the
   greater care taken with integral types.  All CTF files in an archive
   must have the same data model.  (This is not validated.)

   All integers in the ctfa_archive_v1 structure are stored in little-endian byte
   order.

   The code relies on the fact that everything in this header is a uint64_t
   and thus the header needs no padding (in particular, that no padding is
   needed between ctfa_ctfs and the unnamed ctfa_archive_modent array
   that follows it).

   This is *not* the same as the data structure returned by the ctf_arc_*()
   functions:  this is the low-level on-disk representation.  */

/* UPTODO: new native-endianness archive format */
#if 0
#define CTFA_MAGIC 0x8b47f2a4d7623eec		/* Incremented. */
typedef struct ctf_archive
{
  /* Magic number.  (In loaded files, overwritten with the file size
     so ctf_arc_close() knows how much to munmap()).  */
  uint64_t ctfa_magic;

  /* CTF data model.  */
  uint64_t ctfa_model;

  /* Offset of the shared properties table.  */
  uint64_t ctfa_shared_prop_off;

  /* Length of the shared properties table.  */
  uint64_t ctfa_shared_prop_len;

  /* Number of CTF dicts in the archive.  */
  uint64_t ctfa_ndicts;

  /* Offset of the name table.  */
  uint64_t ctfa_names;

  /* Offset of the CTF table.  Each element starts with a size (a little-
     endian uint64_t) then a ctf_dict_t of that size.  */
  uint64_t ctfa_ctfs;
} ctf_archive_t;
#endif
#define CTFA_MAGIC 0x8b47f2a4d7623eeb	/* Random.  */

typedef struct ctf_archive
{
  /* Magic number.  (In loaded files, overwritten with the file size
     so ctf_arc_close() knows how much to munmap()).  */
  uint64_t ctfa_magic;

  /* CTF data model.  */
  uint64_t ctfa_model;

  /* Number of CTF dicts in the archive.  */
  uint64_t ctfa_ndicts;

  /* Offset of the name table.  */
  uint64_t ctfa_names;

  /* Offset of the CTF table.  Each element starts with a size (a little-
     endian uint64_t) then a ctf_dict_t of that size.  */
  uint64_t ctfa_ctfs;
} ctfa_archive_v1_t;

/* An array of ctfa_ndicts of this structure lies at
   ctf_archive[sizeof(struct ctf_archive)] and gives the ctfa_ctfs or
   ctfa_names-relative offsets of each name or ctf_dict_t.  */

typedef struct ctf_archive_modent
{
  uint64_t name_offset;
  uint64_t ctf_offset;
} ctf_archive_modent_t;

#ifdef	__cplusplus
}
#endif

#endif				/* _CTF_H */
