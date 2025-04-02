/* langinfo.h file for DJGPP.
   Copyright (C) 2009-2025 Free Software Foundation, Inc.
   Written by Eli Zaretskii.

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

#ifndef GDB_CONFIG_DJGPP_LANGINFO_H
#define GDB_CONFIG_DJGPP_LANGINFO_H

#include <nl_types.h>

enum {
  CODESET,
  /* Number of enumerated values.  */
  _NL_NUM
};

#define CODESET CODESET

extern char *nl_langinfo (nl_item);

#endif /* GDB_CONFIG_DJGPP_LANGINFO_H */
