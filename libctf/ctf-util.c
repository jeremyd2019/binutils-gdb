/* Simple data structure utilities and helpers.
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

#include <ctf-impl.h>
#include <string.h>
#include "ctf-endian.h"

/* Simple doubly-linked list append routine.  This implementation assumes that
   each list element contains an embedded ctf_list_t as the first member.
   An additional ctf_list_t is used to store the head (l_next) and tail
   (l_prev) pointers.  The current head and tail list elements have their
   previous and next pointers set to NULL, respectively.  */

void
ctf_list_append (ctf_list_t *lp, void *newp)
{
  ctf_list_t *p = lp->l_prev;	/* p = tail list element.  */
  ctf_list_t *q = newp;		/* q = new list element.  */

  lp->l_prev = q;
  q->l_prev = p;
  q->l_next = NULL;

  if (p != NULL)
    p->l_next = q;
  else
    lp->l_next = q;
}

/* Prepend the specified existing element to the given ctf_list_t.  The
   existing pointer should be pointing at a struct with embedded ctf_list_t.  */

void
ctf_list_prepend (ctf_list_t * lp, void *newp)
{
  ctf_list_t *p = newp;		/* p = new list element.  */
  ctf_list_t *q = lp->l_next;	/* q = head list element.  */

  lp->l_next = p;
  p->l_prev = NULL;
  p->l_next = q;

  if (q != NULL)
    q->l_prev = p;
  else
    lp->l_prev = p;
}

/* Delete the specified existing element from the given ctf_list_t.  The
   existing pointer should be pointing at a struct with embedded ctf_list_t.  */

void
ctf_list_delete (ctf_list_t *lp, void *existing)
{
  ctf_list_t *p = existing;

  if (p->l_prev != NULL)
    p->l_prev->l_next = p->l_next;
  else
    lp->l_next = p->l_next;

  if (p->l_next != NULL)
    p->l_next->l_prev = p->l_prev;
  else
    lp->l_prev = p->l_prev;
}

/* Return 1 if the list is empty.  */

int
ctf_list_empty_p (ctf_list_t *lp)
{
  return (lp->l_next == NULL && lp->l_prev == NULL);
}

/* Splice one entire list onto the end of another one.  The existing list is
   emptied.  */

void
ctf_list_splice (ctf_list_t *lp, ctf_list_t *append)
{
  if (ctf_list_empty_p (append))
    return;

  if (lp->l_prev != NULL)
    lp->l_prev->l_next = append->l_next;
  else
    lp->l_next = append->l_next;

  append->l_next->l_prev = lp->l_prev;
  lp->l_prev = append->l_prev;
  append->l_next = NULL;
  append->l_prev = NULL;
}

/* A string appender working on dynamic strings.  Returns NULL on OOM.  */

char *
ctf_str_append (char *s, const char *append)
{
  size_t s_len = 0;

  if (append == NULL)
    return s;

  if (s != NULL)
    s_len = strlen (s);

  size_t append_len = strlen (append);

  if ((s = realloc (s, s_len + append_len + 1)) == NULL)
    return NULL;

  memcpy (s + s_len, append, append_len);
  s[s_len + append_len] = '\0';

  return s;
}

/* A version of ctf_str_append that returns the old string on OOM.  */

char *
ctf_str_append_noerr (char *s, const char *append)
{
  char *new_s;

  new_s = ctf_str_append (s, append);
  if (!new_s)
    return s;
  return new_s;
}

/* Create a ctf_next_t.  */

ctf_next_t *
ctf_next_create (void)
{
  return calloc (1, sizeof (struct ctf_next));
}

/* Destroy a ctf_next_t, for early exit from iterators.  */

void
ctf_next_destroy (ctf_next_t *i)
{
  if (i == NULL)
    return;

  if (i->ctn_iter_fun == (void (*) (void)) ctf_dynhash_next_sorted)
    free (i->u.ctn_sorted_hkv);
  if (i->ctn_next)
    ctf_next_destroy (i->ctn_next);
  if (i->ctn_next_inner)
    ctf_next_destroy (i->ctn_next_inner);
  free (i);
}

/* Copy a ctf_next_t.  */

ctf_next_t *
ctf_next_copy (ctf_next_t *i)
{
  ctf_next_t *i2;

  if ((i2 = ctf_next_create()) == NULL)
    return NULL;
  memcpy (i2, i, sizeof (struct ctf_next));

  if (i2->ctn_next)
    {
      i2->ctn_next = ctf_next_copy (i2->ctn_next);
      if (i2->ctn_next == NULL)
	goto err_next;
    }

  if (i2->ctn_next_inner)
    {
      i2->ctn_next_inner = ctf_next_copy (i2->ctn_next_inner);
      if (i2->ctn_next_inner == NULL)
	goto err_next_inner;
    }

  if (i2->ctn_iter_fun == (void (*) (void)) ctf_dynhash_next_sorted)
    {
      size_t els = ctf_dynhash_elements ((ctf_dynhash_t *) i->cu.ctn_h);
      if ((i2->u.ctn_sorted_hkv = calloc (els, sizeof (ctf_next_hkv_t))) == NULL)
	goto err_sorted_hkv;
      memcpy (i2->u.ctn_sorted_hkv, i->u.ctn_sorted_hkv,
	      els * sizeof (ctf_next_hkv_t));
    }
  return i2;

 err_sorted_hkv:
  ctf_next_destroy (i2->ctn_next_inner);
 err_next_inner:
  ctf_next_destroy (i2->ctn_next);
 err_next:
  ctf_next_destroy (i2);
  return NULL;
}
