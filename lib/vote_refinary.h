/* Copyright (C) 2018 John Törnblom

   This file is part of VoTE (Verifier of Tree Ensembles).

VoTE is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

VoTE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
for more details.

You should have received a copy of the GNU Lesser General Public
License along with VoTE; see the files COPYING and COPYING.LESSER. If not,
see <http://www.gnu.org/licenses/>.  */


#ifndef VOTE_REFINERY_H
#define VOTE_REFINERY_H

#include <stddef.h>
#include <stdbool.h>

#include "vote.h"
#include "vote_tree.h"


/**
 * A refinery evaluates a tree, and emits mappings.
 **/
typedef struct vote_refinery {
  const vote_tree_t *tree;
  vote_mapping_cb_t *emit_cb;
  void              *emit_ctx;
} vote_refinery_t;


/**
 * Create a new refinery.
 **/
vote_refinery_t* vote_refinery_new(const vote_tree_t *t, vote_mapping_cb_t *cb,
				   void* ctx);


/**
 * Delete a refinery.
 **/
void vote_refinery_del(vote_refinery_t* r);


/**
 * Refine and emit mappings.
 **/
bool vote_refinery_emit(vote_refinery_t* r, vote_mapping_t *m);


#endif //VOTE_REFINERY_H
