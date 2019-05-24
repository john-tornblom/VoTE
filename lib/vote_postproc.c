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


#include <stdlib.h>
#include <assert.h>

#include "vote_postproc.h"
#include "vote_math.h"


typedef struct vote_postproc {
  const vote_ensemble_t *ensemble;
  vote_mapping_cb_t     *user_cb;
  void                  *user_ctx;
} vote_postproc_t;


/**
 * Post-processing used by random forests.
 **/
static void
vote_mapping_division(vote_mapping_t *m, size_t divisor) {
  for(size_t i=0; i<m->nb_outputs; i++) {
    m->outputs[i].lower /= divisor;
    m->outputs[i].upper /= divisor;
  }
}


/**
 * Post-processing algorithm used by some gradient boosting machines.
 **/
static void
vote_mapping_softmax(vote_mapping_t *m) {
  vote_bound_t sum = {0, 0};
  vote_bound_t off = {0, 0};
  real_t max = -INFINITY;

  // This is used for numerical stabillity, see
  // http://www.deeplearningbook.org/contents/numerical.html
  for(size_t i=0; i<m->nb_outputs; i++) {
    max = vote_max(max, m->outputs[i].upper);
  }

  for(size_t i=0; i<m->nb_outputs; i++) {
    sum.lower += vote_exp(m->outputs[i].lower - max);
    sum.upper += vote_exp(m->outputs[i].upper - max);
  }

  // log(0) results in undefined behaviour
  assert(sum.lower != 0);
  assert(sum.upper != 0);

  off.lower = vote_log(sum.lower) + max;
  off.upper = vote_log(sum.upper) + max;

  //negate and swap bounds in the box that captures the offset
  real_t tmp = off.lower;
  off.lower = -off.upper;
  off.upper = -tmp;
    
  for(size_t i=0; i<m->nb_outputs; i++) {
    m->outputs[i].lower = vote_exp(off.lower + m->outputs[i].lower);
    m->outputs[i].upper = vote_exp(off.upper + m->outputs[i].upper);
  }
}


/**
 * Apply a post processing algorithm on a mapping.
 **/
static vote_outcome_t
vote_postproc_input(void *ctx, vote_mapping_t *m) {
  vote_postproc_t *pp = (vote_postproc_t*)ctx;

  switch(pp->ensemble->post_process) {    
  case VOTE_POST_PROCESS_DIVISOR:
    vote_mapping_division(m, pp->ensemble->nb_trees);
    break;
    
  case VOTE_POST_PROCESS_SOFTMAX:
    vote_mapping_softmax(m);
    break;

  default:
  case VOTE_POST_PROCESS_NONE:
    break;
  }
  
  return pp->user_cb(pp->user_ctx, m);
}


vote_pipeline_t*
vote_postproc_pipeline(const vote_ensemble_t *e, void *user_ctx,
		       vote_mapping_cb_t *user_cb) {
  vote_postproc_t *pp = calloc(1, sizeof(vote_postproc_t));
  vote_pipeline_t *p = vote_pipeline_new(pp, vote_postproc_input, free);
  
  assert(pp);

  pp->ensemble = e;
  pp->user_ctx = user_ctx;
  pp->user_cb  = user_cb;

  return p;
}
