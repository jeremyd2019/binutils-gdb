/* This testcase is part of GDB, the GNU debugger.

   Copyright 2008-2025 Free Software Foundation, Inc.

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

struct One
{
  int x;
};

struct Two
{
  struct One one;
  int x, y;
};

struct Two two = { { 1 }, 2, 3 };

typedef struct One tOne;
typedef struct Two tTwo;

tOne *onep = &two.one;
tTwo *twop = &two;

int main()
{
  onep->x = twop->y;
  return 0;
}
