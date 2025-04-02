/* This testcase is part of GDB, the GNU debugger.

   Copyright 2024-2025 Free Software Foundation, Inc.

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

/* Used so we have some work to do.  */
volatile int global_var = 0;

int
main (void)
{			/* prologue location */
  ++global_var;		/* first location */

  /*
   *
   *
   * This comment is here as filler.
   *
   *
   */

  ++global_var;		/* second location */

  /*
   *
   *
   * This comment is also here as filler.
   *
   *
   */

  ++global_var;		/* third location */

  /*
   *
   *
   * This is yet another filler comment.
   *
   *
   */

  return 0;		/* fourth location */
}
