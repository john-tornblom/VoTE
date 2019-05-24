/* Copyright (C) 2019 John Törnblom

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

#include <assert.h>
#include <stdlib.h>

#include "vote.h"
#include "vote_pipeline.h"


struct vote_pipeline {
  vote_pipeline_t        *next;
  void                   *ctx;
  vote_mapping_cb_t      *on_input;
  vote_pipeline_del_cb_t *on_delete;
};


vote_pipeline_t*
vote_pipeline_new(void *ctx, vote_mapping_cb_t *on_input,
		  vote_pipeline_del_cb_t *on_delete) {
  vote_pipeline_t *p = calloc(1, sizeof(vote_pipeline_t));
  assert(p);

  p->ctx       = ctx;
  p->on_input  = on_input;
  p->on_delete = on_delete;
  
  return p;
}


void
vote_pipeline_del(vote_pipeline_t *p) {
  if(p->on_delete) {
    p->on_delete(p->ctx);
  }
  free(p);
}


void
vote_pipeline_connect(vote_pipeline_t *src, vote_pipeline_t *sink) {
  assert(!src->next);
  src->next = sink;
}


vote_outcome_t
vote_pipeline_input(const vote_pipeline_t *p, vote_mapping_t *m) {
  return p->on_input(p->ctx, m);
}


vote_outcome_t
vote_pipeline_output(const vote_pipeline_t *p, vote_mapping_t *m) {
  assert(p->next);
  return vote_pipeline_input(p->next, m);
}
