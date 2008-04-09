/*
 * lens.h: Repreentation of lenses
 *
 * Copyright (C) 2007 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: David Lutterkort <dlutter@redhat.com>
 */

#ifndef __LENS_H
#define __LENS_H

#include "syntax.h"
#include "fa.h"

enum lens_tag {
    L_DEL,
    L_STORE,
    L_KEY,
    L_LABEL,
    L_SEQ,
    L_COUNTER,
    L_CONCAT,
    L_UNION,
    L_SUBTREE,
    L_STAR,
    L_MAYBE
};

struct lens {
    unsigned int              ref;
    enum lens_tag             tag;
    struct info              *info;
    struct regexp            *ctype;
    struct regexp            *atype;
    union {
        /* Primitive lenses */
        struct {                   /* L_DEL uses both */
            struct regexp *regexp; /* L_STORE, L_KEY */
            struct string *string; /* L_LABEL, L_SEQ, L_COUNTER */
        };
        /* Combinators */
        struct lens *child;         /* L_SUBTREE, L_STAR, L_MAYBE */
        struct {                    /* L_UNION, L_CONCAT */
            unsigned int nchildren;
            struct lens **children;
        };
    };
};

/* Constructors for various lens types. Constructor assumes ownership of
 * arguments without incrementing. Caller owns returned lenses.
 *
 * The return type is VALUE instead of LENS so that we can return an
 * exception iftypechecking fails.
 */
struct value *lns_make_prim(enum lens_tag tag, struct info *info,
                            struct regexp *regexp, struct string *string);
struct value *lns_make_union(struct info *, struct lens *, struct lens *,
                             int check);
struct value *lns_make_concat(struct info *, struct lens *, struct lens *,
                              int check);
struct value *lns_make_subtree(struct info *, struct lens *);
struct value *lns_make_star(struct info *, struct lens *,
                            int check);
struct value *lns_make_plus(struct info *, struct lens *,
                            int check);
struct value *lns_make_maybe(struct info *, struct lens *,
                             int check);

/* Flags to control debug printing during parsing */
enum parse_debug_flags {
    PF_NONE    = 0,
    PF_ADVANCE = (1 << 0),  /* Show how the lexer advances through the input */
    PF_MATCH   = (1 << 1),  /* Show regex matches */
    PF_TOKEN   = (1 << 2)   /* Show tokenization */
};

/* Auxiliary data structures used during get/put/create */
struct skel {
    struct skel *next;
    struct lens *lens;
    enum lens_tag tag;
    union {
        const char *text;    /* L_DEL */
        struct skel *skels;  /* L_CONCAT, L_STAR, L_MAYBE */
    };
    /* Also tag == L_SUBTREE, with no data in the union */
};

/* A dictionary that maps key to a list of (skel, dict) */
struct dict_entry {
    struct dict_entry *next;
    struct skel *skel;
    struct dict *dict;
};

struct dict {
    struct dict *next;
    const char *key;
    struct dict_entry *entry; /* This will change as entries are looked up */
    struct dict_entry *mark;  /* Pointer to initial entry, will never change */
};

struct lns_error {
    struct lens * lens;
    int           pos;        /* Errors from get/parse */
    char         *path;       /* Errors from put, pos will be -1 */
    char         *message;
};

void free_skel(struct skel *skel);
void free_dict(struct dict *dict);
void free_lns_error(struct lns_error *err);

/* Parse text TEXT according to GRAMMAR. FILENAME indicates what file TEXT
 * was read from.
 * LOG is used to print logging messages. FLAGS controls what is printed
 * and should be a set of flags from enum parse_flags
 */
struct tree *lns_get(struct info *info, struct lens *lens, const char *text,
                     FILE *log, int flags, struct lns_error **err);
struct skel *lns_parse(struct lens *lens, const char *text,
                       struct dict **dict, struct lns_error **err);
void lns_put(FILE *out, struct lens *lens, struct tree *tree,
             const char *text, struct lns_error **err);

void free_lens(struct lens *lens);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
